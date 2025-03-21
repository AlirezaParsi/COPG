#include <jni.h>
#include <string>
#include <zygisk.hh>
#include <nlohmann/json.hpp>
#include <fstream>
#include <unordered_map>
#include <sys/system_properties.h>
#include <dlfcn.h>

using json = nlohmann::json;

struct DeviceInfo {
    std::string brand;
    std::string device;
    std::string manufacturer;
    std::string model;
    std::string fingerprint; // Re-added to match your config
};

// Function pointer for original getprop
typedef int (*orig_prop_get_t)(const char*, char*, const char*);
static orig_prop_get_t orig_prop_get = nullptr;
static DeviceInfo current_info; // Store current spoofed info for native hooking

class SpoofModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api* api, JNIEnv* env) override {
        this->api = api;
        this->env = env;
        buildClass = env->FindClass("android/os/Build");
        if (buildClass) {
            modelField = env->GetStaticFieldID(buildClass, "MODEL", "Ljava/lang/String;");
            brandField = env->GetStaticFieldID(buildClass, "BRAND", "Ljava/lang/String;");
            deviceField = env->GetStaticFieldID(buildClass, "DEVICE", "Ljava/lang/String;");
            manufacturerField = env->GetStaticFieldID(buildClass, "MANUFACTURER", "Ljava/lang/String;");
            fingerprintField = env->GetStaticFieldID(buildClass, "FINGERPRINT", "Ljava/lang/String;"); // Added
        }

        // Hook native getprop function
        void* handle = dlopen("libc.so", RTLD_LAZY);
        if (handle) {
            orig_prop_get = (orig_prop_get_t)dlsym(handle, "__system_property_get");
            dlclose(handle);
        }

        loadConfig();
    }

    void preAppSpecialize(zygisk::AppSpecializeArgs* args) override {
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
            spoofDevice(current_info.brand.c_str(), current_info.device.c_str(), 
                       current_info.manufacturer.c_str(), current_info.model.c_str(), 
                       current_info.fingerprint.c_str());
            spoofSystemProperties(current_info);
            hookNativeGetprop();
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
            spoofDevice(current_info.brand.c_str(), current_info.device.c_str(), 
                       current_info.manufacturer.c_str(), current_info.model.c_str(), 
                       current_info.fingerprint.c_str());
            spoofSystemProperties(current_info);
            hookNativeGetprop();
        }
        env->ReleaseStringUTFChars(args->nice_name, package_name);
    }

private:
    zygisk::Api* api;
    JNIEnv* env;
    std::unordered_map<std::string, DeviceInfo> package_map;
    jclass buildClass = nullptr;
    jfieldID modelField = nullptr, brandField = nullptr, deviceField = nullptr, 
             manufacturerField = nullptr, fingerprintField = nullptr; // Added fingerprintField

    void loadConfig() {
        std::ifstream file("/data/adb/modules/COPG/config.json");
        if (!file.is_open()) return;
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
                    device.value("FINGERPRINT", "generic/brand/device:13/TQ3A.230805.001/123456:user/release-keys") // Added with fallback
                };
                for (const auto& pkg : packages) package_map[pkg] = info;
            }
        } catch (const json::exception&) {}
        file.close();
    }

    void spoofDevice(const char* brand, const char* device, const char* manufacturer, 
                    const char* model, const char* fingerprint) {
        if (modelField) env->SetStaticObjectField(buildClass, modelField, env->NewStringUTF(model));
        if (brandField) env->SetStaticObjectField(buildClass, brandField, env->NewStringUTF(brand));
        if (deviceField) env->SetStaticObjectField(buildClass, deviceField, env->NewStringUTF(device));
        if (manufacturerField) env->SetStaticObjectField(buildClass, manufacturerField, env->NewStringUTF(manufacturer));
        if (fingerprintField) env->SetStaticObjectField(buildClass, fingerprintField, env->NewStringUTF(fingerprint)); // Added
    }

    void spoofSystemProperties(const DeviceInfo& info) {
        if (!info.brand.empty()) __system_property_set("ro.product.brand", info.brand.c_str());
        if (!info.device.empty()) __system_property_set("ro.product.device", info.device.c_str());
        if (!info.manufacturer.empty()) __system_property_set("ro.product.manufacturer", info.manufacturer.c_str());
        if (!info.model.empty()) __system_property_set("ro.product.model", info.model.c_str());
        if (!info.fingerprint.empty()) __system_property_set("ro.build.fingerprint", info.fingerprint.c_str()); // Added
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
        } else if (std::string(name) == "ro.build.fingerprint" && !current_info.fingerprint.empty()) { // Added
            strncpy(value, current_info.fingerprint.c_str(), PROP_VALUE_MAX);
            return current_info.fingerprint.length();
        }
        return orig_prop_get(name, value, default_value);
    }

    void hookNativeGetprop() {
        if (orig_prop_get) {
            void* handle = dlopen("libc.so", RTLD_LAZY);
            if (handle) {
                void* sym = dlsym(handle, "__system_property_get");
                if (sym) {
                    // Simplified hooking; may need memory protection adjustments
                    *(void**)&orig_prop_get = (void*)hooked_prop_get;
                }
                dlclose(handle);
            }
        }
    }
};

REGISTER_ZYGISK_MODULE(SpoofModule)
