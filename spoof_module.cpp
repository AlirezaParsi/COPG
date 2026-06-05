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
typedef void* (*dlsym_fn)(void* handle, const char* symbol);
typedef void* (*dlopen_fn)(const char* filename, int flags);

static __system_property_get_fn original_prop_get = nullptr;
static dlsym_fn original_dlsym = nullptr;
static dlopen_fn original_dlopen = nullptr;
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

static void* hooked_dlsym(void* handle, const char* symbol) {
    void* result = original_dlsym(handle, symbol);
    
    if (symbol && strcmp(symbol, "__system_property_get") == 0) {
        LOGI("🎯 Intercepted dlsym for __system_property_get");
        original_prop_get = (__system_property_get_fn)result;
        
        if (!hook_installed) {
            hook_installed = true;
            LOGI("✅ Hook installed via dlsym interception");
            
            // تست
            char test_val[256] = {0};
            hooked_prop_get("ro.product.model", test_val);
            LOGI("Test: ro.product.model = %s", test_val);
        }
        
        return (void*)hooked_prop_get;
    }
    
    return result;
}

static void* hooked_dlopen(const char* filename, int flags) {
    void* handle = original_dlopen(filename, flags);
    
    if (filename && strstr(filename, "libFIFAMobileNeon.so") != nullptr) {
        LOGI("📚 libFIFAMobileNeon.so loaded");
    }
    
    return handle;
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
        
        original_dlsym = (dlsym_fn)dlsym(RTLD_DEFAULT, "dlsym");
        original_dlopen = (dlopen_fn)dlsym(RTLD_DEFAULT, "dlopen");
        
        if (original_dlsym) {
            LOGI("Original dlsym at %p", original_dlsym);
        }
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

        LOGI("FIFA Mobile detected - will hook dlsym");
        needs_hook = true;
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs* args) override {
        if (!needs_hook) {
            api->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
            return;
        }
        
        LOGI("Installing dlsym hook...");
        
        void* libdl = dlopen("libdl.so", RTLD_LAZY);
        if (!libdl) {
            libdl = dlopen("libc.so", RTLD_LAZY);
        }
        
        if (libdl) {
            void* target = dlsym(libdl, "dlsym");
            if (target) {
                long page_size = sysconf(_SC_PAGESIZE);
                uintptr_t page_start = ((uintptr_t)target) & ~(page_size - 1);
                
                if (mprotect((void*)page_start, page_size, PROT_READ | PROT_WRITE) == 0) {
                    auto* dlsym_ptr = reinterpret_cast<dlsym_fn*>(target);
                    original_dlsym = *dlsym_ptr;
                    *dlsym_ptr = hooked_dlsym;
                    mprotect((void*)page_start, page_size, PROT_READ);
                    LOGI("✅ dlsym hook installed");
                }
            }
            dlclose(libdl);
        }
        
        // همچنین dlopen را هم hook کنیم
        void* libc = dlopen("libc.so", RTLD_LAZY);
        if (libc) {
            void* target = dlsym(libc, "dlopen");
            if (target) {
                long page_size = sysconf(_SC_PAGESIZE);
                uintptr_t page_start = ((uintptr_t)target) & ~(page_size - 1);
                
                if (mprotect((void*)page_start, page_size, PROT_READ | PROT_WRITE) == 0) {
                    auto* dlopen_ptr = reinterpret_cast<dlopen_fn*>(target);
                    original_dlopen = *dlopen_ptr;
                    *dlopen_ptr = hooked_dlopen;
                    mprotect((void*)page_start, page_size, PROT_READ);
                    LOGI("✅ dlopen hook installed");
                }
            }
            dlclose(libc);
        }
        
        LOGI("Waiting for dlsym calls...");
    }

private:
    zygisk::Api* api;
    JNIEnv* env;
    bool needs_hook = false;
};

REGISTER_ZYGISK_MODULE(FIFAModule)
REGISTER_ZYGISK_COMPANION(companion)
