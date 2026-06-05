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

using namespace std::chrono_literals;

#define LOG_TAG "FIFAHook"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

typedef int (*__system_property_get_fn)(const char* name, char* value);
static __system_property_get_fn original_prop_get = nullptr;

static int hooked_prop_get(const char* name, char* value) {
    if (name) {
        if (strcmp(name, "ro.product.model") == 0) {
            strcpy(value, "SM-F9460");
            LOGI("✅ Hooked: ro.product.model -> SM-F9460");
            return strlen(value);
        }
        if (strcmp(name, "ro.product.brand") == 0) {
            strcpy(value, "samsung");
            LOGI("✅ Hooked: ro.product.brand -> samsung");
            return strlen(value);
        }
        if (strcmp(name, "ro.product.manufacturer") == 0) {
            strcpy(value, "samsung");
            LOGI("✅ Hooked: ro.product.manufacturer -> samsung");
            return strlen(value);
        }
        if (strcmp(name, "ro.product.device") == 0) {
            strcpy(value, "q2q");
            LOGI("✅ Hooked: ro.product.device -> q2q");
            return strlen(value);
        }
        if (strcmp(name, "ro.build.fingerprint") == 0) {
            strcpy(value, "samsung/q2qzh/q2q:15/UP1A.231005.007/F946BXXU1BWK4:user/release-keys");
            LOGI("✅ Hooked: ro.build.fingerprint");
            return strlen(value);
        }
    }
    return original_prop_get(name, value);
}

struct LibraryInfo {
    uintptr_t base = 0;
    uintptr_t plt_offset = 0x6e74c78;  // از readelf گرفتیم
    uintptr_t plt_runtime = 0;
};

static bool findLibraryBase(const char* lib_name, LibraryInfo& info) {
    std::ifstream maps("/proc/self/maps");
    if (!maps.is_open()) {
        LOGE("Failed to open /proc/self/maps");
        return false;
    }
    
    std::string line;
    while (std::getline(maps, line)) {
        if (line.find(lib_name) != std::string::npos && line.find("r-xp") != std::string::npos) {
            size_t dash = line.find('-');
            if (dash != std::string::npos) {
                std::string base_str = line.substr(0, dash);
                info.base = std::stoull(base_str, nullptr, 16);
                LOGI("Found %s at base: 0x%lx", lib_name, info.base);
                info.plt_runtime = info.base + info.plt_offset;
                LOGI("PLT entry runtime address: 0x%lx", info.plt_runtime);
                return true;
            }
        }
    }
    return false;
}

static bool applyPLTHook(LibraryInfo& info) {
    long page_size = sysconf(_SC_PAGESIZE);
    if (page_size <= 0) {
        page_size = 4096;  // fallback
    }
    
    uintptr_t page_start = info.plt_runtime & ~(page_size - 1);
    
    LOGI("Page start: 0x%lx, Page size: 0x%lx", page_start, page_size);
    
    if (mprotect((void*)page_start, page_size, PROT_READ | PROT_WRITE) != 0) {
        LOGE("mprotect WRITE failed: %s", strerror(errno));
        return false;
    }
    
    original_prop_get = *reinterpret_cast<__system_property_get_fn*>(info.plt_runtime);
    LOGI("Original __system_property_get at: %p", original_prop_get);
    
    if (!original_prop_get) {
        LOGE("Original function is null!");
        mprotect((void*)page_start, page_size, PROT_READ);
        return false;
    }
    
    *reinterpret_cast<__system_property_get_fn*>(info.plt_runtime) = hooked_prop_get;
    LOGI("PLT entry patched");
    
    mprotect((void*)page_start, page_size, PROT_READ);
    
    __system_property_get_fn current = *reinterpret_cast<__system_property_get_fn*>(info.plt_runtime);
    if (current == hooked_prop_get) {
        LOGI("✅ PLT hook verification successful!");
        return true;
    } else {
        LOGE("❌ PLT hook verification failed!");
        return false;
    }
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

        LOGI("FIFA Mobile detected - preparing hook");
        needs_hook = true;
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs* args) override {
        if (!needs_hook) {
            api->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
            return;
        }
        
        LOGI("postAppSpecialize - starting hook");
        
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        LibraryInfo info;
        if (findLibraryBase("libFIFAMobileNeon.so", info)) {
            if (applyPLTHook(info)) {
                LOGI("✅ FIFA Mobile hook installed successfully!");
                
                char test_val[256] = {0};
                hooked_prop_get("ro.product.model", test_val);
                LOGI("Test: ro.product.model = %s", test_val);
            } else {
                LOGE("❌ Failed to install hook");
            }
        } else {
            LOGE("❌ libFIFAMobileNeon.so not found");
        }
    }

private:
    zygisk::Api* api;
    JNIEnv* env;
    bool needs_hook = false;
};

REGISTER_ZYGISK_MODULE(FIFAModule)
REGISTER_ZYGISK_COMPANION(companion)
