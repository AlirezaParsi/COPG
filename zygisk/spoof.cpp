#include <jni.h>
#include <string>
#include <android/log.h>
#include <zygisk.hh>

#define LOG_TAG "SpoofModule"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

#define COD_PACKAGE "com.activision.callofduty.shooter"
#define PUBG_PACKAGE "com.tencent.ig"
#define BGMI_PACKAGE "com.pubg.imobile"
#define FORTNITE_PACKAGE "com.epicgames.fortnite"          // Added Fortnite package
#define MLBB_PACKAGE "com.mobile.legends"                 // Added Mobile Legends package

class SpoofModule : public zygisk::ModuleBase {
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
            strcmp(package_name, BGMI_PACKAGE) == 0 ||
            strcmp(package_name, FORTNITE_PACKAGE) == 0 ||     // Added Fortnite check
            strcmp(package_name, MLBB_PACKAGE) == 0) {         // Added ML check
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
        } else if (strcmp(package_name, BGMI_PACKAGE) == 0) {
            spoofXiaomi();
        } else if (strcmp(package_name, FORTNITE_PACKAGE) == 0) {
            spoofSamsungS24Ultra();                           // Added Fortnite with S24 Ultra
        } else if (strcmp(package_name, MLBB_PACKAGE) == 0) {
            spoofSamsungS24Ultra();                           // Added Mobile Legends with S24 Ultra
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

    void spoofSamsungS24Ultra() {
        jclass buildClass = env->FindClass("android/os/Build");
        if (!buildClass) {
            LOGD("Failed to find Build class");
            return;
        }

        jfieldID modelField = env->GetStaticFieldID(buildClass, "MODEL", "Ljava/lang/String;");
        jfieldID brandField = env->GetStaticFieldID(buildClass, "BRAND", "Ljava/lang/String;");
        jfieldID deviceField = env->GetStaticFieldID(buildClass, "DEVICE", "Ljava/lang/String;");
        jfieldID manufacturerField = env->GetStaticFieldID(buildClass, "MANUFACTURER", "Ljava/lang/String;");

        env->SetStaticObjectField(buildClass, modelField, env->NewStringUTF("SM-S928B"));
        env->SetStaticObjectField(buildClass, brandField, env->NewStringUTF("samsung"));
        env->SetStaticObjectField(buildClass, deviceField, env->NewStringUTF("e3q"));
        env->SetStaticObjectField(buildClass, manufacturerField, env->NewStringUTF("samsung"));
        LOGD("Spoofed to Samsung Galaxy S24 Ultra (SM-S928B)");
    }
};

REGISTER_ZYGISK_MODULE(SpoofModule)
