#include <jni.h>
#include <string>
#include <android/log.h>
#include <zygisk.hh>
#include <nlohmann/json.hpp>
#include <fstream>
#include <unordered_map>
#include <dobby.h>

#define LOG_TAG "SpoofModule"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

using json = nlohmann::json;

struct DeviceInfo {
    std::string brand;
    std::string device;
    std::string manufacturer;
    std::string model;
    std::string ro_product_model; 
};

class SpoofModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api* api, JNIEnv* env) override {
        this->api = api;
        this->env = env;
        buildClass = env->FindClass("android/os/Build");
        if (buildClass) {
            modelField = env->GetStaticFieldID 
   
(buildClass, "MODEL", "Ljava/lang/String;");
            brandField = env->GetStaticFieldID(buildClass, "BRAND", "Ljava/lang/String;");
            deviceField = env->GetStaticFieldID(buildClass, "DEVICE", "Ljava/lang/String;");
            manufacturerField = env->GetStaticFieldID(buildClass, "MANUFACTURER", "Ljava/lang/String;");
        }

        // Setup System.getProperty hook
        jclass systemClass = env->FindClass("java/lang/System");
        if (systemClass) {
            getPropertyMethod = env->GetStaticMethodID(systemClass, "getProperty", "(Ljava/lang/String;)Ljava/lang/String;");
            if (getPropertyMethod) {
                DobbyHook((void*)getPropertyMethod, (void*)hook_System_getProperty, (void**)&original_getProperty);
                LOGD("Hooked System.getProperty successfully");
            } else {
                LOGD("Failed to find getProperty method");
            }
        } else {
            LOGD("Failed to find java/lang/System class");
        }

        loadConfig();
    }

    void preAppSpecialize(zygisk::AppSpecializeArgs* args) override {
        if (!args || !args->nice_name) {
            api->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
            return;
        }

        const char* package_name = env->GetStringUTFChars(args->nice_name, nullptr);
        if (package_map.find(package_name) == package_map.end()) {
            api->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
        } else {
            current_package = package_name; // Store for hook
        }

        env->ReleaseStringUTFChars(args->nice_name, package_name);
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs* args) override {
        if (!args || !args->nice_name || package_map.empty() || !buildClass) return;

        const char* package_name = env->GetStringUTFChars(args->nice_name, nullptr);
        if (!package_name) return;

        auto it = package_map.find(package_name);
        if (it != package_map.end()) {
            const DeviceInfo& info = it->second;
            spoofDevice(info.brand.c_str(), info.device.c_str(), info.manufacturer.c_str(), info.model.c_str());
        }

        env->ReleaseStringUTFChars(args->nice_name, package_name);
    }

private:
    zygisk::Api* api;
    JNIEnv* env;
    std::unordered_map<std::string, DeviceInfo> package_map;
    std::string current_package; 
    jclass buildClass = nullptr;
    jfieldID modelField = nullptr, brandField = nullptr, deviceField = nullptr, manufacturerField = nullptr;
    static jmethodID getPropertyMethod;
    static void* original_getProperty; 

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
                    device.value("ro.product.model", "") 
                };

                for (const auto& pkg : packages) {
                    package_map[pkg] = info;
                }
            }
            LOGD("Loaded %zu package mappings", package_map.size());
        } catch (const json::exception& e) {
            LOGD("JSON parse error: %s", e.what());
        }

        file.close();
    }

    void spoofDevice(const char* brand, const char* device, const char* manufacturer, const char* model) {
        if (modelField) env->SetStaticObjectField(buildClass, modelField, env->NewStringUTF(model));
        if (brandField) env->SetStaticObjectField(buildClass, brandField, env->NewStringUTF(brand));
        if (deviceField) env->SetStaticObjectField(buildClass, deviceField, env->NewStringUTF(device));
        if (manufacturerField) env->SetStaticObjectField(buildClass, manufacturerField, env->NewStringUTF(manufacturer));
    }

    // Hook function for System.getProperty
    static jstring hook_System_getProperty(JNIEnv* env, jclass clazz, jstring key) {
        SpoofModule* instance = getInstance(); // Access instance for package_map
        if (!instance || instance->current_package.empty()) {
            return static_cast<jstring>(DobbyCall(instance->original_getProperty, env, clazz, key));
        }

        const char* key_str = env->GetStringUTFChars(key, nullptr);
        if (strcmp(key_str, "ro.product.model") == 0) {
            auto it = instance->package_map.find(instance->current_package);
            if (it != instance->package_map.end() && !it->second.ro_product_model.empty()) {
                env->ReleaseStringUTFChars(key, key_str);
                return env->NewStringUTF(it->second.ro_product_model.c_str());
            }
        }

        env->ReleaseStringUTFChars(key, key_str);
        return static_cast<jstring>(DobbyCall(instance->original_getProperty, env, clazz, key));
    }

    // Helper to get instance (since hook is static)
    static SpoofModule* getInstance() {
        static SpoofModule* instance = nullptr;
        if (!instance) {
            // This is a simplification; in practice, you'd need a safer way to set this
            // For now, assume it's set during onLoad
        }
        return instance;
    }

public:
    // Set instance during construction
    SpoofModule() {
        getInstance() = this; // Hacky, see notes below
    }
};

// Static member initialization
jmethodID SpoofModule::getPropertyMethod = nullptr;
void* SpoofModule::original_getProperty = nullptr;

REGISTER_ZYGISK_MODULE(SpoofModule)
