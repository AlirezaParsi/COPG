/* ============================================================================
 *  COPG — device & CPU spoof module for Android
 *  Copyright (c) Alireza Parsi  ·  https://github.com/AlirezaParsi/COPG
 *  Telegram: https://t.me/COPG_module
 *
 *  Original work by Alireza Parsi. The stealth spoofing techniques here —
 *  notably the ANDROID_ID cache-forge (Settings$Secure.sNameValueCache +
 *  forged GenerationTracker) and the copy-on-write property spoof (per-process
 *  MAP_PRIVATE remap of /dev/__properties__ then DLCLOSE, zero memory
 *  residency) — were designed and implemented for this project.
 *
 *  ⚠ If you reuse, fork, or learn from this code in ANOTHER module (free OR
 *  paid), you MUST give clear, visible credit to Alireza Parsi and link back to
 *  the repository above. Copying it into a "personal"/paid module without
 *  attribution is not OK. Be decent — credit the author.
 * ========================================================================== */
#include <jni.h>
#include <string>
#include <zygisk.hpp>
#include "json.hpp"
#include <fstream>
#include <unordered_map>
#include <dlfcn.h>
#include <sys/mman.h>
#include <unistd.h>
#include <android/log.h>
#include <mutex>
#include <functional>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <vector>
#include <unordered_set>
#include <fcntl.h>
#include <sstream>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <dirent.h>
#include <elf.h>
#include <thread>
#include <atomic>
#include <sys/system_properties.h>

using json = nlohmann::json;

#define LOG_TAG "COPGModule"

#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

#define CONFIG_LOG(...) LOGI("[CONFIG] " __VA_ARGS__)
#define SPOOF_LOG(...) LOGI("[SPOOF] " __VA_ARGS__)
#define COMPANION_LOG(...) LOGI("[COMPANION] " __VA_ARGS__)
#define PKG_LOG(...) LOGI("[PKG] " __VA_ARGS__)

#if defined(__aarch64__) || defined(__x86_64__)
    #define IS_64BIT 1
    #define IS_32BIT 0
#elif defined(__arm__) || defined(__i386__)
    #define IS_64BIT 0
    #define IS_32BIT 1
#else
    #error "Unsupported architecture"
#endif


// ─────────────────────────────────────────
// Device Info & Package Flags
// ─────────────────────────────────────────
struct DeviceInfo {
    std::string brand;
    std::string device;
    std::string manufacturer;
    std::string model;
    std::string fingerprint;
    std::string product;
    std::string android_version;
    int sdk_int;
    bool should_spoof_android_version = false;
    bool should_spoof_sdk_int = false;
    std::string serial;                       // Build.SERIAL spoof (optional, per device profile)
    bool should_spoof_serial = false;
    std::string android_id;                   // Settings.Secure ANDROID_ID spoof (optional, per device profile)
    bool should_spoof_android_id = false;
    std::unordered_map<std::string, std::string> prop_overrides;
};

struct PackageFlags {
    bool needs_device_spoof = false;
    bool needs_cpu_mount = false;    // with_cpu
    bool needs_cpu_unmount = false;  // blocked
    bool needs_cow = false;          // cow → stealth COW prop spoof
};

static DeviceInfo current_info;
static std::mutex info_mutex;
static jclass buildClass = nullptr;
static jclass versionClass = nullptr;
static jfieldID modelField = nullptr;
static jfieldID brandField = nullptr;
static jfieldID deviceField = nullptr;
static jfieldID manufacturerField = nullptr;
static jfieldID fingerprintField = nullptr;
static jfieldID productField = nullptr;
static jfieldID releaseField = nullptr;
static jfieldID sdkIntField = nullptr;
static jfieldID serialField = nullptr;

static time_t last_config_mtime = 0;
static const std::string config_path = "/data/adb/modules/COPG/COPG.json";
static const char* spoof_file_path = "/data/adb/modules/COPG/cpuinfo_spoof";

static std::unordered_set<std::string> cpu_blacklist;
static std::unordered_set<std::string> cpu_only_packages;

struct JniString {
    JNIEnv* env;
    jstring jstr;
    const char* chars;
    JniString(JNIEnv* e, jstring s) : env(e), jstr(s), chars(nullptr) {
        if (jstr) chars = env->GetStringUTFChars(jstr, nullptr);
    }
    ~JniString() {
        if (jstr && chars) env->ReleaseStringUTFChars(jstr, chars);
    }
    const char* get() const { return chars; }
};

// ─────────────────────────────────────────
// IPC helpers
// ─────────────────────────────────────────
static bool ipc_writeStr(int fd, const std::string& s) {
    uint32_t len = s.size();
    if (write(fd, &len, 4) != 4) return false;
    if (len > 0 && (size_t)write(fd, s.data(), len) != len) return false;
    return true;
}

static bool ipc_readStr(int fd, std::string& s) {
    uint32_t len = 0;
    if (read(fd, &len, 4) != 4) return false;
    if (len == 0) { s.clear(); return true; }
    s.resize(len);
    return (size_t)read(fd, s.data(), len) == len;
}

static bool ipc_writeU64(int fd, uint64_t v) {
    return write(fd, &v, 8) == 8;
}

static bool ipc_readU64(int fd, uint64_t& v) {
    return read(fd, &v, 8) == 8;
}


// ─────────────────────────────────────────
// Companion (runs as root)
// ─────────────────────────────────────────
static void companion(int fd) {
    COMPANION_LOG("Started");
    char buffer[2048];
    ssize_t bytes = read(fd, buffer, sizeof(buffer)-1);
    
    if (bytes > 0) {
        buffer[bytes] = '\0';
        std::string command = buffer;
        
        if (command == "unmount_spoof" || command == "mount_spoof") {
            int result = -1;
            if (command == "unmount_spoof") {
                result = system("/system/bin/umount /proc/cpuinfo 2>/dev/null");
                COMPANION_LOG("CPU unmount");
            } else {
                if (access(spoof_file_path, F_OK) == 0) {
                    system("/system/bin/umount /proc/cpuinfo 2>/dev/null");
                    char mount_cmd[512];
                    snprintf(mount_cmd, sizeof(mount_cmd), "/system/bin/mount --bind %s /proc/cpuinfo", spoof_file_path);
                    result = system(mount_cmd);
                    COMPANION_LOG("CPU mount");
                }
            }
            write(fd, &result, sizeof(result));
        }
    }
    close(fd);
}

// ─────────────────────────────────────────
// Main Module
// ─────────────────────────────────────────
// ─────────────────────────────────────────
// ANDROID_ID spoof — STEALTH (synchronous, no thread, then DLCLOSE)
// ─────────────────────────────────────────
// ANDROID_ID is not a prop/Build field — Settings.Secure.getString reads it
// through an in-process cache: Settings$Secure.sNameValueCache, which holds
//   ArrayMap<String,String>              mValues
//   ArrayMap<String,GenerationTracker>   mGenerationTrackers   (per-key, A15)
// getStringForUser() returns mValues.get(name) WITHOUT any binder call iff a
// tracker exists for that name AND tracker.isGenerationChanged()==false.
// isGenerationChanged() = (mArray.get(mIndex) != mCurrentGeneration).
//
// So we forge the whole thing synchronously, no ContentResolver/ActivityThread
// needed: build a PRIVATE MemoryIntArray(1)=[0], a GenerationTracker(name,arr,
// 0,0,null) (→ isGenerationChanged() is permanently false because nothing else
// writes our array), then put the fake value + tracker into the cache. The app
// then reads the fake from its OWN heap with the provider never consulted.
// Because it's a one-shot value write (data lives in the ART heap, like a
// Build.* field), the module can DLCLOSE immediately — no resident thread, no
// mapped .so at anti-cheat scan time. Layout verified against an Android 15
// (AOSPA) framework.jar; falls back to the real value if any class/field/ctor
// doesn't resolve (older/newer ART). Value is per-device: COPG.json's
// PACKAGES_<KEY>_DEVICE.ANDROID_ID, applied to every app on that profile.
static void forgeAndroidId(JNIEnv* env, const char* fakeId) {
    if (!fakeId || !*fakeId) return;
    auto clr = [&]{ if (env->ExceptionCheck()) env->ExceptionClear(); };

    jclass secCls = env->FindClass("android/provider/Settings$Secure");           clr();
    jclass nvcCls = env->FindClass("android/provider/Settings$NameValueCache");   clr();
    jclass gtCls  = env->FindClass("android/provider/Settings$GenerationTracker");clr();
    jclass miaCls = env->FindClass("android/util/MemoryIntArray");                clr();
    if (!secCls || !nvcCls || !gtCls || !miaCls) {
        LOGE("[AID] class resolve fail (sec=%p nvc=%p gt=%p mia=%p)", secCls, nvcCls, gtCls, miaCls);
        return;
    }

    jfieldID cacheFld  = env->GetStaticFieldID(secCls, "sNameValueCache", "Landroid/provider/Settings$NameValueCache;"); clr();
    jfieldID valuesFld = env->GetFieldID(nvcCls, "mValues", "Landroid/util/ArrayMap;");                                  clr();
    jfieldID tracksFld = env->GetFieldID(nvcCls, "mGenerationTrackers", "Landroid/util/ArrayMap;");                      clr();
    if (!cacheFld || !valuesFld || !tracksFld) {
        LOGE("[AID] field resolve fail (cache=%p values=%p tracks=%p)", cacheFld, valuesFld, tracksFld);
        return;
    }

    jobject cache = env->GetStaticObjectField(secCls, cacheFld); clr();
    if (!cache) { LOGE("[AID] sNameValueCache null"); return; }
    jobject values = env->GetObjectField(cache, valuesFld); clr();
    jobject tracks = env->GetObjectField(cache, tracksFld); clr();
    if (!values || !tracks) { LOGE("[AID] maps null (values=%p tracks=%p)", values, tracks); return; }

    // Private MemoryIntArray(1), [0]=0 → our tracker's generation never changes.
    jmethodID miaCtor = env->GetMethodID(miaCls, "<init>", "(I)V"); clr();
    jmethodID miaSet  = env->GetMethodID(miaCls, "set", "(II)V");   clr();
    jobject mia = miaCtor ? env->NewObject(miaCls, miaCtor, (jint)1) : nullptr; clr();
    if (!mia) { LOGE("[AID] MemoryIntArray create fail"); return; }
    if (miaSet) { env->CallVoidMethod(mia, miaSet, (jint)0, (jint)0); clr(); }

    jstring key = env->NewStringUTF("android_id");

    // GenerationTracker(String name, MemoryIntArray arr, int index, int curGen, Consumer errorHandler)
    jmethodID gtCtor = env->GetMethodID(gtCls, "<init>",
        "(Ljava/lang/String;Landroid/util/MemoryIntArray;IILjava/util/function/Consumer;)V"); clr();
    jobject tracker = gtCtor ? env->NewObject(gtCls, gtCtor, key, mia, (jint)0, (jint)0, (jobject)nullptr) : nullptr; clr();
    if (!tracker) { LOGE("[AID] GenerationTracker create fail"); return; }

    // ArrayMap.put(Object,Object) via the Map interface signature.
    jclass amCls = env->GetObjectClass(values);
    jmethodID putMid = env->GetMethodID(amCls, "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;"); clr();
    env->DeleteLocalRef(amCls);
    if (!putMid) { LOGE("[AID] ArrayMap.put not found"); return; }

    jstring fakeStr = env->NewStringUTF(fakeId);
    jobject p1 = env->CallObjectMethod(values, putMid, key, fakeStr);  clr(); if (p1) env->DeleteLocalRef(p1);
    jobject p2 = env->CallObjectMethod(tracks, putMid, key, tracker);  clr(); if (p2) env->DeleteLocalRef(p2);

    LOGI("[AID] forged android_id -> %s (synchronous, no thread)", fakeId);
}

// ─────────────────────────────────────────
// Prop spoof — STEALTH (COW remap + in-place overwrite, then DLCLOSE)
// ─────────────────────────────────────────
// Same model as ANDROID_ID: a one-shot DATA write, no resident hook. The bionic
// property area (/dev/__properties__/<ctx>) is MAP_SHARED read-only across all
// processes. We remap the page(s) holding the target prop_info as MAP_PRIVATE
// (copy-on-write), so the edit is PER-PROCESS (system + other apps untouched),
// overwrite the value inline, then DLCLOSE. Covers BOTH __system_property_get
// and __system_property_read_callback (same data). prop_info (short props):
//   atomic_uint32 serial; char value[PROP_VALUE_MAX]; char name[];
// Only EXISTING short props, value len <= PROP_VALUE_MAX-1. Layout is stable
// bionic; fails-safe (logs + keeps real value) if a prop is missing/long.
static std::vector<std::pair<uintptr_t, uintptr_t>> g_priv_prop_ranges;

static bool ensurePropAreaPrivate(const void* addr) {
    uintptr_t t = (uintptr_t)addr;
    for (auto& r : g_priv_prop_ranges) if (t >= r.first && t < r.second) return true; // already COW
    FILE* f = fopen("/proc/self/maps", "r");
    if (!f) return false;
    char line[512];
    bool ok = false;
    while (fgets(line, sizeof(line), f)) {
        uintptr_t s, e; unsigned long long off; char perms[8]; char path[256];
        path[0] = 0;
        if (sscanf(line, "%lx-%lx %7s %llx %*x:%*x %*u %255[^\n]", &s, &e, perms, &off, path) < 4) continue;
        if (t < s || t >= e) continue;
        char* p = path; while (*p == ' ') p++;
        if (strncmp(p, "/dev/__properties__", 19) != 0) break;   // not a prop area
        int fd = open(p, O_RDONLY);
        if (fd >= 0) {
            void* r = mmap((void*)s, (size_t)(e - s), PROT_READ | PROT_WRITE,
                           MAP_PRIVATE | MAP_FIXED, fd, (off_t)off);
            close(fd);
            if (r != MAP_FAILED) { g_priv_prop_ranges.push_back({s, e}); ok = true; }
        }
        break;
    }
    fclose(f);
    return ok;
}

static void forgeProp(const char* name, const char* val) {
    const prop_info* cpi = __system_property_find(name);
    if (!cpi) { LOGW("[PROP] %s: not found, skip", name); return; }
    size_t len = strlen(val);
    if (len >= PROP_VALUE_MAX) { LOGW("[PROP] %s: value len %zu >= %d (long prop), skip", name, len, PROP_VALUE_MAX); return; }
    if (!ensurePropAreaPrivate(cpi)) { LOGE("[PROP] %s: COW remap failed", name); return; }
    volatile uint32_t* serial = (volatile uint32_t*)cpi;
    char* value = (char*)cpi + sizeof(uint32_t);
    uint32_t old = *serial;
    *serial = old | 1;                                   // mark dirty (readers retry)
    __sync_synchronize();
    memcpy(value, val, len); value[len] = '\0';
    __sync_synchronize();
    *serial = ((uint32_t)len << 24) | (((old & 0x00FFFFFFu) + 2) & 0x00FFFFFFu);  // len + bumped gen, bit0=0
    __sync_synchronize();
    char rb[PROP_VALUE_MAX] = {0};
    __system_property_get(name, rb);
    LOGI("[PROP] %s -> '%s' (read-back '%s')", name, val, rb);
}

class COPGModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api* api, JNIEnv* env) override {
        this->api = api;
        this->env = env;
        LOGI("Module loaded");
        ensureBuildClass();
        reloadIfNeeded(true);
    }

    void onUnload() {
        std::lock_guard<std::mutex> lock(info_mutex);
        if (buildClass) { env->DeleteGlobalRef(buildClass); buildClass = nullptr; }
        if (versionClass) { env->DeleteGlobalRef(versionClass); versionClass = nullptr; }
    }

    void preAppSpecialize(zygisk::AppSpecializeArgs* args) override {
        if (!args || !args->nice_name) {
            api->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
            return;
        }

        JniString pkg(env, args->nice_name);
        const char* package_name = pkg.get();
        if (!package_name) {
            api->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
            return;
        }

        PKG_LOG("Processing: %s", package_name);
        do_android_id = false;
        do_prop_cow   = false;

        // Reset build class for forked process
        buildClass = nullptr;
        versionClass = nullptr;
        modelField = brandField = deviceField = manufacturerField = fingerprintField = productField = nullptr;
        releaseField = sdkIntField = nullptr;
        
        // ✅ Reload config every time (in case it changed)
        reloadIfNeeded(false);

        PackageFlags flags;
        bool found_in_config = false;
        
        {
            std::lock_guard<std::mutex> lock(info_mutex);
            DeviceInfo device_info;

            // ✅ Search in device_packages (DeviceInfo + map of packages)
            for (auto& device_entry : device_packages) {
                auto it = device_entry.second.find(package_name);
                if (it != device_entry.second.end()) {
                    found_in_config = true;
                    flags = it->second;  // Copy flags
                    device_info = device_entry.first;
                    current_info = device_info;
                    flags.needs_device_spoof = true;  // Always spoof device if found
                    break;
                }
            }

            // Check blacklist and cpu_only
            bool is_blacklisted = (cpu_blacklist.find(package_name) != cpu_blacklist.end());
            bool is_cpu_only = (cpu_only_packages.find(package_name) != cpu_only_packages.end());

            if (is_blacklisted) {
                found_in_config = true;
                flags.needs_cpu_unmount = true;
            }

            if (is_cpu_only && !found_in_config) {
                found_in_config = true;
                flags.needs_cpu_mount = true;
            }

            // ✅ If not found in config at all, close immediately
            if (!found_in_config) {
                PKG_LOG("%s: Not in config, closing module", package_name);
                api->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
                return;
            }

            // Execute actions
            if (flags.needs_device_spoof) {
                ensureBuildClass();
                spoofDevice(current_info);
                // ANDROID_ID is forged in postAppSpecialize (the MemoryIntArray
                // wants the app's uid / SELinux domain); just flag it here.
                do_android_id = current_info.should_spoof_android_id;

                // `cow` tag → stealth COW prop spoof. The device props are forged
                // (below, still in pre) into a per-process copy-on-write view of
                // the bionic property area, then the module DLCLOSEs — zero
                // residency, no foreign code in the app's memory for an anti-cheat.
                if (flags.needs_cow) {
                    do_prop_cow = true;
                    prop_cow_map = current_info.prop_overrides;  // copy under lock
                }
            }

            if (flags.needs_cpu_unmount) {
                executeCompanionCommand("unmount_spoof");
            } else if (flags.needs_cpu_mount) {
                executeCompanionCommand("mount_spoof");
            }
        }

        // Stealth COW prop spoof — runs HERE in preAppSpecialize. It doesn't need
        // the app uid (unlike ANDROID_ID's ashmem): it only edits /dev/__properties__
        // which is already mapped, and the pre stage's zygote SELinux domain has
        // broader access to the per-context prop files than the restricted app
        // domain. The COW pages persist through specialization + the unload.
        if (do_prop_cow) {
            for (auto& kv : prop_cow_map) forgeProp(kv.first.c_str(), kv.second.c_str());
            LOGI("[PROP] COW spoof done (%zu props)", prop_cow_map.size());
        }

        // Stealth Mode: close now unless ANDROID_ID still needs postAppSpecialize.
        if (!do_android_id) {
            PKG_LOG("%s: No deferred work, closing module for stealth", package_name);
            api->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
        }
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs* args) override {
        // ANDROID_ID is the only thing deferred to post (it needs the app uid for
        // its ashmem). Forge it, then unload: the fake lives in the ART heap, so
        // nothing of COPG stays mapped when the app's anti-cheat scans its memory.
        if (do_android_id) {
            forgeAndroidId(env, current_info.android_id.c_str());
            LOGI("[AID] done, unloading module");
            api->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
        }
    }

private:
    zygisk::Api* api;
    JNIEnv* env;
    bool do_android_id = false;
    bool do_prop_cow = false;    // cow tag → stealth COW prop spoof
    std::unordered_map<std::string, std::string> prop_cow_map;
    // ✅ CORRECT STRUCTURE: DeviceInfo + map of package_name -> PackageFlags
    std::vector<std::pair<DeviceInfo, std::unordered_map<std::string, PackageFlags>>> device_packages;


    // ─────────────────────────────────────────
    // Package tag parsing - ✅ Supports multiple tags
    // ─────────────────────────────────────────
    std::pair<std::string, std::unordered_set<std::string>> parsePackageWithTags(const std::string& package_str) {
        std::string package_name = package_str;
        std::unordered_set<std::string> tags;
        
        // Trim whitespace
        package_name.erase(0, package_name.find_first_not_of(" \t"));
        package_name.erase(package_name.find_last_not_of(" \t") + 1);
        
        // Find first colon
        size_t first_colon = package_name.find(':');
        if (first_colon != std::string::npos && first_colon < package_name.length() - 1) {
            std::string original_name = package_name;
            package_name = original_name.substr(0, first_colon);
            
            // Parse all tags
            size_t start = first_colon + 1;
            while (start < original_name.length()) {
                size_t end = original_name.find(':', start);
                std::string tag;
                if (end == std::string::npos) {
                    tag = original_name.substr(start);
                    start = original_name.length();
                } else {
                    tag = original_name.substr(start, end - start);
                    start = end + 1;
                }
                
                // Trim tag
                tag.erase(0, tag.find_first_not_of(" \t"));
                tag.erase(tag.find_last_not_of(" \t") + 1);
                
                if (!tag.empty()) tags.insert(tag);
            }
        }
        return {package_name, tags};
    }

    // ✅ Parse multiple tags into PackageFlags
    PackageFlags getFlagsFromTags(const std::unordered_set<std::string>& tags) {
        PackageFlags flags;
        
        // Tags are independent and combinable. ALLOWLIST by design: only this module's
        // own tags are looked up. Any other tag (controller tweak tags dab/dnd/nolog/kso,
        // notweak, the retired 'blocked', future tags, typos) is never queried here, so it
        // has zero Zygisk effect. Do NOT turn this into an exhaustive switch/else that
        // would choke on unknown tags.
        //
        // CPU default = BLOCK (unmount). Only 'with_cpu' opts into mounting the CPU spoof.
        // So a package with no tag now unmounts (safe default); the old 'blocked' tag is
        // retired — it behaves identically to no tag.
        if (tags.find("with_cpu") != tags.end()) {
            flags.needs_cpu_mount = true;
        } else {
            flags.needs_cpu_unmount = true;
        }
        if (tags.find("cow") != tags.end()) {
            flags.needs_cow = true;
        }

        return flags;
    }

    bool executeCompanionCommand(const std::string& command) {
        auto fd = api->connectCompanion();
        if (fd < 0) return false;
        write(fd, command.c_str(), command.size());
        int result = -1;
        read(fd, &result, sizeof(result));
        close(fd);
        return result == 0;
    }

    void ensureBuildClass() {
        if (buildClass) return;
        
        jclass localBuild = env->FindClass("android/os/Build");
        if (!localBuild) { env->ExceptionClear(); return; }

        buildClass = static_cast<jclass>(env->NewGlobalRef(localBuild));
        env->DeleteLocalRef(localBuild);
        if (!buildClass) return;

        modelField = env->GetStaticFieldID(buildClass, "MODEL", "Ljava/lang/String;");
        brandField = env->GetStaticFieldID(buildClass, "BRAND", "Ljava/lang/String;");
        deviceField = env->GetStaticFieldID(buildClass, "DEVICE", "Ljava/lang/String;");
        manufacturerField = env->GetStaticFieldID(buildClass, "MANUFACTURER", "Ljava/lang/String;");
        fingerprintField = env->GetStaticFieldID(buildClass, "FINGERPRINT", "Ljava/lang/String;");
        productField = env->GetStaticFieldID(buildClass, "PRODUCT", "Ljava/lang/String;");
        // SERIAL exists on all Androids (reads "unknown" on 8+); guard in case it throws.
        serialField = env->GetStaticFieldID(buildClass, "SERIAL", "Ljava/lang/String;");
        if (env->ExceptionCheck()) { env->ExceptionClear(); serialField = nullptr; }

        jclass localVersion = env->FindClass("android/os/Build$VERSION");
        if (localVersion) {
            versionClass = static_cast<jclass>(env->NewGlobalRef(localVersion));
            env->DeleteLocalRef(localVersion);
            if (versionClass) {
                releaseField = env->GetStaticFieldID(versionClass, "RELEASE", "Ljava/lang/String;");
                sdkIntField = env->GetStaticFieldID(versionClass, "SDK_INT", "I");
            }
        }

        if (env->ExceptionCheck()) {
            env->ExceptionClear();
            if (buildClass) env->DeleteGlobalRef(buildClass);
            if (versionClass) env->DeleteGlobalRef(versionClass);
            buildClass = versionClass = nullptr;
        }
    }

    void reloadIfNeeded(bool force = false) {
        struct stat file_stat;
        if (stat(config_path.c_str(), &file_stat) != 0) return;

        time_t current_mtime = file_stat.st_mtime;
        if (!force && current_mtime == last_config_mtime) return;

        CONFIG_LOG("Loading config...");
        std::ifstream file(config_path);
        if (!file.is_open()) return;

        try {
            json config = json::parse(file);
            // ✅ CORRECT: DeviceInfo + map of package_name -> PackageFlags
            std::vector<std::pair<DeviceInfo, std::unordered_map<std::string, PackageFlags>>> new_device_packages;
            
            cpu_blacklist.clear();
            cpu_only_packages.clear();
            
            if (config.contains("cpu_spoof")) {
                auto cpu_spoof_config = config["cpu_spoof"];
                if (cpu_spoof_config.contains("blacklist")) {
                    for (const auto& pkg : cpu_spoof_config["blacklist"]) cpu_blacklist.insert(pkg.get<std::string>());
                }
                if (cpu_spoof_config.contains("cpu_only_packages")) {
                    // entries may carry controller tweak tags (dnd/dab/kso/nolog) — strip them;
                    // we only need the clean package name for cpu-only matching.
                    for (const auto& pkg : cpu_spoof_config["cpu_only_packages"]) {
                        auto [name, tags] = parsePackageWithTags(pkg.get<std::string>());
                        (void)tags;
                        cpu_only_packages.insert(name);
                    }
                }
            }

            int device_count = 0;
            for (auto& [key, value] : config.items()) {
                if (key.find("PACKAGES_") == 0 && key.rfind("_DEVICE") != key.size() - 7) {
                    std::string device_key = key + "_DEVICE";
                    if (!config.contains(device_key) || !config[device_key].is_object()) continue;
                    
                    auto device = config[device_key];
                    DeviceInfo info;
                    info.brand = device.value("BRAND", "generic");
                    info.device = device.value("DEVICE", "generic");
                    info.manufacturer = device.value("MANUFACTURER", "generic");
                    info.model = device.value("MODEL", "generic");
                    info.fingerprint = device.value("FINGERPRINT", "generic/brand/device:13/TQ3A.230805.001/123456:user/release-keys");
                    info.product = device.value("PRODUCT", info.brand);
                    info.serial = device.value("SERIAL", "");
                    info.should_spoof_serial = !info.serial.empty();
                    info.android_id = device.value("ANDROID_ID", "");
                    info.should_spoof_android_id = !info.android_id.empty();

                    // Auto-generate props
                    info.prop_overrides["ro.product.model"] = info.model;
                    info.prop_overrides["ro.product.brand"] = info.brand;
                    info.prop_overrides["ro.product.manufacturer"] = info.manufacturer;
                    info.prop_overrides["ro.product.device"] = info.device;
                    info.prop_overrides["ro.build.fingerprint"] = info.fingerprint;
                    info.prop_overrides["ro.product.name"] = info.product;
                    
                    if (device.contains("PROPS") && device["PROPS"].is_object()) {
                        for (auto& [pk, pv] : device["PROPS"].items()) {
                            info.prop_overrides[pk] = pv.get<std::string>();
                        }
                    }

                    if (device.contains("ANDROID_VERSION")) {
                        try {
                            if (device["ANDROID_VERSION"].is_string()) {
                                info.android_version = device["ANDROID_VERSION"].get<std::string>();
                                info.should_spoof_android_version = !info.android_version.empty();
                            } else if (device["ANDROID_VERSION"].is_number()) {
                                info.android_version = std::to_string(device["ANDROID_VERSION"].get<int>());
                                info.should_spoof_android_version = true;
                            }
                        } catch (...) { info.should_spoof_android_version = false; }
                    }

                    if (device.contains("SDK_INT")) {
                        try {
                            if (device["SDK_INT"].is_number()) {
                                info.sdk_int = device["SDK_INT"].get<int>();
                                info.should_spoof_sdk_int = true;
                            } else if (device["SDK_INT"].is_string()) {
                                std::string sdk_str = device["SDK_INT"].get<std::string>();
                                if (!sdk_str.empty()) {
                                    info.sdk_int = std::stoi(sdk_str);
                                    info.should_spoof_sdk_int = true;
                                }
                            }
                        } catch (...) { info.should_spoof_sdk_int = false; }
                    }

                    // ✅ CORRECT: Build map of package_name -> PackageFlags
                    std::unordered_map<std::string, PackageFlags> package_settings;
                    if (value.is_array()) {
                        for (const auto& pkg_entry : value) {
                            std::string pkg_str = pkg_entry.get<std::string>();
                            auto [pkg_name, tags] = parsePackageWithTags(pkg_str);
                            PackageFlags flags = getFlagsFromTags(tags);
                            package_settings[pkg_name] = flags;
                        }
                    }
                    
                    // ✅ Store ONE entry per device with ALL its packages
                    new_device_packages.emplace_back(info, package_settings);
                    device_count++;
                }
            }

            {
                std::lock_guard<std::mutex> lock(info_mutex);
                device_packages = std::move(new_device_packages);
            }

            last_config_mtime = current_mtime;
            CONFIG_LOG("Loaded: %d devices, %zu cpu_only, %zu blacklist", device_count, cpu_only_packages.size(), cpu_blacklist.size());
        } catch (const std::exception& e) {
            LOGE("Config error: %s", e.what());
        }
        file.close();
    }

    void spoofDevice(const DeviceInfo& info) {
        if (!buildClass) return;

        auto setStr = [&](jfieldID field, const std::string& value) {
            if (!field) return;
            jstring js = env->NewStringUTF(value.c_str());
            if (!js || env->ExceptionCheck()) { env->ExceptionClear(); return; }
            env->SetStaticObjectField(buildClass, field, js);
            env->DeleteLocalRef(js);
            if (env->ExceptionCheck()) env->ExceptionClear();
        };

        auto setInt = [&](jfieldID field, int value) {
            if (!field) return;
            env->SetStaticIntField(versionClass, field, value);
            if (env->ExceptionCheck()) env->ExceptionClear();
        };

        setStr(modelField, info.model);
        setStr(brandField, info.brand);
        setStr(deviceField, info.device);
        setStr(manufacturerField, info.manufacturer);
        setStr(fingerprintField, info.fingerprint);
        setStr(productField, info.product);
        
        if (info.should_spoof_android_version && versionClass && releaseField) setStr(releaseField, info.android_version);
        if (info.should_spoof_sdk_int && versionClass && sdkIntField) setInt(sdkIntField, info.sdk_int);
        if (info.should_spoof_serial && serialField) setStr(serialField, info.serial);

        SPOOF_LOG("Device spoofed: %s (%s)%s", info.model.c_str(), info.brand.c_str(),
                  info.should_spoof_serial ? " +serial" : "");
    }
};

REGISTER_ZYGISK_MODULE(COPGModule)
REGISTER_ZYGISK_COMPANION(companion)
