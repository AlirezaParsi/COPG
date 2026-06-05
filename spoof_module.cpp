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
typedef void* (*dlopen_fn)(const char* filename, int flags);
static __system_property_get_fn original_prop_get = nullptr;
static dlopen_fn original_dlopen = nullptr;
static std::atomic<bool> hook_installed{false};
static std::atomic<bool> library_hooked{false};

static int hooked_prop_get(const char* name, char* value);
static void hookLibraryPLT();

static void* hooked_dlopen(const char* filename, int flags) {
    void* handle = original_dlopen(filename, flags);
    
    if (filename && strstr(filename, "libFIFAMobileNeon.so") != nullptr) {
        LOGI("📚 libFIFAMobileNeon.so loaded at %p", handle);
        
        std::thread([]() {
            std::this_thread::sleep_for(500ms);
            hookLibraryPLT();
        }).detach();
    }
    
    return handle;
}

static void hookLibraryPLT() {
    if (library_hooked) return;
    
    std::ifstream maps("/proc/self/maps");
    if (!maps.is_open()) return;
    
    std::string line;
    uintptr_t base = 0;
    
    while (std::getline(maps, line)) {
        if (line.find("libFIFAMobileNeon.so") != std::string::npos) {
            if (line.find("r-xp") != std::string::npos) {
                size_t dash = line.find('-');
                if (dash != std::string::npos) {
                    std::string base_str = line.substr(0, dash);
                    base = std::stoull(base_str, nullptr, 16);
                    LOGI("Found libFIFAMobileNeon.so at base: 0x%lx", base);
                    
                    uintptr_t plt_offset = 0x6e74c78;
                    uintptr_t plt_addr = base + plt_offset;
                    
                    long page_size = sysconf(_SC_PAGESIZE);
                    uintptr_t page_start = plt_addr & ~(page_size - 1);
                    
                    if (mprotect((void*)page_start, page_size, PROT_READ | PROT_WRITE) == 0) {
                        auto* plt_entry = reinterpret_cast<__system_property_get_fn*>(plt_addr);
                        original_prop_get = *plt_entry;
                        *plt_entry = hooked_prop_get;
                        mprotect((void*)page_start, page_size, PROT_READ);
                        
                        LOGI("✅ PLT hook installed for libFIFAMobileNeon.so");
                        library_hooked = true;
                        hook_installed = true;
                    }
                    break;
                }
            }
        }
    }
}

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
        
        original_dlopen = (dlopen_fn)dlsym(RTLD_DEFAULT, "dlopen");
        if (original_dlopen) {
            LOGI("Original dlopen at %p", original_dlopen);
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

        LOGI("FIFA Mobile detected - will hook dlopen");
        needs_hook = true;
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs* args) override {
        if (!needs_hook) {
            api->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
            return;
        }
        
        LOGI("Installing dlopen hook...");
        
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
        
        LOGI("Waiting for libFIFAMobileNeon.so to be loaded...");
    }

private:
    zygisk::Api* api;
    JNIEnv* env;
    bool needs_hook = false;
};

REGISTER_ZYGISK_MODULE(FIFAModule)
REGISTER_ZYGISK_COMPANION(companion)
