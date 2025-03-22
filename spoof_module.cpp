#include <jni.h>
#include <string>
#include <zygisk.hh>
#include <nlohmann/json.hpp>
#include <fstream>
#include <unordered_map>
#include <sys/system_properties.h>
#include <dlfcn.h>
#include <sys/mman.h>
#include <unistd.h>
#include "JniHelper.hpp"

using json = nlohmann::json;

struct DeviceInfo {
    std::string brand;
    std::string device;
    std::string manufacturer;
    std::string model;
    std::string fingerprint;
    std::string build_id;
    std::string display;
    std::string product;
    std::string version_release;
    std::string serial;
    std::string cpuinfo;
    std::string serial_content;
};

// Function pointers
typedef int (*orig_prop_get_t)(const char*, char*, const char*);
static orig_prop_get_t orig_prop_get = nullptr;
typedef ssize_t (*orig_read_t)(int, void*, size_t);
static orig_read_t orig_read = nullptr;
typedef void (*orig_set_static_object_field_t)(JNIEnv*, jclass, jfieldID, jobject);
static orig_set_static_object_field_t orig_set_static_object_field = nullptr;
typedef int (*orig_stat_t)(const char*, struct stat*);
static orig_stat_t orig_stat = nullptr;

// Global variables
static DeviceInfo current_info;
static jclass buildClass = nullptr;
static jclass versionClass = nullptr;
static jfieldID modelField = nullptr;
static jfieldID brandField = nullptr;
static jfieldID deviceField = nullptr;
static jfieldID manufacturerField = nullptr;
static jfieldID fingerprintField = nullptr;
static jfieldID buildIdField = nullptr;
static jfieldID displayField = nullptr;
static jfieldID productField = nullptr;
static jfieldID versionReleaseField = nullptr;
static jfieldID sdkIntField = nullptr;
static jfieldID serialField = nullptr;

class SpoofModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api* api, JNIEnv* env) override {
        this->api = api;
        this->env = env;

        try {
            if (!buildClass) {
                jni::ScopedLocalRef<jclass> localBuildClass(env, jni::FindClass(env, "android/os/Build"));
                buildClass = (jclass)env->NewGlobalRef(localBuildClass.get());
                if (buildClass) {
                    modelField = jni::GetStaticFieldID(env, buildClass, "MODEL", "Ljava/lang/String;");
                    brandField = jni::GetStaticFieldID(env, buildClass, "BRAND", "Ljava/lang/String;");
                    deviceField = jni::GetStaticFieldID(env, buildClass, "DEVICE", "Ljava/lang/String;");
                    manufacturerField = jni::GetStaticFieldID(env, buildClass, "MANUFACTURER", "Ljava/lang/String;");
                    fingerprintField = jni::GetStaticFieldID(env, buildClass, "FINGERPRINT", "Ljava/lang/String;");
                    buildIdField = jni::GetStaticFieldID(env, buildClass, "ID", "Ljava/lang/String;");
                    displayField = jni::GetStaticFieldID(env, buildClass, "DISPLAY", "Ljava/lang/String;");
                    productField = jni::GetStaticFieldID(env, buildClass, "PRODUCT", "Ljava/lang/String;");
                    serialField = jni::GetStaticFieldID(env, buildClass, "SERIAL", "Ljava/lang/String;");
                }
            }
            if (!versionClass) {
                jni::ScopedLocalRef<jclass> localVersionClass(env, jni::FindClass(env, "android/os/Build$VERSION"));
                versionClass = (jclass)env->NewGlobalRef(localVersionClass.get());
                if (versionClass) {
                    versionReleaseField = jni::GetStaticFieldID(env, versionClass, "RELEASE", "Ljava/lang/String;");
                    sdkIntField = jni::GetStaticFieldID(env, versionClass, "SDK_INT", "I");
                }
            }
        } catch (const jni::JNIException& e) {
            return;
        }

        void* handle = dlopen("libc.so", RTLD_LAZY);
        if (handle) {
            orig_prop_get = (orig_prop_get_t)dlsym(handle, "__system_property_get");
            orig_read = (orig_read_t)dlsym(handle, "read");
            orig_stat = (orig_stat_t)dlsym(handle, "stat");
            dlclose(handle);
        }
        handle = dlopen("libandroid_runtime.so", RTLD_LAZY);
        if (handle) {
            orig_set_static_object_field = (orig_set_static_object_field_t)dlsym(handle, "JNI_SetStaticObjectField");
            dlclose(handle);
        }

        hookNativeGetprop();
        hookNativeRead();
        hookJniSetStaticObjectField();
        hookStat();
        loadConfig();
    }

    void preAppSpecialize(zygisk::AppSpecializeArgs* args) override {
        if (!args || !args->nice_name) {
            api->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
            return;
        }
        jni::ScopedLocalRef<jstring> niceName(env, args->nice_name);
        const char* package_name = env->GetStringUTFChars(niceName.get(), nullptr);
        if (!package_name) {
            api->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
            return;
        }
        auto it = package_map.find(package_name);
        if (it == package_map.end()) {
            api->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
        } else {
            current_info = it->second;
            spoofDevice(current_info);
            spoofSystemProperties(current_info);
        }
        env->ReleaseStringUTFChars(niceName.get(), package_name);
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs* args) override {
        if (!args || !args->nice_name || package_map.empty() || !buildClass) return;
        jni::ScopedLocalRef<jstring> niceName(env, args->nice_name);
        const char* package_name = env->GetStringUTFChars(niceName.get(), nullptr);
        if (!package_name) return;
        auto it = package_map.find(package_name);
        if (it != package_map.end()) {
            current_info = it->second;
            spoofDevice(current_info);
            spoofSystemProperties(current_info);
        }
        env->ReleaseStringUTFChars(niceName.get(), package_name);
    }

private:
    zygisk::Api* api;
    JNIEnv* env;
    std::unordered_map<std::string, DeviceInfo> package_map;

    std::string obfuscateString(const std::string& input) {
        std::string result = input;
        const char key = 0x5A;
        for (char& c : result) c ^= key;
        return result;
    }

    void loadConfig() {
        std::string obfuscatedPath = obfuscateString("/data/adb/modules/COPG/config.json");
        std::string realPath = obfuscateString(obfuscatedPath);
        std::ifstream file(realPath);
        if (!file.is_open()) return;
        try {
            json config = json::parse(file);
            for (auto& [key, value] : config.items()) {
                if (key.find("_DEVICE") != std::string::npos) continue;
                auto packages = value.get<std::vector<std::string>>();
                std::string device_key = key + "_DEVICE";
                if (!config.contains(device_key)) continue;
                auto device = config[device_key];
                DeviceInfo info;
                info.brand = device["BRAND"].get<std::string>();
                info.device = device["DEVICE"].get<std::string>();
                info.manufacturer = device["MANUFACTURER"].get<std::string>();
                info.model = device["MODEL"].get<std::string>();
                info.fingerprint = device.contains("FINGERPRINT") ? device["FINGERPRINT"].get<std::string>() : "generic/brand/device:13/TQ3A.230805.001/123456:user/release-keys";
                info.build_id = device.contains("BUILD_ID") ? device["BUILD_ID"].get<std::string>() : "";
                info.display = device.contains("DISPLAY") ? device["DISPLAY"].get<std::string>() : "";
                info.product = device.contains("PRODUCT") ? device["PRODUCT"].get<std::string>() : info.device;
                info.version_release = device.contains("VERSION_RELEASE") ? device["VERSION_RELEASE"].get<std::string>() : "";
                info.serial = device.contains("SERIAL") ? device["SERIAL"].get<std::string>() : "";
                info.cpuinfo = device.contains("CPUINFO") ? device["CPUINFO"].get<std::string>() : "";
                info.serial_content = device.contains("SERIAL_CONTENT") ? device["SERIAL_CONTENT"].get<std::string>() : "";
                for (const auto& pkg : packages) package_map[pkg] = info;
            }
        } catch (const json::exception&) {}
        file.close();
    }

    void spoofDevice(const DeviceInfo& info) {
        if (modelField) {
            jni::ScopedLocalRef<jstring> jstr(env, jni::StringToJString(env, info.model));
            env->SetStaticObjectField(buildClass, modelField, jstr.get());
        }
        if (brandField) {
            jni::ScopedLocalRef<jstring> jstr(env, jni::StringToJString(env, info.brand));
            env->SetStaticObjectField(buildClass, brandField, jstr.get());
        }
        if (deviceField) {
            jni::ScopedLocalRef<jstring> jstr(env, jni::StringToJString(env, info.device));
            env->SetStaticObjectField(buildClass, deviceField, jstr.get());
        }
        if (manufacturerField) {
            jni::ScopedLocalRef<jstring> jstr(env, jni::StringToJString(env, info.manufacturer));
            env->SetStaticObjectField(buildClass, manufacturerField, jstr.get());
        }
        if (fingerprintField) {
            jni::ScopedLocalRef<jstring> jstr(env, jni::StringToJString(env, info.fingerprint));
            env->SetStaticObjectField(buildClass, fingerprintField, jstr.get());
        }
        if (buildIdField && !info.build_id.empty()) {
            jni::ScopedLocalRef<jstring> jstr(env, jni::StringToJString(env, info.build_id));
            env->SetStaticObjectField(buildClass, buildIdField, jstr.get());
        }
        if (displayField && !info.display.empty()) {
            jni::ScopedLocalRef<jstring> jstr(env, jni::StringToJString(env, info.display));
            env->SetStaticObjectField(buildClass, displayField, jstr.get());
        }
        if (productField && !info.product.empty()) {
            jni::ScopedLocalRef<jstring> jstr(env, jni::StringToJString(env, info.product));
            env->SetStaticObjectField(buildClass, productField, jstr.get());
        }
        if (versionReleaseField && !info.version_release.empty()) {
            jni::ScopedLocalRef<jstring> jstr(env, jni::StringToJString(env, info.version_release));
            env->SetStaticObjectField(versionClass, versionReleaseField, jstr.get());
        }
        if (sdkIntField && !info.version_release.empty()) 
            env->SetStaticIntField(versionClass, sdkIntField, info.version_release == "13" ? 33 : 34);
        if (serialField && !info.serial.empty()) {
            jni::ScopedLocalRef<jstring> jstr(env, jni::StringToJString(env, info.serial));
            env->SetStaticObjectField(buildClass, serialField, jstr.get());
        }
    }

    void spoofSystemProperties(const DeviceInfo& info) {
        if (!info.brand.empty()) __system_property_set("ro.product.brand", info.brand.c_str());
        if (!info.device.empty()) __system_property_set("ro.product.device", info.device.c_str());
        if (!info.manufacturer.empty()) __system_property_set("ro.product.manufacturer", info.manufacturer.c_str());
        if (!info.model.empty()) __system_property_set("ro.product.model", info.model.c_str());
        if (!info.fingerprint.empty()) __system_property_set("ro.build.fingerprint", info.fingerprint.c_str());
    }

    static int hooked_prop_get(const char* name, char* value, const char* default_value) {
        if (!orig_prop_get) return -1;
        std::string prop_name(name);
        if (prop_name == "ro.product.brand" && !current_info.brand.empty()) {
            strncpy(value, current_info.brand.c_str(), PROP_VALUE_MAX);
            return current_info.brand.length();
        } else if (prop_name == "ro.product.device" && !current_info.device.empty()) {
            strncpy(value, current_info.device.c_str(), PROP_VALUE_MAX);
            return current_info.device.length();
        } else if (prop_name == "ro.product.manufacturer" && !current_info.manufacturer.empty()) {
            strncpy(value, current_info.manufacturer.c_str(), PROP_VALUE_MAX);
            return current_info.manufacturer.length();
        } else if (prop_name == "ro.product.model" && !current_info.model.empty()) {
            strncpy(value, current_info.model.c_str(), PROP_VALUE_MAX);
            return current_info.model.length();
        } else if (prop_name == "ro.build.fingerprint" && !current_info.fingerprint.empty()) {
            strncpy(value, current_info.fingerprint.c_str(), PROP_VALUE_MAX);
            return current_info.fingerprint.length();
        }
        return orig_prop_get(name, value, default_value);
    }

    void hookNativeGetprop() {
        if (!orig_prop_get) return;
        void* sym = dlsym(RTLD_DEFAULT, "__system_property_get");
        if (!sym) return;
        size_t page_size = sysconf(_SC_PAGE_SIZE);
        void* page_start = (void*)((uintptr_t)sym & ~(page_size - 1));
        if (mprotect(page_start, page_size, PROT_READ | PROT_WRITE | PROT_EXEC) == 0) {
            *(void**)&orig_prop_get = sym;
            *(void**)sym = (void*)hooked_prop_get;
            mprotect(page_start, page_size, PROT_READ | PROT_EXEC);
        }
    }

    static ssize_t hooked_read(int fd, void* buf, size_t count) {
        if (!orig_read) return -1;
        char path[256];
        snprintf(path, sizeof(path), "/proc/self/fd/%d", fd);
        char real_path[256];
        ssize_t len = readlink(path, real_path, sizeof(real_path) - 1);
        if (len != -1) {
            real_path[len] = '\0';
            std::string file_path(real_path);
            if (file_path == "/proc/cpuinfo" && !current_info.cpuinfo.empty()) {
                size_t bytes_to_copy = std::min(count, current_info.cpuinfo.length());
                memcpy(buf, current_info.cpuinfo.c_str(), bytes_to_copy);
                return bytes_to_copy;
            } else if (file_path == "/sys/devices/soc0/serial_number" && !current_info.serial_content.empty()) {
                size_t bytes_to_copy = std::min(count, current_info.serial_content.length());
                memcpy(buf, current_info.serial_content.c_str(), bytes_to_copy);
                return bytes_to_copy;
            }
        }
        return orig_read(fd, buf, count);
    }

    void hookNativeRead() {
        if (!orig_read) return;
        void* sym = dlsym(RTLD_DEFAULT, "read");
        if (!sym) return;
        size_t page_size = sysconf(_SC_PAGE_SIZE);
        void* page_start = (void*)((uintptr_t)sym & ~(page_size - 1));
        if (mprotect(page_start, page_size, PROT_READ | PROT_WRITE | PROT_EXEC) == 0) {
            *(void**)&orig_read = sym;
            *(void**)sym = (void*)hooked_read;
            mprotect(page_start, page_size, PROT_READ | PROT_EXEC);
        }
    }

    static void hooked_set_static_object_field(JNIEnv* env, jclass clazz, jfieldID fieldID, jobject value) {
        if (clazz == buildClass) {
            if (fieldID == modelField || fieldID == brandField || fieldID == deviceField ||
                fieldID == manufacturerField || fieldID == fingerprintField || fieldID == buildIdField ||
                fieldID == displayField || fieldID == productField || fieldID == serialField) {
                return;
            }
        } else if (clazz == versionClass) {
            if (fieldID == versionReleaseField) {
                return;
            }
        }
        if (orig_set_static_object_field) {
            orig_set_static_object_field(env, clazz, fieldID, value);
        }
    }

    void hookJniSetStaticObjectField() {
        if (!orig_set_static_object_field) return;
        void* sym = dlsym(RTLD_DEFAULT, "JNI_SetStaticObjectField");
        if (!sym) return;
        size_t page_size = sysconf(_SC_PAGE_SIZE);
        void* page_start = (void*)((uintptr_t)sym & ~(page_size - 1));
        if (mprotect(page_start, page_size, PROT_READ | PROT_WRITE | PROT_EXEC) == 0) {
            *(void**)&orig_set_static_object_field = sym;
            *(void**)sym = (void*)hooked_set_static_object_field;
            mprotect(page_start, page_size, PROT_READ | PROT_EXEC);
        }
    }

    static int hooked_stat(const char* path, struct stat* buf) {
        if (!orig_stat) return -1;
        std::string spath(path);
        if (spath.find("COPG") != std::string::npos || 
            spath.find("zygisk") != std::string::npos || 
            spath.find("/data/adb/modules") != std::string::npos) {
            errno = ENOENT;
            return -1;
        }
        return orig_stat(path, buf);
    }

    void hookStat() {
        if (!orig_stat) return;
        void* sym = dlsym(RTLD_DEFAULT, "stat");
        if (!sym) return;
        size_t page_size = sysconf(_SC_PAGE_SIZE);
        void* page_start = (void*)((uintptr_t)sym & ~(page_size - 1));
        if (mprotect(page_start, page_size, PROT_READ | PROT_WRITE | PROT_EXEC) == 0) {
            *(void**)&orig_stat = sym;
            *(void**)sym = (void*)hooked_stat;
            mprotect(page_start, page_size, PROT_READ | PROT_EXEC);
        }
    }
};

REGISTER_ZYGISK_MODULE(SpoofModule)
