#include <jni.h>
#include <string>
#include <android/log.h>
#include <zygisk.hh>
#include <nlohmann/json.hpp>
#include <fstream>
#include <unordered_map>

#define LOG_TAG "SpoofModule"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

using json = nlohmann::json;

struct DeviceInfo {
    std::string brand;
    std::string device;
    std::string manufacturer;
    std::string model;
};

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
        } else {
            LOGD("Failed to find android/os/Build class");
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
    jclass buildClass = nullptr;
    jfieldID modelField = nullptr, brandField = nullptr, deviceField = nullptr, manufacturerField = nullptr;

    void loadConfig() {
        std::ifstream file("/data/adb/modules/COPG/config.json");
        if (!file.is_open()) {
            LOGD("Failed to open config.json - please update via action script");
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
                    device["MODEL"].get<std::string>()
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
};

REGISTER_ZYGISK_MODULE(SpoofModule)
