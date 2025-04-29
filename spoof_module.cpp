#include <jni.h>
#include <string>
#include <zygisk.hpp>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <sys/system_properties.h>
#include <dlfcn.h>
#include <sys/mman.h>
#include <unistd.h>
#include <android/log.h>
#include <cstdio>
#include <vector>
#include <cerrno>
#include <cstring>

using json = nlohmann::json;

#define LOG_TAG "SpoofModule"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

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

typedef int (*orig_prop_get_t)(const char*, char*, const char*);
static orig_prop_get_t orig_prop_get = nullptr;
typedef ssize_t (*orig_read_t)(int, void*, size_t);
static orig_read_t orig_read = nullptr;
typedef void (*orig_set_static_object_field_t)(JNIEnv*, jclass, jfieldID, jobject);
static orig_set_static_object_field_t orig_set_static_object_field = nullptr;

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

static ssize_t xread(int fd, void *buffer, size_t count) {
    ssize_t total = 0;
    char *buf = static_cast<char *>(buffer);
    while (count > 0) {
        ssize_t ret = TEMP_FAILURE_RETRY(read(fd, buf, count));
        if (ret < 0) return -1;
        buf += ret;
        total += ret;
        count -= ret;
    }
    return total;
}

static ssize_t xwrite(int fd, const void *buffer, size_t count) {
    ssize_t total = 0;
    char *buf = (char *)buffer;
    while (count > 0) {
        ssize_t ret = TEMP_FAILURE_RETRY(write(fd, buf, count));
        if (ret < 0) return -1;
        buf += ret;
        total += ret;
        count -= ret;
    }
    return total;
}

class SpoofModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api* api, JNIEnv* env) override {
        this->api = api;
        this->env = env;

        LOGD("Module loaded successfully");

        if (!buildClass) {
            buildClass = (jclass)env->NewGlobalRef(env->FindClass("android/os/Build"));
            if (buildClass) {
                modelField = env->GetStaticFieldID(buildClass, "MODEL", "Ljava/lang/String;");
                brandField = env->GetStaticFieldID(buildClass, "BRAND", "Ljava/lang/String;");
                deviceField = env->GetStaticFieldID(buildClass, "DEVICE", "Ljava/lang/String;");
                manufacturerField = env->GetStaticFieldID(buildClass, "MANUFACTURER", "Ljava/lang/String;");
                fingerprintField = env->GetStaticFieldID(buildClass, "FINGERPRINT", "Ljava/lang/String;");
                buildIdField = env->GetStaticFieldID(buildClass, "ID", "Ljava/lang/String;");
                displayField = env->GetStaticFieldID(buildClass, "DISPLAY", "Ljava/lang/String;");
                productField = env->GetStaticFieldID(buildClass, "PRODUCT", "Ljava/lang/String;");
                serialField = env->GetStaticFieldID(buildClass, "SERIAL", "Ljava/lang/String;");
            } else {
                LOGE("Failed to find android/os/Build class");
            }
        }
        if (!versionClass) {
            versionClass = (jclass)env->NewGlobalRef(env->FindClass("android/os/Build$VERSION"));
            if (versionClass) {
                versionReleaseField = env->GetStaticFieldID(versionClass, "RELEASE", "Ljava/lang/String;");
                sdkIntField = env->GetStaticFieldID(versionClass, "SDK_INT", "I");
            } else {
                LOGE("Failed to find android/os/Build$VERSION class");
            }
        }

        void* handle = dlopen("libc.so", RTLD_LAZY);
        if (handle) {
            orig_prop_get = (orig_prop_get_t)dlsym(handle, "__system_property_get");
            orig_read = (orig_read_t)dlsym(handle, "read");
            dlclose(handle);
        } else {
            LOGE("Failed to open libc.so");
        }

        hookNativeGetprop();
        hookNativeRead();
        hookJniSetStaticObjectField();
    }

    void preAppSpecialize(zygisk::AppSpecializeArgs* args) override {
        if (!args || !args->nice_name) {
            LOGD("No package name provided, closing module");
            api->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
            return;
        }
        const char* package_name = env->GetStringUTFChars(args->nice_name, nullptr);
        if (!package_name) {
            LOGE("Failed to get package name");
            api->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
            return;
        }
        LOGD("Processing package: %s", package_name);

        loadConfig();

        auto it = package_map.find(package_name);
        if (it == package_map.end()) {
            LOGD("Package %s not found in config, closing module", package_name);
            api->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
        } else {
            current_info = it->second;
            LOGD("Spoofing device for package %s: model %s", package_name, current_info.model.c_str());
            spoofDevice(current_info);
            spoofSystemProperties(current_info);
        }
        env->ReleaseStringUTFChars(args->nice_name, package_name);
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs* args) override {
        if (!args || !args->nice_name || package_map.empty() || !buildClass) return;
        const char* package_name = env->GetStringUTFChars(args->nice_name, nullptr);
        if (!package_name) {
            LOGE("Failed to get package name in postAppSpecialize");
            return;
        }
        auto it = package_map.find(package_name);
        if (it != package_map.end()) {
            current_info = it->second;
            LOGD("Post-specialize spoofing for %s: %s", package_name, current_info.model.c_str());
            spoofDevice(current_info);
            spoofSystemProperties(current_info);
        }
        env->ReleaseStringUTFChars(args->nice_name, package_name);
    }

private:
    zygisk::Api* api;
    JNIEnv* env;
    std::unordered_map<std::string, DeviceInfo> package_map;

    void loadConfig() {
        int fd = api->connectCompanion();
        if (fd < 0) {
            LOGE("Failed to connect to companion process: %s (errno=%d)", strerror(errno), errno);
            return;
        }

        size_t jsonSize = 0;
        if (xread(fd, &jsonSize, sizeof(size_t)) != sizeof(size_t)) {
            LOGE("Failed to read JSON size from companion: %s (errno=%d)", strerror(errno), errno);
            close(fd);
            return;
        }

        if (jsonSize == 0) {
            LOGE("No JSON data received from companion");
            close(fd);
            return;
        }

        std::vector<char> jsonData(jsonSize);
        if (xread(fd, jsonData.data(), jsonSize) != static_cast<ssize_t>(jsonSize)) {
            LOGE("Failed to read JSON data from companion: %s (errno=%d)", strerror(errno), errno);
            close(fd);
            return;
        }

        close(fd);

        try {
            json config = json::parse(jsonData);
            for (auto& [key, value] : config.items()) {
                if (key.find("_DEVICE") != std::string::npos) continue;
                auto packages = value.get<std::vector<std::string>>();
                std::string device_key = key + "_DEVICE";
                if (!config.contains(device_key)) {
                    LOGE("No device info for key %s", key.c_str());
                    continue;
                }
                auto device = config[device_key];

                DeviceInfo info;
                info.brand = device["BRAND"].get<std::string>();
                info.device = device["DEVICE"].get<std::string>();
                info.manufacturer = device["MANUFACTURER"].get<std::string>();
                info.model = device["MODEL"].get<std::string>();
                info.fingerprint = device.contains("FINGERPRINT") ?
                    device["FINGERPRINT"].get<std::string>() : "generic/brand/device:13/TQ3A.230805.001/123456:user/release-keys";
                info.build_id = device.contains("BUILD_ID") ? device["BUILD_ID"].get<std::string>() : "";
                info.display = device.contains("DISPLAY") ? device["DISPLAY"].get<std::string>() : "";
                info.product = device.contains("PRODUCT") ? device["PRODUCT"].get<std::string>() : info.device;
                info.version_release = device.contains("VERSION_RELEASE") ?
                    device["VERSION_RELEASE"].get<std::string>() : "";
                info.serial = device.contains("SERIAL") ? device["SERIAL"].get<std::string>() : "";
                info.cpuinfo = device.contains("CPUINFO") ? device["CPUINFO"].get<std::string>() : "";
                info.serial_content = device.contains("SERIAL_CONTENT") ? device["SERIAL_CONTENT"].get<std::string>() : "";

                for (const auto& pkg : packages) {
                    package_map[pkg] = info;
                    LOGD("Loaded package %s with model %s", pkg.c_str(), info.model.c_str());
                }
            }
            LOGD("Config loaded with %zu packages", package_map.size());
        } catch (const json::exception& e) {
            LOGE("JSON parsing error: %s", e.what());
        }
    }

    void spoofDevice(const DeviceInfo& info) {
        LOGD("Spoofing device: %s", info.model.c_str());
        if (modelField) {
            env->SetStaticObjectField(buildClass, modelField, env->NewStringUTF(info.model.c_str()));
            if (env->ExceptionCheck()) {
                env->ExceptionDescribe();
                env->ExceptionClear();
            }
        }
        if (brandField) {
            env->SetStaticObjectField(buildClass, brandField, env->NewStringUTF(info.brand.c_str()));
            if (env->ExceptionCheck()) {
                env->ExceptionDescribe();
                env->ExceptionClear();
            }
        }
        if (deviceField) {
            env->SetStaticObjectField(buildClass, deviceField, env->NewStringUTF(info.device.c_str()));
            if (env->ExceptionCheck()) {
                env->ExceptionDescribe();
                env->ExceptionClear();
            }
        }
        if (manufacturerField) {
            env->SetStaticObjectField(buildClass, manufacturerField, env->NewStringUTF(info.manufacturer.c_str()));
            if (env->ExceptionCheck()) {
                env->ExceptionDescribe();
                env->ExceptionClear();
            }
        }
        if (fingerprintField) {
            env->SetStaticObjectField(buildClass, fingerprintField, env->NewStringUTF(info.fingerprint.c_str()));
            if (env->ExceptionCheck()) {
                env->ExceptionDescribe();
                env->ExceptionClear();
            }
        }
        if (buildIdField && !info.build_id.empty()) {
            env->SetStaticObjectField(buildClass, buildIdField, env->NewStringUTF(info.build_id.c_str()));
            if (env->ExceptionCheck()) {
                env->ExceptionDescribe();
                env->ExceptionClear();
            }
        }
        if (displayField && !info.display.empty()) {
            env->SetStaticObjectField(buildClass, displayField, env->NewStringUTF(info.display.c_str()));
            if (env->ExceptionCheck()) {
                env->ExceptionDescribe();
                env->ExceptionClear();
            }
        }
        if (productField && !info.product.empty()) {
            env->SetStaticObjectField(buildClass, productField, env->NewStringUTF(info.product.c_str()));
            if (env->ExceptionCheck()) {
                env->ExceptionDescribe();
                env->ExceptionClear();
            }
        }
        if (versionReleaseField && !info.version_release.empty()) {
            env->SetStaticObjectField(versionClass, versionReleaseField, env->NewStringUTF(info.version_release.c_str()));
            if (env->ExceptionCheck()) {
                env->ExceptionDescribe();
                env->ExceptionClear();
            }
        }
        if (sdkIntField && !info.version_release.empty()) {
            env->SetStaticIntField(versionClass, sdkIntField, info.version_release == "13" ? 33 : 34);
            if (env->ExceptionCheck()) {
                env->ExceptionDescribe();
                env->ExceptionClear();
            }
        }
        if (serialField && !info.serial.empty()) {
            env->SetStaticObjectField(buildClass, serialField, env->NewStringUTF(info.serial.c_str()));
            if (env->ExceptionCheck()) {
                env->ExceptionDescribe();
                env->ExceptionClear();
            }
        }
    }

    void spoofSystemProperties(const DeviceInfo& info) {
        LOGD("Spoofing system properties for: %s", info.model.c_str());
        if (!info.brand.empty()) __system_property_set("ro.product.brand", info.brand.c_str());
        if (!info.device.empty()) __system_property_set("ro.product.device", info.device.c_str());
        if (!info.manufacturer.empty()) __system_property_set("ro.product.manufacturer", info.manufacturer.c_str());
        if (!info.model.empty()) __system_property_set("ro.product.model", info.model.c_str());
        if (!info.fingerprint.empty()) __system_property_set("ro.build.fingerprint", info.fingerprint.c_str());
    }

    static int hooked_prop_get(const char* name, char* value, const char* default_value) {
        if (!orig_prop_get) return -1;
        if (std::string(name) == "ro.product.brand" && !current_info.brand.empty()) {
            strncpy(value, current_info.brand.c_str(), PROP_VALUE_MAX);
            return current_info.brand.length();
        } else if (std::string(name) == "ro.product.device" && !current_info.device.empty()) {
            strncpy(value, current_info.device.c_str(), PROP_VALUE_MAX);
            return current_info.device.length();
        } else if (std::string(name) == "ro.product.manufacturer" && !current_info.manufacturer.empty()) {
            strncpy(value, current_info.manufacturer.c_str(), PROP_VALUE_MAX);
            return current_info.manufacturer.length();
        } else if (std::string(name) == "ro.product.model" && !current_info.model.empty()) {
            strncpy(value, current_info.model.c_str(), PROP_VALUE_MAX);
            return current_info.model.length();
        } else if (std::string(name) == "ro.build.fingerprint" && !current_info.fingerprint.empty()) {
            strncpy(value, current_info.fingerprint.c_str(), PROP_VALUE_MAX);
            return current_info.fingerprint.length();
        }
        return orig_prop_get(name, value, default_value);
    }

    void hookNativeGetprop() {
        if (!orig_prop_get) return;
        void* handle = dlopen("libc.so", RTLD_LAZY);
        if (handle) {
            void* sym = dlsym(handle, "__system_property_get");
            if (sym) {
                size_t page_size = sysconf(_SC_PAGE_SIZE);
                void* page_start = (void*)((uintptr_t)sym & ~(page_size - 1));
                if (mprotect(page_start, page_size, PROT_READ | PROT_WRITE | PROT_EXEC) == 0) {
                    *(void**)&orig_prop_get = (void*)hooked_prop_get;
                    mprotect(page_start, page_size, PROT_READ | PROT_EXEC);
                    LOGD("Successfully hooked __system_property_get");
                } else {
                    LOGE("Failed to mprotect for __system_property_get: %s", strerror(errno));
                }
            }
            dlclose(handle);
        } else {
            LOGE("Failed to open libc.so for property hook: %s", strerror(errno));
        }
    }

    static ssize_t hooked_read(int fd, void* buf, size_t count) {
        if (!orig_read) return -1;

        char path[256];
        ssize_t result = -1;
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
        void* handle = dlopen("libc.so", RTLD_LAZY);
        if (handle) {
            void* sym = dlsym(handle, "read");
            if (sym) {
                size_t page_size = sysconf(_SC_PAGE_SIZE);
                void* page_start = (void*)((uintptr_t)sym & ~(page_size - 1));
                if (mprotect(page_start, page_size, PROT_READ | PROT_WRITE | PROT_EXEC) == 0) {
                    *(void**)&orig_read = (void*)hooked_read;
                    mprotect(page_start, page_size, PROT_READ | PROT_EXEC);
                    LOGD("Successfully hooked read");
                } else {
                    LOGE("Failed to mprotect for read: %s", strerror(errno));
                }
            }
            dlclose(handle);
        } else {
            LOGE("Failed to open libc.so for read hook: %s", strerror(errno));
        }
    }

    static void hooked_set_static_object_field(JNIEnv* env, jclass clazz, jfieldID fieldID, jobject value) {
        if (clazz == buildClass) {
            if (fieldID == modelField || fieldID == brandField || fieldID == deviceField ||
                fieldID == manufacturerField || fieldID == fingerprintField || fieldID == buildIdField ||
                fieldID == displayField || fieldID == productField || fieldID == serialField) {
                LOGD("Blocked attempt to reset Build field");
                return;
            }
        } else if (clazz == versionClass) {
            if (fieldID == versionReleaseField) {
                LOGD("Blocked attempt to reset VERSION.RELEASE");
                return;
            }
        }
        if (orig_set_static_object_field) {
            orig_set_static_object_field(env, clazz, fieldID, value);
        }
    }

    void hookJniSetStaticObjectField() {
        void* handle = dlopen("libandroid_runtime.so", RTLD_LAZY);
        if (handle) {
            void* sym = dlsym(handle, "JNI_SetStaticObjectField");
            if (sym) {
                size_t page_size = sysconf(_SC_PAGE_SIZE);
                void* page_start = (void*)((uintptr_t)sym & ~(page_size - 1));
                if (mprotect(page_start, page_size, PROT_READ | PROT_WRITE | PROT_EXEC) == 0) {
                    orig_set_static_object_field = *(orig_set_static_object_field_t*)&sym;
                    *(void**)&sym = (void*)hooked_set_static_object_field;
                    mprotect(page_start, page_size, PROT_READ | PROT_EXEC);
                    LOGD("Successfully hooked JNI_SetStaticObjectField");
                } else {
                    LOGE("Failed to mprotect for JNI_SetStaticObjectField: %s", strerror(errno));
                }
            }
            dlclose(handle);
        } else {
            LOGE("Failed to open libandroid_runtime.so: %s", strerror(errno));
        }
    }
};

static void companion(int fd) {
    const char* config_path = "/data/adb/modules/COPG/config.json";
    std::vector<char> json;

    LOGD("Attempting to read config from: %s", config_path);
    FILE* file = fopen(config_path, "rb");
    if (file) {
        fseek(file, 0, SEEK_END);
        long size = ftell(file);
        fseek(file, 0, SEEK_SET);
        json.resize(size);
        size_t read_size = fread(json.data(), 1, size, file);
        fclose(file);
        if (read_size != static_cast<size_t>(size)) {
            LOGE("Failed to read config from %s: read %zu of %ld bytes", config_path, read_size, size);
            json.clear();
        } else {
            LOGD("Successfully read config from: %s", config_path);
        }
    } else {
        LOGE("Failed to open config at %s: %s (errno=%d)", config_path, strerror(errno), errno);
    }

    size_t jsonSize = json.size();
    xwrite(fd, &jsonSize, sizeof(size_t));
    if (jsonSize > 0) {
        xwrite(fd, json.data(), jsonSize);
    }
}

REGISTER_ZYGISK_MODULE(SpoofModule)
REGISTER_ZYGISK_COMPANION(companion)
