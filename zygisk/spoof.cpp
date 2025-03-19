#include <jni.h>
#include <string>
#include <android/log.h>
#include <zygisk.hh>

#define LOG_TAG "SpoofModule"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

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

        if (isTargetPackage(package_name)) {
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

        // M11 Packages
        if (isPackageInList(package_name, m11_packages, 17)) {
            spoofDevice("ZTE", "NX769J", "ZTE", "NX769J");
        }
        // BS4 Packages
        else if (isPackageInList(package_name, bs4_packages, 1)) {
            spoofDevice("Black Shark", "Black Shark 4", "Xiaomi", "2SM-X706B");
        }
        // MI11TP Packages
        else if (isPackageInList(package_name, mi11tp_packages, 4)) {
            spoofDevice("Xiaomi", "Mi 11T PRO", "Xiaomi", "2107113SI");
        }
        // MI13P Packages
        else if (isPackageInList(package_name, mi13p_packages, 8)) {
            spoofDevice("Xiaomi", "Mi 13 Pro", "Xiaomi", "2210132G");
        }
        // OP8P Packages
        else if (isPackageInList(package_name, op8p_packages, 4)) {
            spoofDevice("OnePlus", "OnePlus 8 PRO", "OnePlus", "IN2020");
        }
        // OP9P Packages
        else if (isPackageInList(package_name, op9p_packages, 3)) {
            spoofDevice("OnePlus", "OnePlus 9 PRO", "OnePlus", "LE2101");
        }
        // F5 Packages
        else if (isPackageInList(package_name, f5_packages, 1)) {
            spoofDevice("POCO", "POCO F5", "Xiaomi", "23049PCD8G");
        }
        // ROG Packages
        else if (isPackageInList(package_name, rog_packages, 2)) {
            spoofDevice("Asus", "ROG Phone", "Asus", "ASUS_Z01QD");
        }
        // ROG6 Packages
        else if (isPackageInList(package_name, rog6_packages, 5)) {
            spoofDevice("Asus", "ROG Phone 6", "Asus", "ASUS_AI2201");
        }
        // Lenovo Tablet Packages
        else if (isPackageInList(package_name, lenovo_tablet_packages, 1)) {
            spoofDevice("Lenovo", "TB-9707F", "Lenovo", "Lenovo TB-9707F");
        }
        // Sony Xperia 5 Packages
        else if (isPackageInList(package_name, sony_xperia5_packages, 3)) {
            spoofDevice("Sony", "Xperia 5 IV", "Sony", "XQ-CQ54");
        }

        env->ReleaseStringUTFChars(args->nice_name, package_name);
    }

private:
    zygisk::Api* api;
    JNIEnv* env;

    // Package lists from config
    const char* m11_packages[17] = {
        "com.mobilelegends.mi", "com.supercell.brawlstars", "com.blizzard.diablo.immortal",
        "com.netease.newspike", "com.activision.callofduty.warzone", "com.pubg.newstate",
        "com.gamedevltd.destinywarfare", "com.pikpok.dr2.play", "com.CarXTech.highWay",
        "com.nekki.shadowfight3", "com.nekki.shadowfightarena", "com.gameloft.android.ANMP.GloftA8HM",
        "com.nekki.shadowfight", "com.ea.game.nfs14_row", "com.ea.games.r3_row",
        "com.supercell.squad", "com.blitzteam.battleprime"
    };

    const char* bs4_packages[1] = {"com.proximabeta.mf.uamo"};
    
    const char* mi11tp_packages[4] = {
        "com.ea.gp.apexlegendsmobilefps", "com.levelinfinite.hotta.gp",
        "com.supercell.clashofclans", "com.vng.mlbbvn"
    };

    const char* mi13p_packages[8] = {
        "com.levelinfinite.sgameGlobal", "com.tencent.tmgp.sgame",
        "com.pubg.imobile", "com.pubg.krmobile", "com.rekoo.pubgm",
        "com.tencent.ig", "com.tencent.tmgp.pubgmhd", "com.vng.pubgmobile"
    };

    const char* op8p_packages[4] = {
        "com.netease.lztgglobal", "com.riotgames.league.wildrift",
        "com.riotgames.league.wildrifttw", "com.riotgames.league.wildriftvn"
    };

    const char* op9p_packages[3] = {
        "com.epicgames.fortnite", "com.epicgames.portal", "com.tencent.lolm"
    };

    const char* f5_packages[1] = {"com.mobile.legends"};

    const char* rog_packages[2] = {"com.dts.freefireth", "com.dts.freefirethmax"};

    const char* rog6_packages[5] = {
        "com.ea.gp.fifamobile", "com.gameloft.android.ANMP.GloftA9HM",
        "com.madfingergames.legends", "com.pearlabyss.blackdesertm",
        "com.pearlabyss.blackdesertm.gl"
    };

    const char* lenovo_tablet_packages[1] = {"com.activision.callofduty.shooter"};

    const char* sony_xperia5_packages[3] = {
        "com.garena.game.codm", "com.tencent.tmgp.kr.codm", "com.vng.codmvn"
    };

    bool isPackageInList(const char* package, const char* list[], int size) {
        for (int i = 0; i < size; i++) {
            if (strcmp(package, list[i]) == 0) return true;
        }
        return false;
    }

    bool isTargetPackage(const char* package) {
        return isPackageInList(package, m11_packages, 17) ||
               isPackageInList(package, bs4_packages, 1) ||
               isPackageInList(package, mi11tp_packages, 4) ||
               isPackageInList(package, mi13p_packages, 8) ||
               isPackageInList(package, op8p_packages, 4) ||
               isPackageInList(package, op9p_packages, 3) ||
               isPackageInList(package, f5_packages, 1) ||
               isPackageInList(package, rog_packages, 2) ||
               isPackageInList(package, rog6_packages, 5) ||
               isPackageInList(package, lenovo_tablet_packages, 1) ||  
               isPackageInList(package, sony_xperia5_packages, 3);
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
