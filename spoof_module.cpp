#include <jni.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <thread>
#include <atomic>
#include <string>
#include <vector>
#include <unordered_map>
#include <android/log.h>
#include <elf.h>
#include <zygisk.hpp>
#include <nlohmann/json.hpp>
#include <fstream>

using json = nlohmann::json;

#define LOG_TAG "COPGHook"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

struct GotHookInfo {
    std::string lib_name;
    std::string symbol;
    std::unordered_map<std::string, std::string> props;
};

typedef int (*prop_get_t)(const char*, char*);

struct HookContext {
    std::atomic<prop_get_t> original{nullptr};
    std::atomic<bool> ready{false};
    std::unordered_map<std::string, std::string> props;
};

static HookContext* g_ctx = nullptr;

static int hooked_prop_get(const char* name, char* value) {
    if (!g_ctx) return 0;

    prop_get_t orig = g_ctx->original.load();
    if (!orig) return 0;

    if (name && g_ctx->ready.load()) {
        auto it = g_ctx->props.find(name);
        if (it != g_ctx->props.end()) {
            strncpy(value, it->second.c_str(), 92);
            value[91] = '\0';
            LOGI("Hooked: %s -> %s", name, value);
            return strlen(value);
        }
    }

    return orig(name, value);
}

static uintptr_t findGotOffsetFromElf(const char* lib_path, const char* symbol) {
    FILE* f = fopen(lib_path, "rb");
    if (!f) {
        LOGE("Cannot open ELF: %s", lib_path);
        return 0;
    }

    Elf64_Ehdr ehdr;
    if (fread(&ehdr, sizeof(ehdr), 1, f) != 1) {
        fclose(f);
        return 0;
    }

    if (memcmp(ehdr.e_ident, ELFMAG, SELFMAG) != 0) {
        LOGE("Not an ELF file");
        fclose(f);
        return 0;
    }

    if (ehdr.e_shoff == 0 || ehdr.e_shentsize == 0) {
        LOGE("No section headers");
        fclose(f);
        return 0;
    }

    std::vector<Elf64_Shdr> shdrs(ehdr.e_shnum);
    fseek(f, ehdr.e_shoff, SEEK_SET);
    if (fread(shdrs.data(), sizeof(Elf64_Shdr), ehdr.e_shnum, f) != ehdr.e_shnum) {
        fclose(f);
        return 0;
    }

    Elf64_Shdr& shstrtab = shdrs[ehdr.e_shstrndx];
    std::vector<char> shstrtab_data(shstrtab.sh_size);
    fseek(f, shstrtab.sh_offset, SEEK_SET);
    fread(shstrtab_data.data(), 1, shstrtab.sh_size, f);

    Elf64_Shdr* rela_plt = nullptr;
    Elf64_Shdr* dynsym = nullptr;
    Elf64_Shdr* dynstr = nullptr;

    for (auto& shdr : shdrs) {
        const char* name_str = shstrtab_data.data() + shdr.sh_name;
        if (strcmp(name_str, ".rela.plt") == 0) rela_plt = &shdr;
        else if (strcmp(name_str, ".dynsym") == 0) dynsym = &shdr;
        else if (strcmp(name_str, ".dynstr") == 0) dynstr = &shdr;
    }

    if (!rela_plt || !dynsym || !dynstr) {
        LOGE("Missing sections");
        fclose(f);
        return 0;
    }

    std::vector<char> dynstr_data(dynstr->sh_size);
    fseek(f, dynstr->sh_offset, SEEK_SET);
    fread(dynstr_data.data(), 1, dynstr->sh_size, f);

    size_t sym_count = dynsym->sh_size / sizeof(Elf64_Sym);
    std::vector<Elf64_Sym> syms(sym_count);
    fseek(f, dynsym->sh_offset, SEEK_SET);
    fread(syms.data(), sizeof(Elf64_Sym), sym_count, f);

    size_t rela_count = rela_plt->sh_size / sizeof(Elf64_Rela);
    std::vector<Elf64_Rela> relas(rela_count);
    fseek(f, rela_plt->sh_offset, SEEK_SET);
    fread(relas.data(), sizeof(Elf64_Rela), rela_count, f);

    fclose(f);

    for (auto& rela : relas) {
        uint32_t sym_idx = ELF64_R_SYM(rela.r_info);
        if (sym_idx >= sym_count) continue;

        const char* sym_name = dynstr_data.data() + syms[sym_idx].st_name;
        if (strcmp(sym_name, symbol) == 0) {
            LOGI("Found %s at GOT offset: 0x%lx", symbol, (uintptr_t)rela.r_offset);
            return (uintptr_t)rela.r_offset;
        }
    }

    LOGE("Symbol %s not found in ELF", symbol);
    return 0;
}

static std::string findLibPath(const char* lib_name) {
    FILE* maps = fopen("/proc/self/maps", "r");
    if (!maps) return "";

    char line[512];
    std::string result;

    while (fgets(line, sizeof(line), maps)) {
        if (strstr(line, lib_name)) {
            char* path_start = strrchr(line, ' ');
            if (path_start) {
                path_start++;
                size_t len = strlen(path_start);
                if (len > 0 && path_start[len-1] == '\n')
                    path_start[len-1] = '\0';
                result = path_start;
                break;
            }
        }
    }

    fclose(maps);
    return result;
}

static bool findLoadBase(const char* lib_name, uintptr_t& load_base) {
    FILE* maps = fopen("/proc/self/maps", "r");
    if (!maps) return false;

    char line[512];
    bool found = false;

    while (fgets(line, sizeof(line), maps)) {
        if (!strstr(line, lib_name)) continue;
        if (!strstr(line, "r--p")) continue;

        uintptr_t start, end;
        char perms[8];
        uintptr_t file_offset;
        sscanf(line, "%lx-%lx %s %lx", &start, &end, perms, &file_offset);

        if (file_offset == 0) {
            load_base = start;
            LOGI("Load base of %s: 0x%lx", lib_name, load_base);
            found = true;
            break;
        }
    }

    fclose(maps);
    return found;
}

static bool applyGotHook(const GotHookInfo& hook_info, HookContext* ctx) {
    const char* lib_name = hook_info.lib_name.c_str();
    const char* symbol = hook_info.symbol.c_str();

    std::string lib_path = findLibPath(lib_name);
    if (lib_path.empty()) {
        LOGE("Library %s not found in maps", lib_name);
        return false;
    }
    LOGI("Library path: %s", lib_path.c_str());

    uintptr_t got_offset = findGotOffsetFromElf(lib_path.c_str(), symbol);
    if (got_offset == 0) {
        LOGE("GOT offset not found for %s", symbol);
        return false;
    }

    uintptr_t load_base = 0;
    if (!findLoadBase(lib_name, load_base)) {
        LOGE("Load base not found for %s", lib_name);
        return false;
    }

    uintptr_t got_addr = load_base + got_offset;
    LOGI("GOT address: 0x%lx", got_addr);

    prop_get_t orig = *(prop_get_t*)got_addr;
    if (!orig) {
        LOGE("Original function is null!");
        return false;
    }
    ctx->original.store(orig);
    LOGI("Original %s: %p", symbol, orig);

    size_t page_size = getpagesize();
    uintptr_t page = got_addr & ~(page_size - 1);
    if (mprotect((void*)page, page_size, PROT_READ | PROT_WRITE) != 0) {
        LOGE("mprotect failed: %s", strerror(errno));
        return false;
    }

    *(prop_get_t*)got_addr = hooked_prop_get;

    mprotect((void*)page, page_size, PROT_READ);

    if (*(prop_get_t*)got_addr != hooked_prop_get) {
        LOGE("Hook verification failed!");
        return false;
    }

    ctx->ready.store(true);
    LOGI("GOT hook applied for %s in %s", symbol, lib_name);
    return true;
}

class COPGModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api* api, JNIEnv* env) override {
        this->api = api;
        this->env = env;
        LOGI("COPG module loaded");
        loadConfig();
    }

    void preAppSpecialize(zygisk::AppSpecializeArgs* args) override {
        if (!args || !args->nice_name) {
            api->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
            return;
        }

        const char* pkg = env->GetStringUTFChars(args->nice_name, nullptr);
        if (!pkg) {
            api->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
            return;
        }

        std::string package_name = pkg;
        env->ReleaseStringUTFChars(args->nice_name, pkg);

        LOGI("Processing: %s", package_name.c_str());

        auto it = got_hooks.find(package_name);
        if (it != got_hooks.end()) {
            current_hook_info = it->second;
            needs_got_hook = true;
            LOGI("%s: needs GOT hook", package_name.c_str());
            return;
        }

        api->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs* args) override {
        if (!needs_got_hook) {
            api->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
            return;
        }

        g_ctx = new HookContext();
        g_ctx->props = current_hook_info.props;

        GotHookInfo hook_info = current_hook_info;

        std::thread([hook_info]() {
            LOGI("GOT hook thread started for %s", hook_info.lib_name.c_str());

            for (int i = 0; i < 60; i++) {
                uintptr_t base = 0;
                if (findLoadBase(hook_info.lib_name.c_str(), base)) {
                    LOGI("Library found after %d seconds", i);
                    sleep(1);

                    if (applyGotHook(hook_info, g_ctx)) {
                        LOGI("GOT hook successful!");

                        char test[256] = {0};
                        hooked_prop_get("ro.product.model", test);
                        LOGI("Test: ro.product.model = %s", test);
                    } else {
                        LOGE("GOT hook failed!");
                    }
                    return;
                }
                sleep(1);
            }
            LOGE("Library never loaded");
        }).detach();
    }

private:
    zygisk::Api* api;
    JNIEnv* env;
    bool needs_got_hook = false;
    GotHookInfo current_hook_info;
    std::unordered_map<std::string, GotHookInfo> got_hooks;

    void loadConfig() {
        const char* config_path = "/data/adb/modules/COPG/COPG.json";
        std::ifstream f(config_path);
        if (!f.is_open()) return;

        try {
            json config = json::parse(f);

            if (config.contains("got_hooks")) {
                for (auto& [pkg, hook_data] : config["got_hooks"].items()) {
                    GotHookInfo info;
                    info.lib_name = hook_data.value("lib", "");
                    info.symbol = hook_data.value("symbol", "__system_property_get");

                    if (hook_data.contains("props")) {
                        for (auto& [k, v] : hook_data["props"].items()) {
                            info.props[k] = v.get<std::string>();
                        }
                    }

                    if (!info.lib_name.empty()) {
                        got_hooks[pkg] = info;
                        LOGI("Loaded GOT hook for %s -> %s",
                             pkg.c_str(), info.lib_name.c_str());
                    }
                }
            }
        } catch (...) {
            LOGE("Config parse error");
        }
    }
};

REGISTER_ZYGISK_MODULE(COPGModule)
