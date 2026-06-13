#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <thread>
#include <chrono>
#include <atomic>
#include <memory>
#include <csignal>
#include <functional>
#include <system_error>
#include <dirent.h>
#include <sys/stat.h>
#include <cstring>
#include "json.hpp"

#ifdef __linux__
    #include <sys/inotify.h>
    #include <unistd.h>
    #include <poll.h>
    #include <errno.h>
#endif

using json = nlohmann::json;

namespace xwatcher {
    enum class FileEvent {
        Unspecified, Removed, Created, Modified, Opened, 
        AttributesChanged, None, Renamed, Moved
    };

    class Watcher {
    private:
        struct File {
            std::string name;
            int context;
            void* additional_data;
            std::function<void(FileEvent, const std::string&, int, void*)> callback;
        };

        struct Directory {
            std::vector<File> files;
            std::string path;
            int context;
            void* additional_data;
            std::function<void(FileEvent, const std::string&, int, void*)> callback;

            #ifdef __linux__
                int inotify_watch_fd = -1;
            #endif
        };

        std::vector<Directory> directories;
        std::thread worker_thread;
        std::atomic<bool> alive{false};
        std::atomic<bool> initialized{false};

        #ifdef __linux__
            int inotify_fd = -1;
        #endif

        void process_events() {
            #ifdef __linux__
                constexpr size_t EVENT_SIZE = sizeof(struct inotify_event);
                constexpr size_t BUF_LEN = 1024 * (EVENT_SIZE + 16);
                char buffer[BUF_LEN];
                
                while (alive.load()) {
                    if (inotify_fd < 0) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                        continue;
                    }

                    struct pollfd pfd = { inotify_fd, POLLIN, 0 };
                    int ret = poll(&pfd, 1, 100);

                    if (ret < 0) {
                        if (errno == EINTR) continue;
                        std::this_thread::sleep_for(std::chrono::seconds(1));
                        continue;
                    } else if (ret == 0) {
                        continue;
                    }

                    ssize_t length = read(inotify_fd, buffer, BUF_LEN);
                    if (length < 0) {
                        continue;
                    }

                    ssize_t i = 0;
                    while (i < length) {
                        struct inotify_event* event = reinterpret_cast<struct inotify_event*>(&buffer[i]);
                        
                        if (event->mask & IN_IGNORED) {
                            i += EVENT_SIZE + event->len;
                            continue;
                        }

                        Directory* directory = nullptr;
                        for (auto& dir : directories) {
                            if (dir.inotify_watch_fd == event->wd) {
                                directory = &dir;
                                break;
                            }
                        }

                        if (!directory) {
                            i += EVENT_SIZE + event->len;
                            continue;
                        }

                        FileEvent file_event = FileEvent::None;
                        
                        if (event->mask & IN_CREATE) file_event = FileEvent::Created;
                        else if (event->mask & IN_MODIFY) file_event = FileEvent::Modified;
                        else if (event->mask & IN_DELETE) file_event = FileEvent::Removed;
                        else if (event->mask & IN_DELETE_SELF) file_event = FileEvent::Removed;
                        else if (event->mask & IN_MOVE_SELF) file_event = FileEvent::Moved;
                        else if (event->mask & IN_MOVED_FROM) file_event = FileEvent::Moved;
                        else if (event->mask & IN_MOVED_TO) file_event = FileEvent::Created;
                        else if (event->mask & (IN_CLOSE_WRITE | IN_CLOSE_NOWRITE)) file_event = FileEvent::Opened;
                        else if (event->mask & IN_ATTRIB) file_event = FileEvent::AttributesChanged;
                        else if (event->mask & IN_OPEN) file_event = FileEvent::Opened;

                        if (file_event != FileEvent::None) {
                            try {
                                if (event->len > 0) {
                                    File* file = nullptr;
                                    for (auto& f : directory->files) {
                                        if (f.name == event->name) {
                                            file = &f;
                                            break;
                                        }
                                    }

                                    if (file && file->callback) {
                                        std::string full_path = directory->path + "/" + file->name;
                                        file->callback(file_event, full_path, file->context, file->additional_data);
                                    } else if (directory->callback) {
                                        std::string full_path = directory->path + "/" + event->name;
                                        directory->callback(file_event, full_path, directory->context, directory->additional_data);
                                    }
                                }
                            } catch (const std::exception& e) {
                                std::cerr << "Error in file event callback: " << e.what() << std::endl;
                            }
                        }

                        i += EVENT_SIZE + event->len;
                    }
                }
            #endif
        }

        std::string extract_filename(const std::string& path) {
            size_t pos = path.find_last_of("/\\");
            return (pos != std::string::npos) ? path.substr(pos + 1) : path;
        }

    public:
        Watcher() {
            #ifdef __linux__
                inotify_fd = inotify_init1(O_NONBLOCK);
                if (inotify_fd >= 0) {
                    initialized.store(true);
                }
            #endif
        }

        ~Watcher() {
            stop();
            #ifdef __linux__
                if (inotify_fd >= 0) {
                    close(inotify_fd);
                }
            #endif
        }

        bool is_initialized() const {
            return initialized.load();
        }

        bool add_file(const std::string& path, 
                     std::function<void(FileEvent, const std::string&, int, void*)> callback,
                     int context = 0, void* additional_data = nullptr) {
            
            if (!initialized.load()) {
                std::cerr << "Watcher not initialized" << std::endl;
                return false;
            }

            std::string clean_path = path;
            if (!clean_path.empty() && (clean_path.back() == '/' || clean_path.back() == '\\')) {
                clean_path.pop_back();
            }

            struct stat buffer;
            if (stat(clean_path.c_str(), &buffer) != 0) {
                std::cout << "File does not exist (may be created later): " << clean_path << std::endl;
            }

            std::string filename = extract_filename(clean_path);
            std::string directory_path = clean_path.substr(0, clean_path.length() - filename.length() - 1);

            Directory* directory = nullptr;
            for (auto& dir : directories) {
                if (dir.path == directory_path) {
                    directory = &dir;
                    break;
                }
            }

            if (!directory) {
                Directory new_dir;
                new_dir.path = directory_path;
                new_dir.context = 0;
                new_dir.additional_data = nullptr;
                new_dir.callback = nullptr;

                #ifdef __linux__
                    new_dir.inotify_watch_fd = inotify_add_watch(inotify_fd, directory_path.c_str(), 
                        IN_CREATE | IN_MODIFY | IN_DELETE | IN_DELETE_SELF | IN_MOVE_SELF | 
                        IN_MOVED_FROM | IN_MOVED_TO | IN_CLOSE_WRITE | IN_ATTRIB | IN_OPEN);
                    if (new_dir.inotify_watch_fd == -1) {
                        std::cerr << "Failed to add inotify watch for: " << directory_path << std::endl;
                        return false;
                    }
                #endif

                directories.push_back(std::move(new_dir));
                directory = &directories.back();
            }

            for (const auto& file : directory->files) {
                if (file.name == filename) {
                    std::cout << "File already being watched: " << filename << std::endl;
                    return false;
                }
            }

            File new_file;
            new_file.name = filename;
            new_file.context = context;
            new_file.additional_data = additional_data;
            new_file.callback = callback;

            directory->files.push_back(std::move(new_file));
            return true;
        }

        bool start() {
            if (alive.load()) {
                return false;
            }

            if (directories.empty()) {
                std::cerr << "No files to watch" << std::endl;
                return false;
            }

            if (!initialized.load()) {
                std::cerr << "Watcher not initialized properly" << std::endl;
                return false;
            }

            alive.store(true);
            worker_thread = std::thread(&Watcher::process_events, this);
            return true;
        }

        void stop() {
            alive.store(false);
            if (worker_thread.joinable()) {
                worker_thread.join();
            }
        }
    };
}

class AdvancedAppController {
private:
    const std::string CONFIG_JSON = "/data/adb/modules/COPG/COPG.json";
    const std::string DEFAULTS_FILE = "/data/adb/copg_defaults";

    // Per-app tweak tags (colon-suffixes in COPG.json). Each maps to one comfort toggle:
    //   dab   -> disable auto-brightness   (screen_brightness_mode 0)
    //   dnd   -> do-not-disturb            (cmd notification set_dnd priority)
    //   nolog -> disable logging           (stop logd)
    //   kso   -> keep screen on            (screen_off_timeout 300000000)
    const std::set<std::string> KNOWN_TWEAKS = {"dab", "dnd", "nolog", "kso"};

    std::unordered_map<std::string, bool> monitored_packages;
    std::unordered_map<std::string, bool> previous_monitored_packages;
    // package -> tweak tags it requests while active
    std::unordered_map<std::string, std::set<std::string>> package_tweaks;
    // tweaks currently applied to the system (originals backed up under DEFAULTS_FILE.*)
    std::set<std::string> applied_tweaks;
    std::atomic<bool> running{true};
    std::atomic<bool> config_loaded{false};
    std::atomic<bool> config_file_exists{false};

    xwatcher::Watcher config_watcher;

    enum class AppState {
        FOREGROUND,
        BACKGROUND,
        UNKNOWN
    };

    struct StateTracker {
        std::unordered_map<std::string, AppState> last_states;
        int no_change_counter{0};
        int cycle_counter{0};

        void reset() {
            last_states.clear();
            no_change_counter = 0;
            cycle_counter = 0;
        }
    } state_tracker;

public:
    AdvancedAppController() {
        std::cout << "🚀 Initializing Advanced App Controller..." << std::endl;
        
        if (!config_watcher.is_initialized()) {
            std::cerr << "❌ Config watcher failed to initialize" << std::endl;
            return;
        }

        if (!check_config_file()) {
            std::cerr << "⚠️ Config file not found, waiting for creation..." << std::endl;
            config_file_exists.store(false);
        } else {
            if (!load_config()) {
                std::cerr << "❌ Failed to load initial configuration" << std::endl;
                return;
            }
            config_file_exists.store(true);
        }
        
        restore_saved_states();
        
        if (!setup_config_watcher()) {
            std::cerr << "❌ Failed to setup config watcher" << std::endl;
            return;
        }

        config_loaded.store(true);
        std::cout << "✅ Controller initialized successfully" << std::endl;
    }

    ~AdvancedAppController() {
        stop();
    }

    void stop() {
        running = false;
        config_watcher.stop();
    }

    bool is_ready() const {
        return config_loaded.load();
    }

private:
    bool check_config_file() {
        struct stat buffer;
        return stat(CONFIG_JSON.c_str(), &buffer) == 0;
    }

    std::pair<std::string, std::unordered_set<std::string>> parsePackageWithTags(const std::string& package_str) {
        std::string package_name = package_str;
        std::unordered_set<std::string> tags;
        
        size_t colon_pos = package_str.find(':');
        if (colon_pos != std::string::npos) {
            package_name = package_str.substr(0, colon_pos);
            
            std::string tags_part = package_str.substr(colon_pos + 1);
            size_t start = 0;
            size_t end = tags_part.find(':');
            
            while (end != std::string::npos) {
                std::string tag = tags_part.substr(start, end - start);
                if (!tag.empty()) {
                    tags.insert(tag);
                }
                start = end + 1;
                end = tags_part.find(':', start);
            }
            
            std::string last_tag = tags_part.substr(start);
            if (!last_tag.empty()) {
                tags.insert(last_tag);
            }
        }
        
        return {package_name, tags};
    }

    bool setup_config_watcher() {
        auto callback = [this](xwatcher::FileEvent event, const std::string&, int, void*) {
            try {
                if (event == xwatcher::FileEvent::Modified || event == xwatcher::FileEvent::Created) {
                    std::cout << "🔄 Config file changed - reloading..." << std::endl;
                    safe_reload_config();
                } else if (event == xwatcher::FileEvent::Removed) {
                    std::cout << "🗑️ Config file deleted - clearing packages..." << std::endl;
                    handle_empty_config();
                }
            } catch (const std::exception& e) {
                std::cerr << "❌ Error in config watcher callback: " << e.what() << std::endl;
            }
        };

        if (!config_watcher.add_file(CONFIG_JSON, callback)) {
            std::cerr << "❌ Failed to add config file to watcher" << std::endl;
            return false;
        }

        if (!config_watcher.start()) {
            std::cerr << "❌ Failed to start config watcher" << std::endl;
            return false;
        }

        std::cout << "👀 Built-in config watcher started" << std::endl;
        return true;
    }

    void safe_reload_config() {
        struct stat buffer;
        if (stat(CONFIG_JSON.c_str(), &buffer) != 0) {
            std::cout << "⚠️ Config file deleted, clearing all packages" << std::endl;
            handle_empty_config();
            return;
        }

        for (int attempt = 0; attempt < 3; attempt++) {
            if (load_config()) {
                handle_config_changes();
                state_tracker.reset();
                config_file_exists.store(true);
                std::cout << "✅ Configuration reloaded successfully" << std::endl;
                return;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100 * (attempt + 1)));
        }
        std::cerr << "❌ Failed to reload configuration - invalid JSON" << std::endl;
    }

    void handle_empty_config() {
        std::cout << "🔄 Handling empty config - clearing packages and restoring system states" << std::endl;
        
        monitored_packages.clear();
        previous_monitored_packages.clear();
        package_tweaks.clear();

        sync_tweaks({});  // restore everything currently applied

        state_tracker.reset();
        config_file_exists.store(false);
        
        std::cout << "✅ All packages cleared, waiting for new config file..." << std::endl;
    }

    void handle_config_changes() {
        if (previous_monitored_packages.empty()) {
            previous_monitored_packages = monitored_packages;
            return;
        }

        if (monitored_packages.empty()) {
            sync_tweaks({});  // nothing monitored -> restore all
            previous_monitored_packages = monitored_packages;
            return;
        }

        std::vector<std::string> removed_packages;
        std::vector<std::string> added_packages;

        for (const auto& [package, installed] : previous_monitored_packages) {
            if (monitored_packages.find(package) == monitored_packages.end()) {
                removed_packages.push_back(package);
            }
        }
        for (const auto& [package, installed] : monitored_packages) {
            if (previous_monitored_packages.find(package) == previous_monitored_packages.end()) {
                added_packages.push_back(package);
            }
        }

        auto log_list = [](const char* label, const std::vector<std::string>& v) {
            if (v.empty()) return;
            std::cout << label;
            for (size_t i = 0; i < v.size(); ++i) {
                std::cout << v[i];
                if (i + 1 < v.size()) std::cout << ", ";
            }
            std::cout << std::endl;
        };
        log_list("📋 Packages removed from config: ", removed_packages);
        log_list("📋 Packages added to config: ", added_packages);

        // Re-evaluate the live system against the new config immediately.
        reconcile();

        previous_monitored_packages = monitored_packages;
    }

    bool load_config() {
        try {
            std::ifstream config_file(CONFIG_JSON);
            if (!config_file.is_open()) {
                std::cerr << "⚠️ Config file not found or inaccessible: " << CONFIG_JSON << std::endl;
                monitored_packages.clear();
                package_tweaks.clear();
                previous_monitored_packages.clear();
                return false;
            }
            
            json config;
            try {
                config_file >> config;
            } catch (const json::exception& e) {
                std::cerr << "❌ Invalid JSON in config file: " << e.what() << std::endl;
                return false;
            }
            
            std::unordered_map<std::string, bool> new_packages;
            std::unordered_map<std::string, std::set<std::string>> new_package_tweaks;
            std::set<std::string> cpu_only_packages;
            int total_packages = 0;
            int cpu_only_count = 0;

            if (config.contains("cpu_spoof")) {
                auto cpu_spoof_config = config["cpu_spoof"];

                if (cpu_spoof_config.contains("cpu_only_packages")) {
                    for (const auto& pkg : cpu_spoof_config["cpu_only_packages"]) {
                        if (pkg.is_string()) {
                            // cpu_only entries may carry tweak tags too (dnd/dab/kso/nolog)
                            auto [name, tags] = parsePackageWithTags(pkg.get<std::string>());
                            cpu_only_packages.insert(name);
                            cpu_only_count++;
                            std::set<std::string> tweaks;
                            for (const auto& tag : tags) {
                                if (KNOWN_TWEAKS.find(tag) != KNOWN_TWEAKS.end()) tweaks.insert(tag);
                            }
                            if (!tweaks.empty()) {
                                new_package_tweaks[name] = tweaks;
                                std::cout << "🎛️ Tweaks for " << name << " (cpu_only): ";
                                for (const auto& t : tweaks) std::cout << t << " ";
                                std::cout << std::endl;
                            }
                        }
                    }
                }
            }
            
            for (auto& [key, value] : config.items()) {
                if (key.find("PACKAGES_") == 0 && key.rfind("_DEVICE") != key.size() - 7) {
                    for (auto& package_entry : value) {
                        if (package_entry.is_string()) {
                            std::string package_str = package_entry.get<std::string>();
                            
                            auto [package_name, tags] = parsePackageWithTags(package_str);

                            new_packages[package_name] = false;
                            total_packages++;

                            // Per-app tweak tags (subset of tags that are comfort toggles)
                            std::set<std::string> tweaks;
                            for (const auto& tag : tags) {
                                if (KNOWN_TWEAKS.find(tag) != KNOWN_TWEAKS.end()) {
                                    tweaks.insert(tag);
                                }
                            }
                            if (!tweaks.empty()) {
                                new_package_tweaks[package_name] = tweaks;
                                std::cout << "🎛️ Tweaks for " << package_name << ": ";
                                for (const auto& t : tweaks) std::cout << t << " ";
                                std::cout << std::endl;
                            }
                        }
                    }
                }
            }

            for (const auto& cpu_only_pkg : cpu_only_packages) {
                new_packages[cpu_only_pkg] = false;
                total_packages++;
                std::cout << "🔧 Added CPU only package to monitoring: " << cpu_only_pkg << std::endl;
            }

            std::cout << "📋 Found " << total_packages << " packages in config" << std::endl;
            std::cout << "🔧 Found " << cpu_only_count << " CPU only packages" << std::endl;

            filter_installed_packages(new_packages);

            monitored_packages = std::move(new_packages);
            package_tweaks = std::move(new_package_tweaks);
            std::cout << "📦 Loaded " << monitored_packages.size() << " installed packages" << std::endl;
            std::cout << "🎛️ " << package_tweaks.size() << " packages have per-app tweaks" << std::endl;

            return true;

        } catch (const std::exception& e) {
            std::cerr << "❌ Error loading config: " << e.what() << std::endl;
            monitored_packages.clear();
            package_tweaks.clear();
            return false;
        }
    }

    void filter_installed_packages(std::unordered_map<std::string, bool>& packages) {
        if (packages.empty()) {
            return;
        }
        
        std::cout << "🔍 Checking installed packages using pm list packages..." << std::endl;
        
        std::string pm_result = execute_command("pm list packages");
        
        if (!pm_result.empty()) {
            std::unordered_map<std::string, bool> installed_packages;
            size_t pos = 0;
            int found_count = 0;
            
            while ((pos = pm_result.find("package:", pos)) != std::string::npos) {
                size_t end = pm_result.find("\n", pos);
                if (end == std::string::npos) break;
                
                std::string package_line = pm_result.substr(pos + 8, end - pos - 8);
                package_line.erase(std::remove(package_line.begin(), package_line.end(), '\r'), package_line.end());
                
                installed_packages[package_line] = true;
                found_count++;
                pos = end + 1;
            }
            
            std::cout << "   📊 Found " << found_count << " total packages in system" << std::endl;
            
            int matched_count = 0;
            for (auto it = packages.begin(); it != packages.end();) {
                if (installed_packages.find(it->first) != installed_packages.end()) {
                    it->second = true;
                    matched_count++;
                    ++it;
                } else {
                    it = packages.erase(it);
                }
            }
            
            std::cout << "   ✅ Package filtering completed: " << matched_count << " packages matched" << std::endl;
        } else {
            std::cerr << "   ❌ pm list packages failed" << std::endl;
            for (auto& [package, installed] : packages) {
                installed = true;
            }
        }
    }

    std::string execute_command(const std::string& cmd) {
        std::string full_cmd = cmd + " 2>/dev/null";
        FILE* pipe = popen(full_cmd.c_str(), "r");
        if (!pipe) {
            return "";
        }
        
        char buffer[256];
        std::string result = "";
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            result += buffer;
        }
        pclose(pipe);
        
        return result;
    }

    bool execute_command_bool(const std::string& cmd) {
        std::string full_cmd = cmd + " >/dev/null 2>&1";
        return system(full_cmd.c_str()) == 0;
    }

    std::unordered_map<std::string, AppState> get_all_active_apps() {
        std::unordered_map<std::string, AppState> active_apps;
        
        if (monitored_packages.empty()) {
            return active_apps;
        }
        
        std::string foreground_cmd = "dumpsys window visible-apps 2>/dev/null | grep -o -E 'package=[^ ]+' | cut -d'=' -f2";
        std::string foreground_result = execute_command(foreground_cmd);
        
        if (!foreground_result.empty()) {
            std::istringstream iss(foreground_result);
            std::string package;
            while (std::getline(iss, package)) {
                package.erase(std::remove(package.begin(), package.end(), '\r'), package.end());
                if (!package.empty() && monitored_packages.find(package) != monitored_packages.end()) {
                    active_apps[package] = AppState::FOREGROUND;
                }
            }
        }
        
        std::string background_cmd = "dumpsys window windows 2>/dev/null | grep -E 'Window #' | grep -o -E '[a-zA-Z0-9\\._-]+\\.[a-zA-Z0-9\\._-]+/[a-zA-Z0-9\\._-]+' | cut -d'/' -f1 | uniq";
        std::string background_result = execute_command(background_cmd);
        
        if (!background_result.empty()) {
            std::istringstream iss(background_result);
            std::string package;
            while (std::getline(iss, package)) {
                package.erase(std::remove(package.begin(), package.end(), '\r'), package.end());
                if (!package.empty() &&
                    monitored_packages.find(package) != monitored_packages.end() &&
                    active_apps.find(package) == active_apps.end()) {
                    active_apps[package] = AppState::BACKGROUND;
                }
            }
        }
        
        return active_apps;
    }

    // Union of tweak tags requested by all currently-active (FG/BG) apps.
    // active_apps only contains monitored packages (see get_all_active_apps).
    std::set<std::string> compute_desired_tweaks(const std::unordered_map<std::string, AppState>& active_apps) {
        std::set<std::string> desired;
        for (const auto& [package, state] : active_apps) {
            auto it = package_tweaks.find(package);
            if (it != package_tweaks.end()) {
                desired.insert(it->second.begin(), it->second.end());
            }
        }
        return desired;
    }

    // Apply the tweaks in `desired` and restore any currently-applied tweak no longer wanted.
    // Per-tweak diff: each tweak's original is backed up on first apply, restored when it leaves.
    void sync_tweaks(const std::set<std::string>& desired) {
        for (const auto& tag : desired) {
            if (applied_tweaks.find(tag) == applied_tweaks.end()) {
                apply_tweak(tag);
                applied_tweaks.insert(tag);
                std::cout << "🎛️ Applied tweak: " << tag << std::endl;
            }
        }
        for (auto it = applied_tweaks.begin(); it != applied_tweaks.end(); ) {
            if (desired.find(*it) == desired.end()) {
                restore_tweak(*it);
                std::cout << "🔄 Restored tweak: " << *it << std::endl;
                it = applied_tweaks.erase(it);
            } else {
                ++it;
            }
        }
    }

    // Read the live foreground/background set and reconcile applied tweaks against it.
    void reconcile() {
        auto active_apps = get_all_active_apps();
        sync_tweaks(compute_desired_tweaks(active_apps));
    }

    bool has_state_changed(const std::unordered_map<std::string, AppState>& current_states) {
        if (current_states.size() != state_tracker.last_states.size()) {
            return true;
        }
        
        for (const auto& [package, state] : current_states) {
            auto it = state_tracker.last_states.find(package);
            if (it == state_tracker.last_states.end() || it->second != state) {
                return true;
            }
        }
        
        return false;
    }

    void log_state_changes(const std::unordered_map<std::string, AppState>& current_states) {
        state_tracker.cycle_counter++;
        
        if (!has_state_changed(current_states)) {
            state_tracker.no_change_counter++;
            if (state_tracker.no_change_counter == 1 || state_tracker.no_change_counter % 50 == 0) {
                std::cout << "📱 System stable - No state changes (" << state_tracker.no_change_counter << " cycles)" << std::endl;
            }
            return;
        }
        
        state_tracker.no_change_counter = 0;
        
        std::cout << "🔄 State changes detected (Cycle: " << state_tracker.cycle_counter << "):" << std::endl;
        
        for (const auto& [package, state] : current_states) {
            auto it = state_tracker.last_states.find(package);
            if (it == state_tracker.last_states.end()) {
                std::cout << "   ➕ " << package << ": " << state_to_string(state) << std::endl;
            } else if (it->second != state) {
                std::cout << "   🔄 " << package << ": " << state_to_string(it->second) 
                          << " → " << state_to_string(state) << std::endl;
            }
        }
        
        for (const auto& [package, old_state] : state_tracker.last_states) {
            if (current_states.find(package) == current_states.end()) {
                std::cout << "   ❌ " << package << ": CLOSED" << std::endl;
            }
        }
        
        state_tracker.last_states = current_states;
    }

    static std::string trim(const std::string& s) {
        size_t a = s.find_first_not_of(" \t\r\n");
        if (a == std::string::npos) return "";
        size_t b = s.find_last_not_of(" \t\r\n");
        return s.substr(a, b - a + 1);
    }

    bool file_exists(const std::string& path) {
        struct stat b;
        return stat(path.c_str(), &b) == 0;
    }

    // Back up a setting's current value once (skip if a backup already exists).
    void save_setting_once(const std::string& suffix, const std::string& get_cmd) {
        std::string path = DEFAULTS_FILE + suffix;
        if (file_exists(path)) return;
        std::string val = trim(execute_command(get_cmd));
        if (val.empty() || val == "null") return;
        std::ofstream f(path);
        if (f) {
            f << val;
            f.close();
            execute_command_bool("chmod 644 " + path);
        }
    }

    void restore_setting(const std::string& suffix, const std::string& put_prefix) {
        std::string path = DEFAULTS_FILE + suffix;
        std::ifstream f(path);
        if (f) {
            std::string val;
            f >> val;
            if (!val.empty()) execute_command_bool(put_prefix + val);
        }
        execute_command_bool("rm -f " + path);
    }

    void restore_dnd() {
        std::string path = DEFAULTS_FILE + ".dnd";
        std::ifstream f(path);
        std::string cmd = "cmd notification set_dnd off";
        if (f) {
            std::string val;
            f >> val;
            if (val == "1") cmd = "cmd notification set_dnd priority";
            else if (val == "2") cmd = "cmd notification set_dnd total";
            else if (val == "3") cmd = "cmd notification set_dnd alarms";
        }
        execute_command_bool(cmd);
        execute_command_bool("rm -f " + path);
    }

    // Apply one tweak, backing up the original it touches (so it can be restored later).
    void apply_tweak(const std::string& tag) {
        if (tag == "dab") {
            save_setting_once(".brightness", "settings get system screen_brightness_mode");
            execute_command_bool("settings put system screen_brightness_mode 0");
        } else if (tag == "dnd") {
            save_setting_once(".dnd", "settings get global zen_mode");
            execute_command_bool("cmd notification set_dnd priority");
        } else if (tag == "nolog") {
            // marker so a crash mid-session still re-enables logd on next start
            { std::ofstream m(DEFAULTS_FILE + ".nolog"); m << "1"; }
            execute_command_bool("chmod 644 " + DEFAULTS_FILE + ".nolog");
            execute_command_bool("stop logd");
        } else if (tag == "kso") {
            save_setting_once(".timeout", "settings get system screen_off_timeout");
            execute_command_bool("settings put system screen_off_timeout 300000000");
        }
    }

    void restore_tweak(const std::string& tag) {
        if (tag == "dab") {
            restore_setting(".brightness", "settings put system screen_brightness_mode ");
        } else if (tag == "dnd") {
            restore_dnd();
        } else if (tag == "nolog") {
            execute_command_bool("start logd");
            execute_command_bool("rm -f " + DEFAULTS_FILE + ".nolog");
        } else if (tag == "kso") {
            restore_setting(".timeout", "settings put system screen_off_timeout ");
        }
    }

    // Crash recovery: on startup, undo any tweak whose backup/marker is still on disk.
    void restore_saved_states() {
        bool any = false;
        if (file_exists(DEFAULTS_FILE + ".brightness")) { restore_setting(".brightness", "settings put system screen_brightness_mode "); any = true; }
        if (file_exists(DEFAULTS_FILE + ".dnd"))         { restore_dnd(); any = true; }
        if (file_exists(DEFAULTS_FILE + ".timeout"))     { restore_setting(".timeout", "settings put system screen_off_timeout "); any = true; }
        if (file_exists(DEFAULTS_FILE + ".nolog"))       { execute_command_bool("start logd"); execute_command_bool("rm -f " + DEFAULTS_FILE + ".nolog"); any = true; }
        if (any) std::cout << "✅ Recovered leftover system states" << std::endl;
    }

    std::string state_to_string(AppState state) {
        switch (state) {
            case AppState::FOREGROUND: return "✅ FOREGROUND";
            case AppState::BACKGROUND: return "🔵 BACKGROUND"; 
            default: return "❓ UNKNOWN";
        }
    }

public:
    void run_controller() {
        if (!is_ready()) {
            std::cerr << "❌ Controller not ready" << std::endl;
            return;
        }

        std::cout << "🚀 Advanced App Controller Started" << std::endl;
        std::cout << "📊 Monitoring " << monitored_packages.size() << " installed packages" << std::endl;
        std::cout << "🎛️ " << package_tweaks.size() << " packages have per-app tweaks" << std::endl;
        std::cout << "🎯 States: FOREGROUND, BACKGROUND only" << std::endl;
        std::cout << "🎛️ Per-app tweaks: dab / dnd / nolog / kso (from package tags)" << std::endl;
        std::cout << "🔄 Instant response to config changes" << std::endl;
        
        int debounce_count = 0;
        const int DEBOUNCE_THRESHOLD = 2;
        int config_wait_counter = 0;
        std::set<std::string> pending_desired;
        
        while (running.load()) {
            try {
                if (!config_file_exists.load()) {
                    config_wait_counter++;
                    if (config_wait_counter % 20 == 0) {
                        std::cout << "⏳ Waiting for config file..." << std::endl;
                    }
                    
                    struct stat buffer;
                    if (stat(CONFIG_JSON.c_str(), &buffer) == 0) {
                        std::cout << "📁 Config file detected - loading..." << std::endl;
                        if (load_config()) {
                            config_file_exists.store(true);
                            config_wait_counter = 0;
                            std::cout << "✅ Config loaded successfully" << std::endl;
                        }
                    }
                    
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                    continue;
                }
                
                auto active_apps = get_all_active_apps();

                log_state_changes(active_apps);

                std::set<std::string> desired = compute_desired_tweaks(active_apps);

                if (desired == applied_tweaks) {
                    debounce_count = 0;
                } else {
                    // Debounce transient flips before touching system state.
                    if (desired == pending_desired) {
                        debounce_count++;
                    } else {
                        pending_desired = desired;
                        debounce_count = 1;
                    }
                    if (debounce_count >= DEBOUNCE_THRESHOLD) {
                        if (desired.empty()) {
                            std::cout << "🏁 No tweakable app active - restoring system states" << std::endl;
                        } else {
                            std::cout << "🎯 Active tweak set:";
                            for (const auto& t : desired) std::cout << " " << t;
                            std::cout << std::endl;
                        }
                        sync_tweaks(desired);
                        debounce_count = 0;
                    }
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            } catch (const std::exception& e) {
                std::cerr << "❌ Error in main loop: " << e.what() << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }
    }
};

std::unique_ptr<AdvancedAppController> g_controller = nullptr;

void signal_handler(int signal) {
    std::cout << "\n🛑 Received signal " << signal << ", shutting down..." << std::endl;
    if (g_controller) {
        g_controller->stop();
    }
}

int main() {
    std::cout << "🎮 Starting Advanced App Controller..." << std::endl;
    
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);
    
    try {
        g_controller = std::make_unique<AdvancedAppController>();
        
        if (!g_controller->is_ready()) {
            std::cerr << "💥 Controller initialization failed" << std::endl;
            return 1;
        }
        
        g_controller->run_controller();
        
        std::cout << "✅ Shutdown complete" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "💥 Fatal error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
