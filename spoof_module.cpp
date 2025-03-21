#include <sys/types.h>
#include <zygisk.hh>
#include <android/log.h>
#include <dlfcn.h>
#include <string>
#include <sys/system_properties.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <unordered_set>

#define LOG_TAG "SpoofModule"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

using json = nlohmann::json;

class SpoofModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api* api, JNIEnv* env) override {
        this->api = api;
        this->env = env;
        loadConfig();
    }

    void preAppSpecialize(zygisk::AppSpecializeArgs* args) override {
        if (!args || !args->nice_name) {
            api->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
            return;
        }

        const char* package_name = env->GetStringUTFChars(args->nice_name, nullptr);
        if (package_set.find(package_name) == package_set.end()) {
            LOGD("Skipping non-target package: %s", package_name);
            api->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
        } else {
            char abi[PROP_VALUE_MAX];
            __system_property_get("ro.product.cpu.abi", abi);
            const char* gadget_path = strstr(abi, "arm64") ?
                "/data/adb/modules/COPG/frida/libcopg-arm64.so" :
                "/data/adb/modules/COPG/frida/libcopg-armv7-a.so";
            
            void* handle = dlopen(gadget_path, RTLD_NOW);
            if (!handle) {
                LOGD("Failed to load Frida Gadget for %s: %s", package_name, dlerror());
            } else {
                LOGD("Frida Gadget 16.7.0 loaded for %s", package_name);
            }
        }

        env->ReleaseStringUTFChars(args->nice_name, package_name);
    }

private:
    zygisk::Api* api;
    JNIEnv* env;
    std::unordered_set<std::string> package_set;

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
                for (const auto& pkg : value.get<std::vector<std::string>>()) {
                    package_set.insert(pkg);
                }
            }
            LOGD("Loaded %zu target packages", package_set.size());
        } catch (const json::exception& e) {
            LOGD("JSON parse error: %s", e.what());
        }

        file.close();
    }
};

REGISTER_ZYGISK_MODULE(SpoofModule)
