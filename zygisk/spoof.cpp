#include <jni.h>
#include <string>
#include <android/log.h>
#include <zygisk.hh>

#define LOG_TAG "SpoofModule"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

#define COD_PACKAGE "com.activision.callofduty.shooter"
#define PUBG_PACKAGE "com.tencent.ig"
#define BGMI_PACKAGE "com.pubg.imobile"  // Added BGMI package

static class SpoofModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api* api, JNIEnv* env) override {
        this->api = api;
        this->env = env;
        LOGD("SpoofModule loaded");
    }

    void preAppSpecialize(zygisk::AppSpecializeArgs* args) override {
        if (!args || !args->nice_name) {
            LOGD("No app info, skipping");
            api->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
            return;
        }

        const char* package_name = env->GetStringUTFChars(args->nice_name, nullptr);
        LOGD("Specializing app: %s", package_name);

        if (strcmp(package_name, COD_PACKAGE) == 0 || 
            strcmp(package_name, PUBG_PACKAGE) == 0 || 
            strcmp(package_name, BGMI_PACKAGE) == 0) {  // Added BGMI check
            LOGD("Target app detected, proceeding with spoofing");
        } else {
            api->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
        }

        env->ReleaseStringUTFChars(args->nice_name, package_name);
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs* args) override {
        if (!args || !args->nice_name) return;

        const char* package_name = env->GetStringUTFChars(args->nice_name, nullptr);
        if (!package_name) return;

        if (strcmp(package_name, COD_PACKAGE) == 0) {
            spoofLenovo();
        } else if (strcmp(package_name, PUBG_PACKAGE) == 0) {
            spoofXiaomi();
        } else if (strcmp(package_name, BGMI_PACKAGE) == 0) {  // Added BGMI handling
            spoofXiaomi();  // Using same spoof as PUBG Global, you can modify this
        }

        env->ReleaseStringUTFChars(args->nice_name, package_name);
    }

private:
    zygisk::Api* api;
    JNIEnv* env;

    void spoofLenovo() {
        jclass buildClass = env->FindClass("android/os/Build");
        if (!buildClass) {
            LOGD("Failed to find Build class");
            return;
        }

        jfieldID modelField = env->GetStaticFieldID(buildClass, "MODEL", "Ljava/lang/String;");
        jfieldID brandField = env->GetStaticFieldID(buildClass, "BRAND", "Ljava/lang/String;");
        jfieldID deviceField = env->GetStaticFieldID(buildClass, "DEVICE", "Ljava/lang/String;");
        jfieldID manufacturerField = env->GetStaticFieldID(buildClass, "MANUFACTURER", "Ljava/lang/String;");

        env->SetStaticObjectField(buildClass, modelField, env->NewStringUTF("Lenovo TB-9707F"));
        env->SetStaticObjectField(buildClass, brandField, env->NewStringUTF("Lenovo"));
        env->SetStaticObjectField(buildClass, deviceField, env->NewStringUTF("TB-9707F"));
        env->SetStaticObjectField(buildClass, manufacturerField, env->NewStringUTF("Lenovo"));
        LOGD("Spoofed to Lenovo TB-9707F");
    }

    void spoofXiaomi() {
        jclass buildClass = env->FindClass("android/os/Build");
        if (!buildClass) {
            LOGD("Failed to find Build class");
            return;
        }

        jfieldID modelField = env->GetStaticFieldID(buildClass, "MODEL", "Ljava/lang/String;");
        jfieldID brandField = env->GetStaticFieldID(buildClass, "BRAND", "Ljava/lang/String;");
        jfieldID deviceField = env->GetStaticFieldID(buildClass, "DEVICE", "Ljava/lang/String;");
        jfieldID manufacturerField = env->GetStaticFieldID(buildClass, "MANUFACTURER", "Ljava/lang/String;");

        env->SetStaticObjectField(buildClass, modelField, env->NewStringUTF("2210132G"));
        env->SetStaticObjectField(buildClass, brandField, env->NewStringUTF("Xiaomi"));
        env->SetStaticObjectField(buildClass, deviceField, env->NewStringUTF("2210132G"));
        env->SetStaticObjectField(buildClass, manufacturerField, env->NewStringUTF("Xiaomi"));
        LOGD("Spoofed to Xiaomi 2210132G");
    }
};

REGISTER_ZYGISK_MODULE(SpoofModule)
