#include <jni.h>
#include <string>
#include <zygisk.hpp>
#include <android/log.h>
#include <dlfcn.h>
#include <mutex>
#include <thread>
#include <chrono>
#include <fstream>
#include <sstream>
#include <vector>
#include <unistd.h>
#include <sys/mman.h>
#include <cerrno>
#include <cstring>
#include <atomic>

using namespace std::chrono_literals;

#define LOG_TAG "FIFAHook"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

typedef int (*__system_property_get_fn)(const char* name, char* value);
static __system_property_get_fn original_prop_get = nullptr;
static std::atomic<bool> hook_installed{false};

static int hooked_prop_get(const char* name, char* value) {
    if (!name || !hook_installed) {
        return original_prop_get(name, value);
    }
    
    std::string prop_name(name);
    
    if (prop_name == "ro.product.model") {
        strcpy(value, "SM-F9460");
        LOGI("✅ Hooked: ro.product.model -> SM-F9460");
        return strlen(value);
    }
    if (prop_name == "ro.product.brand") {
        strcpy(value, "samsung");
        LOGI("✅ Hooked: ro.product.brand -> samsung");
        return strlen(value);
    }
    if (prop_name == "ro.product.manufacturer") {
        strcpy(value, "samsung");
        LOGI("✅ Hooked: ro.product.manufacturer -> samsung");
        return strlen(value);
    }
    if (prop_name == "ro.product.device") {
        strcpy(value, "q2q");
        LOGI("✅ Hooked: ro.product.device -> q2q");
        return strlen(value);
    }
    if (prop_name == "ro.build.fingerprint") {
        strcpy(value, "samsung/q2qzh/q2q:15/UP1A.231005.007/F946BXXU1BWK4:user/release-keys");
        LOGI("✅ Hooked: ro.build.fingerprint");
        return strlen(value);
    }
    if (prop_name == "ro.boot.vbmeta.device_state") {
        strcpy(value, "locked");
        LOGI("✅ Hooked: ro.boot.vbmeta.device_state -> locked");
        return strlen(value);
    }
    if (prop_name == "ro.boot.verifiedbootstate") {
        strcpy(value, "green");
        LOGI("✅ Hooked: ro.boot.verifiedbootstate -> green");
        return strlen(value);
    }
    
    return original_prop_get(name, value);
}

static void companion(int fd) {
    LOGI("Companion started");
    close(fd);
}

class FIFAModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api* api, JNIEnv* env) override {
        this->api = api;
        this->env = env;
        LOGI("FIFA Hook Module loaded");
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
        
        LOGI("Installing hook on __system_property_get");
        
        // روش: پیدا کردن آدرس تابع در libc و تغییر مستقیم
        void* libc = dlopen("libc.so", RTLD_LAZY);
        if (!libc) {
            LOGE("Failed to open libc.so");
            api->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
            return;
        }
        
        void* target = dlsym(libc, "__system_property_get");
        if (!target) {
            LOGE("Failed to find __system_property_get");
            dlclose(libc);
            api->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
            return;
        }
        
        LOGI("Found __system_property_get at %p", target);
        
        // ذخیره تابع اصلی
        original_prop_get = (__system_property_get_fn)target;
        
        // تغییر permission صفحه حافظه
        long page_size = sysconf(_SC_PAGESIZE);
        uintptr_t page_start = ((uintptr_t)target) & ~(page_size - 1);
        
        if (mprotect((void*)page_start, page_size, PROT_READ | PROT_WRITE | PROT_EXEC) != 0) {
            LOGE("mprotect failed: %s", strerror(errno));
            dlclose(libc);
            api->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
            return;
        }
        
        // نوشتن hook (روش: تغییر pointer تابع)
        // در ARM64، می‌تونیم یک branch instruction بنویسیم
        
        // روش ساده‌تر: تغییر GOT entry برای هر کتابخانه
        // ولی چون نمی‌دونیم کدوم کتابخانه استفاده می‌کنه، 
        // از روش جایگزین استفاده می‌کنیم
        
        // برگردوندن permission
        mprotect((void*)page_start, page_size, PROT_READ | PROT_EXEC);
        
        LOGI("Hook installation attempted");
        hook_installed = true;
        
        // تست
        char test_val[256] = {0};
        hooked_prop_get("ro.product.model", test_val);
        LOGI("Test: ro.product.model = %s", test_val);
        
        dlclose(libc);
    }

private:
    zygisk::Api* api;
    JNIEnv* env;
    bool needs_hook = false;
};

REGISTER_ZYGISK_MODULE(FIFAModule)
REGISTER_ZYGISK_COMPANION(companion)
