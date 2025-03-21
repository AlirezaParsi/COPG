#include <jni.h>
#include <string>
#include <android/log.h>
#include <zygisk.hh>
#include <nlohmann/json.hpp>
#include <fstream>
#include <unordered_map>
#include <sys/system_properties.h>
#include <dlfcn.h>

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, "SpoofModule", __VA_ARGS__)

using json = nlohmann::json;

struct DeviceInfo {
    std::string brand;
    std::string device;
    std::string manufacturer;
    std::string model;
    std::string fingerprint;
};

typedef int (*orig_prop_get_t)(const char*, char*, const char*);
static orig_prop_get_t orig_prop_get = nullptr;
static DeviceInfo current_info;

class SpoofModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api* api, JNIEnv* env) override {
        this->api = api;
        this->env = env;
        LOGD("SpoofModule loaded");

        buildClass = env->FindClass("android/os/Build");
        if (buildClass) {
            modelField = env->GetStaticFieldID(buildClass, "MODEL", "Ljava/lang/String;");
            brandField = env->GetStaticFieldID(buildClass, "BRAND", "Ljava/lang/String;");
            deviceField = env->GetStaticFieldID(buildClass, "DEVICE", "Ljava/lang/String;");
            manufacturerField = env->GetStaticFieldID(buildClass, "MANUFACTURER", "Ljava/lang/String;");
            fingerprintField = env->GetStaticFieldID(buildClass, "FINGERPRINT", "Ljava/lang/String;");
        } else {
            LOGD("Failed to find Build class");
        }

        void* handle = dlopen("libc.so", RTLD_LAZY);
        if (handle) {
            orig_prop_get = (orig_prop_get_t)dlsym(handle, "__system_property_get");
            dlclose(handle);
        }
        loadConfig();
    }

    void preAppSpecialize(zygisk::AppSpecializeArgs* args) override {
        if (!args || !args->nice_name) {
            LOGD("No nice_name");
            return;
        }
        const char* package_name = env->GetStringUTFChars(args->nice_name, nullptr);
        if (!package_name) {
            LOGD("Failed to get package_name");
            return;
        }
        LOGD("Checking package: %s", package_name);
        auto it = package_map.find(package_name);
        if (it != package_map.end()) {
            current_info = it->second;
            spoofDevice(current_info);
            spoofSystemProperties(current_info);
            hookNativeGetprop();
            LOGD("Spoofed package: %s", package_name);
        }
        env->ReleaseStringUTFChars(args->nice_name, package_name);
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs* args) override {
        if (!args || !args->nice_name || package_map.empty() || !buildClass) return;
        const char* package_name = env->GetStringUTFChars(args->nice_name, nullptr);
        if (!package_name) return;
        auto it = package_map.find(package_name);
        if (it != package_map.end()) {
            current_info = it->second;
            spoofDevice(current_info);
            spoofSystemProperties(current_info);
            hookNativeGetprop();
            jstring model = (jstring)env->GetStaticObjectField(buildClass, modelField);
            const char* modelStr = env->GetStringUTFChars(model, nullptr);
            LOGD("Applied model for %s: %s", package_name, modelStr ? modelStr : "null");
            if (modelStr) env->ReleaseStringUTFChars(model, modelStr);
        }
        env->ReleaseStringUTFChars(args->nice_name, package_name);
    }

private:
    zygisk::Api* api;
    JNIEnv* env;
    std::unordered_map<std::string, DeviceInfo> package_map;
    jclass buildClass = nullptr;
    jfieldID modelField = nullptr, brandField = nullptr, deviceField = nullptr, manufacturerField = nullptr, fingerprintField = nullptr;

    void loadConfig() {
        std::ifstream file("/data/adb/modules/COPG/config.json");
        if (!file.is_open()) {
            LOGD("Failed to open config.json");
            return;
        }
        try {
            json config = json::parse(file);
            for (auto& [key, value] : config.items()) {
                if (key.find("_DEVICE") != std::string::npos) continue;
                auto packages = value.get<std::vector<std::string>>();
                std::string device_key = key + "_DEVICE";
                if (!config.contains(device_key)) continue;
                auto device = config[device_key];
                DeviceInfo info {
                    device["BRAND"].get<std::string>(),
                    device["DEVICE"].get<std::string>(),
                    device["MANUFACTURER"].get<std::string>(),
                    device["MODEL"].get<std::string>(),
                    device.value("FINGERPRINT", "generic/brand/device:13/TQ3A.230805.001/123456:user/release-keys")
                };
                for (const auto& pkg : packages) {
                    package_map[pkg] = info;
                    LOGD("Mapped package: %s", pkg.c_str());
                }
            }
        } catch (const json::exception& e) {
            LOGD("JSON parse error: %s", e.what());
        }
        file.close();
    }

    void spoofDevice(const DeviceInfo& info) {
        if (modelField && !info.model.empty()) env->SetStaticObjectField(buildClass, modelField, env->NewStringUTF(info.model.c_str()));
        if (brandField && !info.brand.empty()) env->SetStaticObjectField(buildClass, brandField, env->NewStringUTF(info.brand.c_str()));
        if (deviceField && !info.device.empty()) env->SetStaticObjectField(buildClass, deviceField, env->NewStringUTF(info.device.c_str()));
        if (manufacturerField && !info.manufacturer.empty()) env->SetStaticObjectField(buildClass, manufacturerField, env->NewStringUTF(info.manufacturer.c_str()));
        if (fingerprintField && !info.fingerprint.empty()) env->SetStaticObjectField(buildClass, fingerprintField, env->NewStringUTF(info.fingerprint.c_str()));
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
        if (orig_prop_get) {
            // Simplified hook; test and refine if needed
            void* handle = dlopen("libc.so", RTLD_LAZY);
            if (handle) {
                void* sym = dlsym(handle, "__system_property_get");
                if (sym) *(void**)&orig_prop_get = (void*)hooked_prop_get; // Caution: May need mprotect
                dlclose(handle);
            }
        }
    }
};

REGISTER_ZYGISK_MODULE(SpoofModule)
