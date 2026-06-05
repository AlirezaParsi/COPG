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
    if (!name) {
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
        
        // گرفتن تابع اصلی برای reference
        original_prop_get = (__system_property_get_fn)dlsym(RTLD_DEFAULT, "__system_property_get");
        if (original_prop_get) {
            LOGI("Original __system_property_get at %p", original_prop_get);
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

        LOGI("FIFA Mobile detected - will hook in post");
        needs_hook = true;
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs* args) override {
        if (!needs_hook) {
            api->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
            return;
        }
        
        LOGI("Waiting for library to load...");
        
        // صبر برای بارگذاری کتابخانه FIFA
        for (int attempt = 0; attempt < 30; attempt++) {
            std::this_thread::sleep_for(1s);
            
            // پیدا کردن کتابخانه در مموری
            if (findAndHookLibrary()) {
                LOGI("✅ Hook installed successfully!");
                hook_installed = true;
                
                // تست نهایی
                char test[256];
                original_prop_get = (__system_property_get_fn)dlsym(RTLD_DEFAULT, "__system_property_get");
                hooked_prop_get("ro.product.model", test);
                LOGI("Final test: ro.product.model = %s", test);
                return;
            }
        }
        
        LOGE("Failed to find and hook library after 30 seconds");
    }

private:
    zygisk::Api* api;
    JNIEnv* env;
    bool needs_hook = false;
    
    bool findAndHookLibrary() {
        std::ifstream maps("/proc/self/maps");
        if (!maps.is_open()) return false;
        
        std::string line;
        uintptr_t base = 0;
        bool found = false;
        
        while (std::getline(maps, line)) {
            if (line.find("libFIFAMobileNeon.so") != std::string::npos) {
                if (line.find("r-xp") != std::string::npos) {
                    size_t dash = line.find('-');
                    if (dash != std::string::npos) {
                        std::string base_str = line.substr(0, dash);
                        base = std::stoull(base_str, nullptr, 16);
                        LOGI("Found libFIFAMobileNeon.so at base: 0x%lx", base);
                        found = true;
                        break;
                    }
                }
            }
        }
        
        if (!found) return false;
        
        // آدرس PLT entry برای __system_property_get
        // این offset رو از readelf قبلاً گرفتی: 0x6e74c78
        uintptr_t plt_offset = 0x6e74c78;
        uintptr_t plt_addr = base + plt_offset;
        
        LOGI("PLT entry address: 0x%lx", plt_addr);
        
        // تغییر permission
        long page_size = sysconf(_SC_PAGESIZE);
        uintptr_t page_start = plt_addr & ~(page_size - 1);
        
        if (mprotect((void*)page_start, page_size, PROT_READ | PROT_WRITE) != 0) {
            LOGE("mprotect failed: %s", strerror(errno));
            return false;
        }
        
        // ذخیره تابع اصلی
        auto* plt_entry = reinterpret_cast<__system_property_get_fn*>(plt_addr);
        original_prop_get = *plt_entry;
        LOGI("Original PLT entry points to: %p", original_prop_get);
        
        // تغییر به hook
        *plt_entry = hooked_prop_get;
        
        // برگردوندن permission
        mprotect((void*)page_start, page_size, PROT_READ);
        
        // verification
        if (*plt_entry == hooked_prop_get) {
            LOGI("PLT hook verification successful!");
            return true;
        }
        
        return false;
    }
};

REGISTER_ZYGISK_MODULE(FIFAModule)
REGISTER_ZYGISK_COMPANION(companion)
