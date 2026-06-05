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

// از readelf گرفتیم
static const uintptr_t GOT_FILE_OFFSET = 0x6e74c78;

typedef int (*prop_get_t)(const char*, char*);
static prop_get_t original_prop_get = nullptr;
static std::atomic<bool> hook_ready{false};

static int hooked_prop_get(const char* name, char* value) {
    // اول original رو صدا بزن
    int result = original_prop_get(name, value);

    if (!name) return result;

    if (strcmp(name, "ro.product.model") == 0) {
        strcpy(value, "SM-F9460");
        LOGI("Hooked: ro.product.model -> SM-F9460");
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

    return result;
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

        // اولین r--p = load base (offset 0 در فایل)
        if (!strstr(line, "r--p")) continue;

        // چک کن offset فایل صفر باشه
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

    // ذخیره تابع اصلی
    original_prop_get = *(prop_get_t*)got_addr;
    if (!original_prop_get) {
        LOGE("original_prop_get is null!");
        return false;
    }
    LOGI("Original __system_property_get: %p", original_prop_get);

    // writable کن
    size_t page_size = getpagesize();
    uintptr_t page = got_addr & ~(page_size - 1);
    if (mprotect((void*)page, page_size, PROT_READ | PROT_WRITE) != 0) {
        LOGE("mprotect failed: %s", strerror(errno));
        return false;
    }

    // patch کن
    *(prop_get_t*)got_addr = hooked_prop_get;

    // برگردون به read-only
    mprotect((void*)page, page_size, PROT_READ);

    // verify
    if (*(prop_get_t*)got_addr != hooked_prop_get) {
        LOGE("Hook verification failed!");
        return false;
    }

    LOGI("Hook verified successfully");

    // تست
    char test[256] = {0};
    hooked_prop_get("ro.product.model", test);
    LOGI("Test result: ro.product.model = %s", test);

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
        // DLCLOSE نزن - ماژول باید باز بمونه
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs* args) override {
        if (!needs_hook) {
            api->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
            return;
        }

        LOGI("Starting hook thread...");

        // thread جدا - منتظر load شدن library
        std::thread([]() {
            LOGI("Hook thread running");

            for (int i = 0; i < 60; i++) {
                // چک کن library load شده
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
                        // چک offset = 0
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
                    sleep(1); // کمی صبر کن تا GOT fill بشه
                    if (applyHook()) {
                        hook_ready = true;
                        LOGI("Hook applied successfully!");
                    } else {
                        LOGE("Hook failed!");
                    }
                    return;
                }

                sleep(1);
            }

            LOGE("Library never loaded after 60s");
        }).detach();

        // DLCLOSE نزن چون hook داریم
    }

private:
    zygisk::Api* api;
    JNIEnv* env;
    bool needs_hook = false;
};

REGISTER_ZYGISK_MODULE(FIFAModule)
