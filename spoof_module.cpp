#include <jni.h>
#include <string>
#include <zygisk.hh>
#include <nlohmann/json.hpp>
#include <fstream>
#include <unordered_map>
#include <sys/system_properties.h>
#include <dlfcn.h>
#include "AndroidJNIHelper.hpp" // Adjust based on actual header

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

// Global variables (unchanged)
static DeviceInfo current_info;
static jclass buildClass = nullptr;
static jclass versionClass = nullptr;
static jfieldID modelField = nullptr;
// ... (other fields unchanged)

class SpoofModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api* api, JNIEnv* env) override {
        this->api = api;
        this->env = env;

        // Initialize JNI fields (unchanged)
        if (!buildClass) {
            buildClass = (jclass)env->NewGlobalRef(env->FindClass("android/os/Build"));
            if (buildClass) {
                modelField = env->GetStaticFieldID(buildClass, "MODEL", "Ljava/lang/String;");
                brandField = env->GetStaticFieldID(buildClass, "BRAND", "Ljava/lang/String;");
                // ... (other fields unchanged)
            }
        }
        if (!versionClass) {
            versionClass = (jclass)env->NewGlobalRef(env->FindClass("android/os/Build$VERSION"));
            if (versionClass) {
                versionReleaseField = env->GetStaticFieldID(versionClass, "RELEASE", "Ljava/lang/String;");
                sdkIntField = env->GetStaticFieldID(versionClass, "SDK_INT", "I");
            }
        }

        // Initialize original function pointers
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

        // Set up hooks with Android-JNI-Helper
        hookNativeGetprop();
        hookNativeRead();
        hookJniSetStaticObjectField();
        hookStat(); // New: hide module files
        loadConfig();
    }

    void preAppSpecialize(zygisk::AppSpecializeArgs* args) override {
        // Unchanged logic
        if (!args || !args->nice_name) {
            api->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
            return;
        }
        const char* package_name = env->GetStringUTFChars(args->nice_name, nullptr);
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
        env->ReleaseStringUTFChars(args->nice_name, package_name);
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs* args) override {
        // Unchanged logic
        if (!args || !args->nice_name || package_map.empty() || !buildClass) return;
        const char* package_name = env->GetStringUTFChars(args->nice_name, nullptr);
        if (!package_name) return;
        auto it = package_map.find(package_name);
        if (it != package_map.end()) {
            current_info = it->second;
            spoofDevice(current_info);
            spoofSystemProperties(current_info);
        }
        env->ReleaseStringUTFChars(args->nice_name, package_name);
    }

private:
    zygisk::Api* api;
    JNIEnv* env;
    std::unordered_map<std::string, DeviceInfo> package_map;

    // Manual string obfuscation (if library doesn’t provide it)
    std::string obfuscateString(const std::string& input) {
        std::string result = input;
        const char key = 0x5A;
        for (char& c : result) c ^= key;
        return result;
    }

    void loadConfig() {
        std::string obfuscatedPath = obfuscateString("/data/adb/modules/COPG/config.json");
        std::string realPath = obfuscateString(obfuscatedPath); // Reverse XOR
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
                // ... (rest unchanged)
                for (const auto& pkg : packages) package_map[pkg] = info;
            }
        } catch (const json::exception&) {}
        file.close();
    }

    void spoofDevice(const DeviceInfo& info) {
        // Unchanged logic
        if (modelField) env->SetStaticObjectField(buildClass, modelField, env->NewStringUTF(info.model.c_str()));
        // ... (rest unchanged)
    }

    void spoofSystemProperties(const DeviceInfo& info) {
        // Unchanged logic
        if (!info.brand.empty()) __system_property_set("ro.product.brand", info.brand.c_str());
        // ... (rest unchanged)
    }

    static int hooked_prop_get(const char* name, char* value, const char* default_value) {
        // Unchanged logic
        if (!orig_prop_get) return -1;
        if (std::string(name) == "ro.product.brand" && !current_info.brand.empty()) {
            strncpy(value, current_info.brand.c_str(), PROP_VALUE_MAX);
            return current_info.brand.length();
        }
        return orig_prop_get(name, value, default_value);
    }

    void hookNativeGetprop() {
        if (!orig_prop_get) return;
        AndroidJNIHelper::HookFunction("libc.so", "__system_property_get",
                                      (void*)hooked_prop_get,
                                      (void**)&orig_prop_get);
    }

    static ssize_t hooked_read(int fd, void* buf, size_t count) {
        // Unchanged logic
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
            }
        }
        return orig_read(fd, buf, count);
    }

    void hookNativeRead() {
        if (!orig_read) return;
        AndroidJNIHelper::HookFunction("libc.so", "read",
                                      (void*)hooked_read,
                                      (void**)&orig_read);
    }

    static void hooked_set_static_object_field(JNIEnv* env, jclass clazz, jfieldID fieldID, jobject value) {
        // Unchanged logic
        if (clazz == buildClass) {
            if (fieldID == modelField || fieldID == brandField) return;
        }
        if (orig_set_static_object_field) {
            orig_set_static_object_field(env, clazz, fieldID, value);
        }
    }

    void hookJniSetStaticObjectField() {
        if (!orig_set_static_object_field) return;
        AndroidJNIHelper::HookJNIMethod(env, "JNI_SetStaticObjectField",
                                       (void*)hooked_set_static_object_field,
                                       (void**)&orig_set_static_object_field);
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
        AndroidJNIHelper::HookFunction("libc.so", "stat",
                                      (void*)hooked_stat,
                                      (void**)&orig_stat);
    }
};

REGISTER_ZYGISK_MODULE(SpoofModule)
