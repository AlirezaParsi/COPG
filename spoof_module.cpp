#include <jni.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <dirent.h>
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
#include <sys/socket.h>

using json = nlohmann::json;

#define LOG_TAG "COPGHook"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// ─────────────────────────────────────────
// انواع تابع
// ─────────────────────────────────────────
typedef int         (*prop_get_t)    (const char*, char*);
typedef void        (*prop_read_cb_t)(void*, const char*, const char*, uint32_t);
typedef void        (*prop_read_t)   (const void*, prop_read_cb_t, void*);
typedef const void* (*prop_find_t)   (const char*);
typedef void        (*prop_read_old_t)(const void*, unsigned*, char*, char*);

static const char* TARGET_SYMBOLS[] = {
    "__system_property_get",
    "__system_property_read_callback",
    "__system_property_find",
    "__system_property_read",
    nullptr
};

// ─────────────────────────────────────────
// ساختار hook
// ─────────────────────────────────────────
struct HookEntry {
    std::string lib_name;
    std::string symbol;
    uintptr_t   got_offset = 0;
    uintptr_t   load_base  = 0;
    void*       original   = nullptr;
    bool        hooked     = false;
};

struct ProcessContext {
    std::unordered_map<std::string, std::string> props;
    std::vector<HookEntry> hooks;
    std::atomic<bool> ready{false};
    std::string package_name;

    prop_get_t      orig_get      = nullptr;
    prop_read_t     orig_read_cb  = nullptr;
    prop_find_t     orig_find     = nullptr;
    prop_read_old_t orig_read_old = nullptr;
};

static ProcessContext*  g_ctx         = nullptr;
static prop_read_cb_t   g_app_callback = nullptr;

// ─────────────────────────────────────────
// IPC helpers
// ─────────────────────────────────────────
static bool writeStr(int fd, const std::string& s) {
    uint32_t len = s.size();
    if (write(fd, &len, 4) != 4) return false;
    if (len > 0 && (size_t)write(fd, s.data(), len) != len)
        return false;
    return true;
}

static bool readStr(int fd, std::string& s) {
    uint32_t len = 0;
    if (read(fd, &len, 4) != 4) return false;
    if (len == 0) { s.clear(); return true; }
    s.resize(len);
    return (size_t)read(fd, s.data(), len) == len;
}

static bool writeU64(int fd, uint64_t v) {
    return write(fd, &v, 8) == 8;
}

static bool readU64(int fd, uint64_t& v) {
    return read(fd, &v, 8) == 8;
}

// ─────────────────────────────────────────
// hook functions
// ─────────────────────────────────────────
static int hooked_prop_get(const char* name, char* value) {
    if (!g_ctx || !g_ctx->orig_get) return 0;

    if (name && g_ctx->ready.load()) {
        auto it = g_ctx->props.find(name);
        if (it != g_ctx->props.end()) {
            strncpy(value, it->second.c_str(), 91);
            value[91] = '\0';
            LOGI("prop_get: %s -> %s", name, value);
            return (int)strlen(value);
        }
    }
    return g_ctx->orig_get(name, value);
}

static void hooked_read_cb(void* cookie, const char* name,
                            const char* value, uint32_t serial) {
    if (!g_app_callback) return;

    if (g_ctx && name && g_ctx->ready.load()) {
        auto it = g_ctx->props.find(name);
        if (it != g_ctx->props.end()) {
            LOGI("read_cb: %s -> %s", name, it->second.c_str());
            g_app_callback(cookie, name, it->second.c_str(), serial);
            return;
        }
    }
    g_app_callback(cookie, name, value, serial);
}

static void hooked_prop_read(const void* pi,
                              prop_read_cb_t cb, void* cookie) {
    if (!g_ctx || !g_ctx->orig_read_cb) return;
    g_app_callback = cb;
    g_ctx->orig_read_cb(pi, hooked_read_cb, cookie);
}

static const void* hooked_prop_find(const char* name) {
    if (!g_ctx || !g_ctx->orig_find) return nullptr;
    return g_ctx->orig_find(name);
}

static void hooked_prop_read_old(const void* pi, unsigned* serial,
                                  char* name, char* value) {
    if (!g_ctx || !g_ctx->orig_read_old) return;
    g_ctx->orig_read_old(pi, serial, name, value);

    if (name && value && g_ctx->ready.load()) {
        auto it = g_ctx->props.find(name);
        if (it != g_ctx->props.end()) {
            strncpy(value, it->second.c_str(), 91);
            value[91] = '\0';
            LOGI("read_old: %s -> %s", name, value);
        }
    }
}

// ─────────────────────────────────────────
// ELF parser (در companion اجرا میشه)
// ─────────────────────────────────────────
static uintptr_t findFromSections(FILE* f, const Elf64_Ehdr& ehdr,
                                   const char* symbol) {
    std::vector<Elf64_Shdr> shdrs(ehdr.e_shnum);
    fseek(f, ehdr.e_shoff, SEEK_SET);
    if (fread(shdrs.data(), sizeof(Elf64_Shdr), ehdr.e_shnum, f)
        != (size_t)ehdr.e_shnum) return 0;

    Elf64_Shdr& ss = shdrs[ehdr.e_shstrndx];
    std::vector<char> shstrtab(ss.sh_size);
    fseek(f, ss.sh_offset, SEEK_SET);
    fread(shstrtab.data(), 1, ss.sh_size, f);

    Elf64_Shdr *rela_plt=nullptr, *rela_dyn=nullptr,
               *dynsym_h=nullptr, *dynstr_h=nullptr;
    for (auto& s : shdrs) {
        const char* n = shstrtab.data() + s.sh_name;
        if      (!strcmp(n, ".rela.plt")) rela_plt = &s;
        else if (!strcmp(n, ".rela.dyn")) rela_dyn = &s;
        else if (!strcmp(n, ".dynsym"))   dynsym_h = &s;
        else if (!strcmp(n, ".dynstr"))   dynstr_h = &s;
    }
    if (!dynsym_h || !dynstr_h) return 0;

    std::vector<char> dynstr(dynstr_h->sh_size);
    fseek(f, dynstr_h->sh_offset, SEEK_SET);
    fread(dynstr.data(), 1, dynstr_h->sh_size, f);

    size_t sym_count = dynsym_h->sh_size / sizeof(Elf64_Sym);
    std::vector<Elf64_Sym> syms(sym_count);
    fseek(f, dynsym_h->sh_offset, SEEK_SET);
    fread(syms.data(), sizeof(Elf64_Sym), sym_count, f);

    auto search = [&](Elf64_Shdr* rh) -> uintptr_t {
        if (!rh) return 0;
        size_t cnt = rh->sh_size / sizeof(Elf64_Rela);
        std::vector<Elf64_Rela> rs(cnt);
        fseek(f, rh->sh_offset, SEEK_SET);
        fread(rs.data(), sizeof(Elf64_Rela), cnt, f);
        for (auto& r : rs) {
            uint32_t idx = ELF64_R_SYM(r.r_info);
            if (idx >= sym_count) continue;
            if (!strcmp(dynstr.data()+syms[idx].st_name, symbol))
                return (uintptr_t)r.r_offset;
        }
        return 0;
    };

    uintptr_t off = search(rela_plt);
    if (!off) off = search(rela_dyn);
    return off;
}

static uintptr_t findFromDynamic(FILE* f, const Elf64_Ehdr& ehdr,
                                  const char* symbol) {
    std::vector<Elf64_Phdr> phdrs(ehdr.e_phnum);
    fseek(f, ehdr.e_phoff, SEEK_SET);
    fread(phdrs.data(), sizeof(Elf64_Phdr), ehdr.e_phnum, f);

    Elf64_Phdr* dyn_ph = nullptr;
    for (auto& ph : phdrs)
        if (ph.p_type == PT_DYNAMIC) { dyn_ph = &ph; break; }
    if (!dyn_ph) return 0;

    size_t dyn_cnt = dyn_ph->p_filesz / sizeof(Elf64_Dyn);
    std::vector<Elf64_Dyn> dyns(dyn_cnt);
    fseek(f, dyn_ph->p_offset, SEEK_SET);
    fread(dyns.data(), sizeof(Elf64_Dyn), dyn_cnt, f);

    uintptr_t strtab_va=0, symtab_va=0, rela_va=0, rela_sz=0;
    uintptr_t syment = sizeof(Elf64_Sym);
    for (auto& d : dyns) {
        switch(d.d_tag) {
            case DT_STRTAB:    strtab_va = d.d_un.d_ptr; break;
            case DT_SYMTAB:    symtab_va = d.d_un.d_ptr; break;
            case DT_JMPREL:    rela_va   = d.d_un.d_ptr; break;
            case DT_PLTRELSZ:  rela_sz   = d.d_un.d_val; break;
            case DT_SYMENT:    syment    = d.d_un.d_val; break;
        }
    }
    if (!strtab_va || !symtab_va || !rela_va) return 0;

    auto va2off = [&](uintptr_t va) -> uintptr_t {
        for (auto& ph : phdrs) {
            if (ph.p_type != PT_LOAD) continue;
            if (va >= ph.p_vaddr && va < ph.p_vaddr + ph.p_filesz)
                return va - ph.p_vaddr + ph.p_offset;
        }
        return 0;
    };

    uintptr_t strtab_off = va2off(strtab_va);
    uintptr_t symtab_off = va2off(symtab_va);
    uintptr_t rela_off   = va2off(rela_va);
    if (!strtab_off || !symtab_off || !rela_off) return 0;

    size_t rela_cnt = rela_sz / sizeof(Elf64_Rela);
    std::vector<Elf64_Rela> relas(rela_cnt);
    fseek(f, rela_off, SEEK_SET);
    fread(relas.data(), sizeof(Elf64_Rela), rela_cnt, f);

    std::vector<char> strtab(65536);
    fseek(f, strtab_off, SEEK_SET);
    fread(strtab.data(), 1, strtab.size(), f);

    for (auto& r : relas) {
        uint32_t sym_idx = ELF64_R_SYM(r.r_info);
        Elf64_Sym sym;
        fseek(f, symtab_off + sym_idx * syment, SEEK_SET);
        fread(&sym, sizeof(sym), 1, f);
        if (sym.st_name >= strtab.size()) continue;
        if (!strcmp(strtab.data() + sym.st_name, symbol))
            return (uintptr_t)r.r_offset;
    }
    return 0;
}

static uintptr_t findGotOffset(const char* path, const char* sym) {
    FILE* f = fopen(path, "rb");
    if (!f) return 0;

    Elf64_Ehdr ehdr;
    if (fread(&ehdr, sizeof(ehdr), 1, f) != 1
        || memcmp(ehdr.e_ident, ELFMAG, SELFMAG) != 0) {
        fclose(f); return 0;
    }

    uintptr_t off = 0;
    if (ehdr.e_shoff) off = findFromSections(f, ehdr, sym);
    if (!off)         off = findFromDynamic(f, ehdr, sym);
    fclose(f);
    return off;
}

// ─────────────────────────────────────────
// companion: اجرا با root
// ─────────────────────────────────────────
static void companion(int fd) {
    LOGI("[companion] started");

    // دریافت package name
    std::string pkg;
    if (!readStr(fd, pkg)) {
        LOGE("[companion] read pkg failed");
        return;
    }
    LOGI("[companion] scanning for: %s", pkg.c_str());

    // پیدا کردن lib dir
    std::string lib_dir;
    const char* base = "/data/app";
    DIR* d1 = opendir(base);
    if (d1) {
        struct dirent* e1;
        while ((e1 = readdir(d1)) && lib_dir.empty()) {
            if (e1->d_name[0] == '.') continue;
            std::string tier1 = std::string(base) + "/" + e1->d_name;
            DIR* d2 = opendir(tier1.c_str());
            if (!d2) continue;
            struct dirent* e2;
            while ((e2 = readdir(d2)) && lib_dir.empty()) {
                if (e2->d_name[0] == '.') continue;
                if (strncmp(e2->d_name, pkg.c_str(), pkg.size()) != 0)
                    continue;
                if (e2->d_name[pkg.size()] != '-') continue;
                std::string candidate = tier1 + "/"
                    + e2->d_name + "/lib/arm64";
                struct stat st;
                if (stat(candidate.c_str(), &st) == 0
                    && S_ISDIR(st.st_mode))
                    lib_dir = candidate;
            }
            closedir(d2);
        }
        closedir(d1);
    }

    if (lib_dir.empty()) {
        LOGE("[companion] lib dir not found for %s", pkg.c_str());
        uint32_t zero = 0;
        write(fd, &zero, 4);
        return;
    }
    LOGI("[companion] lib dir: %s", lib_dir.c_str());

    // اسکن .so ها و ELF parsing
    // فرمت ارسال:
    //   uint32_t count
    //   برای هر entry:
    //     string lib_name
    //     string symbol
    //     uint64_t got_offset

    struct ScanResult {
        std::string lib_name;
        std::string symbol;
        uintptr_t   got_offset;
    };
    std::vector<ScanResult> results;

    DIR* d = opendir(lib_dir.c_str());
    if (d) {
        struct dirent* e;
        while ((e = readdir(d))) {
            size_t len = strlen(e->d_name);
            if (len < 4) continue;
            if (strcmp(e->d_name + len - 3, ".so") != 0) continue;

            std::string path = lib_dir + "/" + e->d_name;
            for (int i = 0; TARGET_SYMBOLS[i]; i++) {
                uintptr_t off = findGotOffset(
                    path.c_str(), TARGET_SYMBOLS[i]);
                if (!off) continue;

                LOGI("[companion] found %s in %s @ 0x%lx",
                     TARGET_SYMBOLS[i], e->d_name, off);
                results.push_back({e->d_name,
                                   TARGET_SYMBOLS[i], off});
            }
        }
        closedir(d);
    }

    // ارسال نتایج
    uint32_t count = (uint32_t)results.size();
    write(fd, &count, 4);

    for (auto& r : results) {
        writeStr(fd, r.lib_name);
        writeStr(fd, r.symbol);
        writeU64(fd, (uint64_t)r.got_offset);
    }

    LOGI("[companion] sent %u results", count);
}

// ─────────────────────────────────────────
// load base از maps
// ─────────────────────────────────────────
static uintptr_t getLoadBase(const char* lib_name) {
    FILE* maps = fopen("/proc/self/maps", "r");
    if (!maps) return 0;
    char line[512];
    uintptr_t base = 0;
    while (fgets(line, sizeof(line), maps)) {
        if (!strstr(line, lib_name)) continue;
        if (!strstr(line, "r--p")) continue;
        uintptr_t start, end, offset;
        char perms[8];
        sscanf(line, "%lx-%lx %s %lx", &start, &end, perms, &offset);
        if (offset == 0) { base = start; break; }
    }
    fclose(maps);
    return base;
}

// ─────────────────────────────────────────
// اعمال hook
// ─────────────────────────────────────────
static bool applyHook(HookEntry& e) {
    uintptr_t got = e.load_base + e.got_offset;
    e.original = *(void**)got;
    if (!e.original) return false;

    void* hook = nullptr;
    if      (e.symbol == "__system_property_get")
        hook = (void*)hooked_prop_get;
    else if (e.symbol == "__system_property_read_callback")
        hook = (void*)hooked_prop_read;
    else if (e.symbol == "__system_property_find")
        hook = (void*)hooked_prop_find;
    else if (e.symbol == "__system_property_read")
        hook = (void*)hooked_prop_read_old;
    if (!hook) return false;

    size_t ps = getpagesize();
    uintptr_t pg = got & ~(ps - 1);
    if (mprotect((void*)pg, ps, PROT_READ|PROT_WRITE) != 0)
        return false;

    *(void**)got = hook;
    mprotect((void*)pg, ps, PROT_READ);

    e.hooked = (*(void**)got == hook);
    if (e.hooked)
        LOGI("Hooked %s in %s", e.symbol.c_str(), e.lib_name.c_str());
    return e.hooked;
}

// ─────────────────────────────────────────
// ماژول
// ─────────────────────────────────────────
class COPGGotHook : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api* api, JNIEnv* env) override {
        this->api = api;
        this->env = env;
        loadConfig();
    }

    void preAppSpecialize(zygisk::AppSpecializeArgs* args) override {
        if (!args || !args->nice_name) {
            api->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
            return;
        }

        const char* raw = env->GetStringUTFChars(
            args->nice_name, nullptr);
        if (!raw) {
            api->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
            return;
        }
        std::string pkg = raw;
        env->ReleaseStringUTFChars(args->nice_name, raw);

        auto it = package_props.find(pkg);
        if (it == package_props.end()) {
            api->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
            return;
        }

        LOGI("Target: %s", pkg.c_str());

        // ─ companion: scan و ELF parse ─
        int cfd = api->connectCompanion();
        if (cfd < 0) {
            LOGE("connectCompanion failed");
            api->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
            return;
        }

        // ارسال package name
        writeStr(cfd, pkg);

        // دریافت نتایج
        uint32_t count = 0;
        if (read(cfd, &count, 4) != 4 || count == 0) {
            LOGE("No hooks found by companion");
            close(cfd);
            api->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
            return;
        }

        for (uint32_t i = 0; i < count; i++) {
            HookEntry e;
            uint64_t off = 0;
            if (!readStr(cfd, e.lib_name)) break;
            if (!readStr(cfd, e.symbol))   break;
            if (!readU64(cfd, off))         break;
            e.got_offset = (uintptr_t)off;
            precomputed.push_back(e);
            LOGI("Received: %s in %s @ 0x%lx",
                 e.symbol.c_str(), e.lib_name.c_str(), e.got_offset);
        }
        close(cfd);

        LOGI("Got %zu hook targets from companion", precomputed.size());

        current_pkg   = pkg;
        current_props = it->second;
        needs_hook    = true;
        // DLCLOSE نزن
    }

    void postAppSpecialize(
            const zygisk::AppSpecializeArgs* args) override {
        if (!needs_hook) {
            api->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
            return;
        }

        g_ctx = new ProcessContext();
        g_ctx->props        = current_props;
        g_ctx->package_name = current_pkg;

        std::vector<HookEntry> to_hook = precomputed;

        std::thread([to_hook]() {
            LOGI("Hook thread: %zu targets", to_hook.size());

            for (int w = 0; w < 30; w++) {
                bool any = false;
                for (auto& h : to_hook)
                    if (getLoadBase(h.lib_name.c_str())) { any=true; break; }
                if (any) { sleep(1); break; }
                sleep(1);
            }

            for (auto h : to_hook) {
                h.load_base = getLoadBase(h.lib_name.c_str());
                if (!h.load_base) continue;

                if (applyHook(h)) {
                    if      (h.symbol == "__system_property_get")
                        g_ctx->orig_get      = (prop_get_t)h.original;
                    else if (h.symbol == "__system_property_read_callback")
                        g_ctx->orig_read_cb  = (prop_read_t)h.original;
                    else if (h.symbol == "__system_property_find")
                        g_ctx->orig_find     = (prop_find_t)h.original;
                    else if (h.symbol == "__system_property_read")
                        g_ctx->orig_read_old = (prop_read_old_t)h.original;
                    g_ctx->hooks.push_back(h);
                }
            }

            g_ctx->ready.store(true);
            LOGI("Done: %zu hooks for %s",
                 g_ctx->hooks.size(),
                 g_ctx->package_name.c_str());
        }).detach();
    }

private:
    zygisk::Api* api;
    JNIEnv*      env;
    bool         needs_hook = false;
    std::string  current_pkg;
    std::vector<HookEntry> precomputed;
    std::unordered_map<std::string, std::string> current_props;
    std::unordered_map<std::string,
        std::unordered_map<std::string,std::string>> package_props;

    void loadConfig() {
        std::ifstream f("/data/adb/modules/COPG/COPG.json");
        if (!f.is_open()) return;
        try {
            json cfg = json::parse(f);
            if (!cfg.contains("got_hooks")) return;
            for (auto& [pkg, data] : cfg["got_hooks"].items()) {
                if (!data.contains("props")) continue;
                std::unordered_map<std::string,std::string> props;
                for (auto& [k,v] : data["props"].items())
                    props[k] = v.get<std::string>();
                package_props[pkg] = props;
                LOGI("Config: %s", pkg.c_str());
            }
        } catch(...) { LOGE("Config error"); }
    }
};

REGISTER_ZYGISK_MODULE(COPGGotHook)
REGISTER_ZYGISK_COMPANION(companion)
