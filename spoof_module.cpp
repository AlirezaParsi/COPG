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
    if (name && !hook_installed) {
        // قبل از نصب hook، مستقیم برگردان
        return original_prop_get(name, value);
    }
    
    if (name && hook_installed) {
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
    }
    
    return original_prop_get(name, value);
}

static void companion(int fd) {
    LOGI("Companion started");
    char buf[256];
    read(fd, buf, sizeof(buf));
    close(fd);
}

class FIFAModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api* api, JNIEnv* env) override {
        this->api = api;
        this->env = env;
        LOGI("FIFA Hook Module loaded");
        
        // گرفتن تابع اصلی از libc
        original_prop_get = (__system_property_get_fn)dlsym(RTLD_DEFAULT, "__system_property_get");
        if (original_prop_get) {
            LOGI("Found __system_property_get at %p", original_prop_get);
        } else {
            LOGE("Failed to find __system_property_get");
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

        LOGI("FIFA Mobile detected");
        needs_hook = true;
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs* args) override {
        if (!needs_hook) {
            api->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
            return;
        }
        
        LOGI("postAppSpecialize - installing hook via PLT");
        
        // صبر برای بارگذاری کامل کتابخانه
        std::this_thread::sleep_for(3s);
        
        // جستجوی کتابخانه
        LibraryInfo info;
        if (findLibrary(info)) {
            if (installPLTHook(info)) {
                hook_installed = true;
                LOGI("✅ Hook installed successfully!");
                
                // تست
                char test[256];
                hooked_prop_get("ro.product.model", test);
                LOGI("Test: ro.product.model = %s", test);
                return;
            }
        }
        
        // روش جایگزین: inline hook
        LOGI("Trying alternative hook method...");
        if (installInlineHook()) {
            hook_installed = true;
            LOGI("✅ Inline hook installed!");
            return;
        }
        
        LOGE("❌ All hook methods failed!");
    }

private:
    zygisk::Api* api;
    JNIEnv* env;
    bool needs_hook = false;
    
    struct LibraryInfo {
        uintptr_t base = 0;
        uintptr_t plt_offset = 0x6e74c78;
        uintptr_t plt_runtime = 0;
    };
    
    bool findLibrary(LibraryInfo& info) {
        std::ifstream maps("/proc/self/maps");
        if (!maps.is_open()) {
            LOGE("Cannot open maps");
            return false;
        }
        
        std::string line;
        while (std::getline(maps, line)) {
            // جستجوی کتابخانه اصلی FIFA
            if (line.find("libFIFAMobileNeon.so") != std::string::npos ||
                line.find("FIFAMobileNeon.so") != std::string::npos) {
                
                if (line.find("r-xp") != std::string::npos) {
                    size_t dash = line.find('-');
                    if (dash != std::string::npos) {
                        std::string base_str = line.substr(0, dash);
                        info.base = std::stoull(base_str, nullptr, 16);
                        info.plt_runtime = info.base + info.plt_offset;
                        
                        LOGI("Found library at base: 0x%lx", info.base);
                        LOGI("PLT entry at: 0x%lx", info.plt_runtime);
                        return true;
                    }
                }
            }
        }
        
        // لاگ کردن برای دیباگ
        maps.clear();
        maps.seekg(0);
        LOGI("Loaded libraries:");
        while (std::getline(maps, line)) {
            if (line.find(".so") != std::string::npos && line.find("fifa") != std::string::npos) {
                LOGI("  %s", line.c_str());
            }
        }
        
        return false;
    }
    
    bool installPLTHook(LibraryInfo& info) {
        long page_size = sysconf(_SC_PAGESIZE);
        if (page_size <= 0) page_size = 4096;
        
        uintptr_t page_start = info.plt_runtime & ~(page_size - 1);
        
        if (mprotect((void*)page_start, page_size, PROT_READ | PROT_WRITE) != 0) {
            LOGE("mprotect failed: %s", strerror(errno));
            return false;
        }
        
        auto* plt_entry = reinterpret_cast<__system_property_get_fn*>(info.plt_runtime);
        original_prop_get = *plt_entry;
        LOGI("Original function: %p", original_prop_get);
        
        *plt_entry = hooked_prop_get;
        
        mprotect((void*)page_start, page_size, PROT_READ);
        
        return (*plt_entry == hooked_prop_get);
    }
    
    bool installInlineHook() {
        // روش ساده: تابع اصلی را عوض کنیم
        if (!original_prop_get) return false;
        
        // این نیاز به Dobby یا فریمورک مشابه دارد
        LOGI("Inline hook requires Dobby framework");
        return false;
    }
};

REGISTER_ZYGISK_MODULE(FIFAModule)
REGISTER_ZYGISK_COMPANION(companion)
