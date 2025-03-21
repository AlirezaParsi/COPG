#include <sys/types.h> // Add this for dev_t and ino_t
#include <zygisk.hh>
#include <android/log.h>
#include <dlfcn.h>
#include <string>
#include <sys/system_properties.h>

#define LOG_TAG "SpoofModule"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

class SpoofModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api* api, JNIEnv* env) override {
        this->api = api;
        this->env = env;
    }

    void preAppSpecialize(zygisk::AppSpecializeArgs* args) override {
        if (!args || !args->nice_name) {
            api->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
            return;
        }

        const char* package_name = env->GetStringUTFChars(args->nice_name, nullptr);
        if (strcmp(package_name, "com.activision.callofduty.shooter") != 0) {
            api->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
        } else {
            char abi[PROP_VALUE_MAX];
            __system_property_get("ro.product.cpu.abi", abi);
            const char* gadget_path = strstr(abi, "arm64") ?
                "/data/adb/modules/COPG/frida/libcopg-arm64.so" :
                "/data/adb/modules/COPG/frida/libcopg-armv7-a.so";
            
            void* handle = dlopen(gadget_path, RTLD_NOW);
            if (!handle) {
                LOGD("Failed to load Frida Gadget: %s", dlerror());
            } else {
                LOGD("Frida Gadget 16.7.0 loaded for COD");
            }
        }

        env->ReleaseStringUTFChars(args->nice_name, package_name);
    }

private:
    zygisk::Api* api;
    JNIEnv* env;
};

REGISTER_ZYGISK_MODULE(SpoofModule)
