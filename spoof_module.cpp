#include <jni.h>
#include <string>
#include <android/log.h>
#include <zygisk.hh>
#include <nlohmann/json.hpp>
#include <fstream>

#define LOG_TAG "SpoofModule"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

using json = nlohmann::json;

class SpoofModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api* api, JNIEnv* env) override {
        this->api = api;
        this->env = env;
        LOGD("SpoofModule loaded");
        loadConfig();
    }

    void preAppSpecialize(zygisk::AppSpecializeArgs* args) override {
        if (!args || !args->nice_name) {
            LOGD("No app info, skipping");
            api->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
            return;
        }

        const char* package_name = env->GetStringUTFChars(args->nice_name, nullptr);
        LOGD("Specializing app: %s", package_name);

        if (isTargetPackage(package_name)) {
            LOGD("Target app detected, proceeding with spoofing");
        } else {
            api->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
        }

        env->ReleaseStringUTFChars(args->nice_name, package_name);
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs* args) override {
        if (!args || !args->nice_name || config.empty()) return;

        const char* package_name = env->GetStringUTFChars(args->nice_name, nullptr);
        if (!package_name) return;

        for (auto& [key, value] : config.items()) {
            if (key.find("_DEVICE") != std::string::npos) continue;

            auto packages = value.get<std::vector<std::string>>();
            for (const auto& pkg : packages) {
                if (strcmp(package_name, pkg.c_str()) == 0) {
                    std::string device_key = key + "_DEVICE";
                    if (config.contains(device_key)) {
                        auto device = config[device_key];
                        spoofDevice(
                            device["BRAND"].get<std::string>().c_str(),
                            device["DEVICE"].get<std::string>().c_str(),
                            device["MANUFACTURER"].get<std::string>().c_str(),
                            device["MODEL"].get<std::string>().c_str()
                        );
                        break;
                    }
                }
            }
        }

        env->ReleaseStringUTFChars(args->nice_name, package_name);
    }

private:
    zygisk::Api* api;
    JNIEnv* env;
    json config;

    void loadConfig() {
        std::ifstream file("/data/adb/modules/COPG/config.json");
        if (!file.is_open()) {
            LOGD("Failed to open config.json");
            return;
        }

        try {
            config = json::parse(file);
            LOGD("Config loaded successfully");
        } catch (const json::exception& e) {
            LOGD("JSON parse error: %s", e.what());
        }

        file.close();
    }

    bool isTargetPackage(const char* package) {
        if (config.empty()) return false;

        for (auto& [key, value] : config.items()) {
            if (key.find("_DEVICE") != std::string::npos) continue;

            auto packages = value.get<std::vector<std::string>>();
            for (const auto& pkg : packages) {
                if (strcmp(package, pkg.c_str()) == 0) return true;
            }
        }
        return false;
    }

    void spoofDevice(const char* brand, const char* device, const char* manufacturer, const char* model) {
        jclass buildClass = env->FindClass("android/os/Build");
        if (!buildClass) {
            LOGD("Failed to find Build class");
            return;
        }

        jfieldID modelField = env->GetStaticFieldID(buildClass, "MODEL", "Ljava/lang/String;");
        jfieldID brandField = env->GetStaticFieldID(buildClass, "BRAND", "Ljava/lang/String;");
        jfieldID deviceField = env->GetStaticFieldID(buildClass, "DEVICE", "Ljava/lang/String;");
        jfieldID manufacturerField = env->GetStaticFieldID(buildClass, "MANUFACTURER", "Ljava/lang/String;");

        env->SetStaticObjectField(buildClass, modelField, env->NewStringUTF(model));
        env->SetStaticObjectField(buildClass, brandField, env->NewStringUTF(brand));
        env->SetStaticObjectField(buildClass, deviceField, env->NewStringUTF(device));
        env->SetStaticObjectField(buildClass, manufacturerField, env->NewStringUTF(manufacturer));
        LOGD("Spoofed to %s %s (%s)", brand, device, model);
    }
};

REGISTER_ZYGISK_MODULE(SpoofModule)
