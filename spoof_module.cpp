#include <jni.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <thread>
#include <atomic>
#include <android/log.h>
#include <zygisk.hpp>

#define LOG_TAG "FIFAHook"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

static const uintptr_t GOT_FILE_OFFSET = 0x6e74c78;

typedef int (*prop_get_t)(const char*, char*);
static std::atomic<prop_get_t> original_prop_get{nullptr};
static std::atomic<bool> hook_ready{false};

static int hooked_prop_get(const char* name, char* value) {
    prop_get_t orig = original_prop_get.load();
    
    // Safety check - اگه هنوز مقداردهی نشده
    if (!orig) {
        LOGI("Hook called before initialization, calling fallback");
        return 0;
    }
    
    if (!name) {
        return orig(name, value);
    }

    if (strcmp(name, "ro.product.model") == 0) {
        strcpy(value, "SM-F9460");
        LOGI("✅ Hooked: ro.product.model -> SM-F9460");
        return strlen(value);
    }
    if (strcmp(name, "ro.product.brand") == 0) {
        strcpy(value, "samsung");
        return strlen(value);
    }
    if (strcmp(name, "ro.product.manufacturer") == 0) {
        strcpy(value, "samsung");
        return strlen(value);
    }
    if (strcmp(name, "ro.product.device") == 0) {
        strcpy(value, "q2q");
        return strlen(value);
    }
    if (strcmp(name, "ro.build.fingerprint") == 0) {
        strcpy(value, "samsung/q2qzh/q2q:15/UP1A.231005.007/F946BXXU1BWK4:user/release-keys");
        return strlen(value);
    }
    if (strcmp(name, "ro.boot.vbmeta.device_state") == 0) {
        strcpy(value, "locked");
        return strlen(value);
    }
    if (strcmp(name, "ro.boot.verifiedbootstate") == 0) {
        strcpy(value, "green");
        return strlen(value);
    }

    return orig(name, value);
}

static bool findLoadBase(uintptr_t& load_base) {
    FILE* maps = fopen("/proc/self/maps", "r");
    if (!maps) {
        LOGE("Cannot open /proc/self/maps");
        return false;
    }

    char line[512];
    bool found = false;

    while (fgets(line, sizeof(line), maps)) {
        if (!strstr(line, "libFIFAMobileNeon.so")) continue;
        if (!strstr(line, "r--p")) continue;

        uintptr_t start, end;
        char perms[8];
        uintptr_t file_offset;
        sscanf(line, "%lx-%lx %s %lx", &start, &end, perms, &file_offset);

        if (file_offset == 0) {
            load_base = start;
            LOGI("Load base: 0x%lx", load_base);
            found = true;
            break;
        }
    }

    fclose(maps);
    return found;
}

static bool applyHook() {
    uintptr_t load_base = 0;
    if (!findLoadBase(load_base)) {
        LOGE("Failed to find load base");
        return false;
    }

    uintptr_t got_addr = load_base + GOT_FILE_OFFSET;
    LOGI("GOT address: 0x%lx", got_addr);

    prop_get_t orig = *(prop_get_t*)got_addr;
    if (!orig) {
        LOGE("original_prop_get is null!");
        return false;
    }
    
    // ذخیره تابع اصلی با atomic
    original_prop_get.store(orig);
    LOGI("Original __system_property_get: %p", orig);

    size_t page_size = getpagesize();
    uintptr_t page = got_addr & ~(page_size - 1);
    
    if (mprotect((void*)page, page_size, PROT_READ | PROT_WRITE) != 0) {
        LOGE("mprotect failed: %s", strerror(errno));
        return false;
    }

    *(prop_get_t*)got_addr = hooked_prop_get;

    if (mprotect((void*)page, page_size, PROT_READ) != 0) {
        LOGE("mprotect restore failed: %s", strerror(errno));
    }

    if (*(prop_get_t*)got_addr != hooked_prop_get) {
        LOGE("Hook verification failed!");
        return false;
    }

    LOGI("✅ Hook verified successfully!");
    hook_ready.store(true);

    char test[256] = {0};
    hooked_prop_get("ro.product.model", test);
    LOGI("Test: ro.product.model = %s", test);

    return true;
}

class FIFAModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api* api, JNIEnv* env) override {
        this->api = api;
        this->env = env;
        LOGI("FIFAHook module loaded");
    }

    void preAppSpecialize(zygisk::AppSpecializeArgs* args) override {
        if (!args || !args->nice_name) {
            api->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
            return;
        }

        const char* name = env->GetStringUTFChars(args->nice_name, nullptr);
        bool is_fifa = (name && strcmp(name, "com.ea.gp.fifamobile") == 0);
        if (name) env->ReleaseStringUTFChars(args->nice_name, name);

        if (!is_fifa) {
            api->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
            return;
        }

        LOGI("FIFA Mobile detected");
        needs_hook = true;
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs* args) override {
        if (!needs_hook) {
            api->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
            return;
        }

        LOGI("Starting hook thread...");

        std::thread([]() {
            LOGI("Hook thread running");

            for (int i = 0; i < 60; i++) {
                FILE* maps = fopen("/proc/self/maps", "r");
                if (!maps) {
                    sleep(1);
                    continue;
                }

                char line[512];
                bool loaded = false;
                while (fgets(line, sizeof(line), maps)) {
                    if (strstr(line, "libFIFAMobileNeon.so") &&
                        strstr(line, "r--p")) {
                        uintptr_t start, end;
                        char perms[8];
                        uintptr_t offset;
                        sscanf(line, "%lx-%lx %s %lx",
                               &start, &end, perms, &offset);
                        if (offset == 0) {
                            loaded = true;
                            break;
                        }
                    }
                }
                fclose(maps);

                if (loaded) {
                    LOGI("Library loaded after %d seconds", i);
                    sleep(1); // Wait for GOT to be filled
                    if (applyHook()) {
                        LOGI("✅ Hook applied successfully!");
                    } else {
                        LOGE("Hook failed!");
                    }
                    return;
                }

                sleep(1);
            }

            LOGE("Library never loaded after 60s");
        }).detach();
    }

private:
    zygisk::Api* api;
    JNIEnv* env;
    bool needs_hook = false;
};

REGISTER_ZYGISK_MODULE(FIFAModule)
