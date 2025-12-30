#include <jni.h>
#include <string>
#include <zygisk.hpp>
#include <nlohmann/json.hpp>
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

static bool debug_mode = false;

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
static std::once_flag build_once;

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

static void companion(int fd) {
    COMPANION_LOG("Started");
    
    char buffer[2048];
    ssize_t bytes = read(fd, buffer, sizeof(buffer)-1);
    
    if (bytes > 0) {
        buffer[bytes] = '\0';
        std::string command = buffer;
        
        int result = -1;
        
        if (command == "unmount_spoof") {
            result = system("/system/bin/umount /proc/cpuinfo 2>/dev/null");
            COMPANION_LOG("CPU unmount");
        } else if (command == "mount_spoof") {
            if (access(spoof_file_path, F_OK) == 0) {
                system("/system/bin/umount /proc/cpuinfo 2>/dev/null");
                char mount_cmd[512];
                snprintf(mount_cmd, sizeof(mount_cmd), 
                        "/system/bin/mount --bind %s /proc/cpuinfo", spoof_file_path);
                result = system(mount_cmd);
                COMPANION_LOG("CPU mount");
            } else {
                LOGE("Spoof file missing: %s", spoof_file_path);
            }
        } else if (command == "read_build_props") {
            result = 0;
            COMPANION_LOG("Build props read (no action required)");
        } else if (command == "restore_build_props") {
            result = 0;
            COMPANION_LOG("Build props restore (no action required)");
        }
        
        write(fd, &result, sizeof(result));
    }
    
    close(fd);
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
        if (buildClass) {
            env->DeleteGlobalRef(buildClass);
            buildClass = nullptr;
        }
        if (versionClass) {
            env->DeleteGlobalRef(versionClass);
            versionClass = nullptr;
        }
    }

    void preAppSpecialize(zygisk::AppSpecializeArgs* args) override {
        if (!args || !args->nice_name) {
            LOGI("No package name, closing module");
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
        reloadIfNeeded(false);

        bool should_close = true;
        bool current_needs_device_spoof = false;
        bool current_needs_cpu_spoof = false;
        bool should_unmount_cpu = false;
        bool is_blacklisted = false;
        bool is_cpu_only = false;
        
        {
            std::lock_guard<std::mutex> lock(info_mutex);
            
            DeviceInfo device_info;
            std::string package_setting = "";
            bool found_in_device_list = false;

            for (auto& device_entry : device_packages) {
                auto it = device_entry.second.find(package_name);
                if (it != device_entry.second.end()) {
                    found_in_device_list = true;
                    package_setting = it->second;
                    current_needs_device_spoof = true;
                    device_info = device_entry.first;
                    current_info = device_info;
                    
                    if (package_setting == "with_cpu") {
                        current_needs_cpu_spoof = true;
                    } else if (package_setting == "blocked") {
                        should_unmount_cpu = true;
                    }
                    break;
                }
            }

            is_blacklisted = (cpu_blacklist.find(package_name) != cpu_blacklist.end());
            is_cpu_only = (cpu_only_packages.find(package_name) != cpu_only_packages.end());

            if (is_blacklisted) {
                should_unmount_cpu = true;
                PKG_LOG("%s: CPU blacklisted", package_name);
            }

            if (is_cpu_only && !found_in_device_list && !is_blacklisted) {
                current_needs_cpu_spoof = true;
                PKG_LOG("%s: CPU spoof only", package_name);
            }

            if (found_in_device_list && package_setting.empty() && !is_blacklisted && is_cpu_only) {
                current_needs_cpu_spoof = true;
            }

            if (current_needs_device_spoof && current_needs_cpu_spoof) {
                PKG_LOG("%s: Device+CPU spoof", package_name);
            } else if (current_needs_device_spoof) {
                PKG_LOG("%s: Device spoof only", package_name);
            } else if (current_needs_cpu_spoof) {
                PKG_LOG("%s: CPU spoof only", package_name);
            } else if (should_unmount_cpu) {
                PKG_LOG("%s: CPU blocked", package_name);
            }

            if (current_needs_device_spoof) {
                spoofDevice(current_info);
                should_close = false;
            }

            if (should_unmount_cpu) {
                executeCompanionCommand("unmount_spoof");
            } else if (current_needs_cpu_spoof) {
                executeCompanionCommand("mount_spoof");
            }

            if (current_needs_device_spoof || current_needs_cpu_spoof || is_blacklisted) {
                should_close = false;
            }
        }

        if (should_close) {
            LOGI("%s: Not in config, closing", package_name);
            api->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
        } else {
            api->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
        }
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs* args) override {
        api->setOption(zygisk::DLCLOSE_MODULE_LIBRARY);
    }

private:
    zygisk::Api* api;
    JNIEnv* env;
    std::vector<std::pair<DeviceInfo, std::unordered_map<std::string, std::string>>> device_packages;

    std::pair<std::string, std::unordered_set<std::string>> parsePackageWithTags(const std::string& package_str) {
        std::string package_name = package_str;
        std::unordered_set<std::string> tags;
        
        package_name.erase(0, package_name.find_first_not_of(" \t"));
        package_name.erase(package_name.find_last_not_of(" \t") + 1);
        
        size_t first_colon = package_name.find(':');
        if (first_colon != std::string::npos && first_colon < package_name.length() - 1) {
            std::string original_name = package_name;
            package_name = original_name.substr(0, first_colon);
            
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
                
                tag.erase(0, tag.find_first_not_of(" \t"));
                tag.erase(tag.find_last_not_of(" \t") + 1);
                
                if (!tag.empty()) {
                    tags.insert(tag);
                }
            }
        }
        
        return {package_name, tags};
    }

    std::string getZygiskSettingFromTags(const std::unordered_set<std::string>& tags) {
        if (tags.find("blocked") != tags.end()) {
            return "blocked";
        } else if (tags.find("with_cpu") != tags.end()) {
            return "with_cpu";
        }
        
        return "";
    }

    bool executeCompanionCommand(const std::string& command) {
        auto fd = api->connectCompanion();
        if (fd < 0) {
            return false;
        }
        
        write(fd, command.c_str(), command.size());
        
        int result = -1;
        read(fd, &result, sizeof(result));
        close(fd);
        
        return result == 0;
    }

    void ensureBuildClass() {
        std::call_once(build_once, [&] {
            jclass localBuild = env->FindClass("android/os/Build");
            if (!localBuild) {
                env->ExceptionClear();
                return;
            }

            buildClass = static_cast<jclass>(env->NewGlobalRef(localBuild));
            env->DeleteLocalRef(localBuild);
            if (!buildClass) {
                return;
            }

            modelField = env->GetStaticFieldID(buildClass, "MODEL", "Ljava/lang/String;");
            brandField = env->GetStaticFieldID(buildClass, "BRAND", "Ljava/lang/String;");
            deviceField = env->GetStaticFieldID(buildClass, "DEVICE", "Ljava/lang/String;");
            manufacturerField = env->GetStaticFieldID(buildClass, "MANUFACTURER", "Ljava/lang/String;");
            fingerprintField = env->GetStaticFieldID(buildClass, "FINGERPRINT", "Ljava/lang/String;");
            productField = env->GetStaticFieldID(buildClass, "PRODUCT", "Ljava/lang/String;");

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
                buildClass = nullptr;
                versionClass = nullptr;
            }
        });
    }

    void reloadIfNeeded(bool force = false) {
        struct stat file_stat;
        if (stat(config_path.c_str(), &file_stat) != 0) {
            CONFIG_LOG("Config missing: %s", config_path.c_str());
            return;
        }

        time_t current_mtime = file_stat.st_mtime;
        if (!force && current_mtime == last_config_mtime) {
            return;
        }

        CONFIG_LOG("Loading config...");

        std::ifstream file(config_path);
        if (!file.is_open()) {
            CONFIG_LOG("Failed to open config");
            return;
        }

        try {
            json config = json::parse(file);
            std::vector<std::pair<DeviceInfo, std::unordered_map<std::string, std::string>>> new_device_packages;
            
            cpu_blacklist.clear();
            cpu_only_packages.clear();
            
            if (config.contains("cpu_spoof")) {
                auto cpu_spoof_config = config["cpu_spoof"];
                
                if (cpu_spoof_config.contains("blacklist")) {
                    for (const auto& pkg : cpu_spoof_config["blacklist"]) {
                        cpu_blacklist.insert(pkg.get<std::string>());
                    }
                }
                
                if (cpu_spoof_config.contains("cpu_only_packages")) {
                    for (const auto& pkg : cpu_spoof_config["cpu_only_packages"]) {
                        cpu_only_packages.insert(pkg.get<std::string>());
                    }
                }
            }

            int device_count = 0;
            for (auto& [key, value] : config.items()) {
                if (key.find("PACKAGES_") == 0 && key.rfind("_DEVICE") != key.size() - 7) {
                    std::string device_key = key + "_DEVICE";
                    if (!config.contains(device_key) || !config[device_key].is_object()) {
                        continue;
                    }
                    
                    auto device = config[device_key];
                    DeviceInfo info;
                    info.brand = device.value("BRAND", "generic");
                    info.device = device.value("DEVICE", "generic");
                    info.manufacturer = device.value("MANUFACTURER", "generic");
                    info.model = device.value("MODEL", "generic");
                    info.fingerprint = device.value("FINGERPRINT", "generic/brand/device:13/TQ3A.230805.001/123456:user/release-keys");
                    info.product = device.value("PRODUCT", info.brand);

                    if (device.contains("ANDROID_VERSION")) {
                        try {
                            if (device["ANDROID_VERSION"].is_string()) {
                                info.android_version = device["ANDROID_VERSION"].get<std::string>();
                                info.should_spoof_android_version = !info.android_version.empty();
                            } else if (device["ANDROID_VERSION"].is_number()) {
                                info.android_version = std::to_string(device["ANDROID_VERSION"].get<int>());
                                info.should_spoof_android_version = true;
                            }
                        } catch (const std::exception& e) {
                            LOGW("Failed to parse ANDROID_VERSION: %s", e.what());
                            info.should_spoof_android_version = false;
                        }
                    } else {
                        info.should_spoof_android_version = false;
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
                        } catch (const std::exception& e) {
                            LOGW("Failed to parse SDK_INT: %s", e.what());
                            info.should_spoof_sdk_int = false;
                        }
                    } else {
                        info.should_spoof_sdk_int = false;
                    }

                    std::unordered_map<std::string, std::string> package_settings;
                    
                    if (value.is_array()) {
                        for (const auto& pkg_entry : value) {
                            std::string pkg_str = pkg_entry.get<std::string>();
                            
                            auto [pkg_name, tags] = parsePackageWithTags(pkg_str);
                            std::string setting = getZygiskSettingFromTags(tags);
                            
                            package_settings[pkg_name] = setting;
                        }
                    }
                    
                    new_device_packages.emplace_back(info, package_settings);
                    device_count++;
                }
            }

            {
                std::lock_guard<std::mutex> lock(info_mutex);
                device_packages = std::move(new_device_packages);
            }

            last_config_mtime = current_mtime;
            CONFIG_LOG("Loaded: %d devices, %zu cpu_only, %zu blacklist", 
                      device_count, cpu_only_packages.size(), cpu_blacklist.size());
        } catch (const json::exception& e) {
            LOGE("JSON error: %s", e.what());
        } catch (const std::exception& e) {
            LOGE("Config error: %s", e.what());
        }
        file.close();
    }

    void spoofDevice(const DeviceInfo& info) {
        if (!buildClass) {
            return;
        }

        auto setStr = [&](jfieldID field, const std::string& value) {
            if (!field) return;
            jstring js = env->NewStringUTF(value.c_str());
            if (!js || env->ExceptionCheck()) {
                env->ExceptionClear();
                return;
            }
            env->SetStaticObjectField(buildClass, field, js);
            env->DeleteLocalRef(js);
            if (env->ExceptionCheck()) {
                env->ExceptionClear();
            }
        };

        auto setInt = [&](jfieldID field, int value) {
            if (!field) return;
            env->SetStaticIntField(versionClass, field, value);
            if (env->ExceptionCheck()) {
                env->ExceptionClear();
            }
        };

        setStr(modelField, info.model);
        setStr(brandField, info.brand);
        setStr(deviceField, info.device);
        setStr(manufacturerField, info.manufacturer);
        setStr(fingerprintField, info.fingerprint);
        setStr(productField, info.product);
        
        if (info.should_spoof_android_version && versionClass && releaseField) {
            setStr(releaseField, info.android_version);
        }
        
        if (info.should_spoof_sdk_int && versionClass && sdkIntField) {
            setInt(sdkIntField, info.sdk_int);
        }
        
        SPOOF_LOG("Device spoofed: %s (%s)", info.model.c_str(), info.brand.c_str());
    }
};

REGISTER_ZYGISK_MODULE(COPGModule)
REGISTER_ZYGISK_COMPANION(companion)
