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
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <dirent.h>
#include <elf.h>
#include <thread>
#include <atomic>

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
#define GOT_LOG(...) LOGI("[GOT] " __VA_ARGS__)

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
// GOT Hook Types
// ─────────────────────────────────────────
typedef int (*prop_get_t)(const char*, char*);
typedef void (*prop_read_cb_t)(void*, const char*, const char*, uint32_t);
typedef void (*prop_read_t)(const void*, prop_read_cb_t, void*);
typedef const void* (*prop_find_t)(const char*);
typedef void (*prop_read_old_t)(const void*, unsigned*, char*, char*);

static const char* TARGET_SYMBOLS[] = {
    "__system_property_get",
    "__system_property_read_callback",
    "__system_property_find",
    "__system_property_read",
    nullptr
};

static const char* ABI_DIRS[] = {
    "arm64", "arm64-v8a", "arm", "armeabi-v7a", "x86_64", "x86", nullptr
};

struct GotHookEntry {
    std::string lib_name;
    std::string symbol;
    uintptr_t got_offset = 0;
    uintptr_t load_base = 0;
    void* original = nullptr;
    bool hooked = false;
};

struct GotProcessContext {
    std::unordered_map<std::string, std::string> props;
    std::vector<GotHookEntry> hooks;
    std::atomic<bool> ready{false};
    std::string package_name;

    prop_get_t orig_get = nullptr;
    prop_read_t orig_read_cb = nullptr;
    prop_find_t orig_find = nullptr;
    prop_read_old_t orig_read_old = nullptr;
};

static GotProcessContext* g_got_ctx = nullptr;
static prop_read_cb_t g_app_callback = nullptr;

// ─────────────────────────────────────────
// GOT Hook Functions
// ─────────────────────────────────────────
static int hooked_prop_get(const char* name, char* value) {
    if (!g_got_ctx || !g_got_ctx->orig_get) return 0;
    if (name && g_got_ctx->ready.load()) {
        auto it = g_got_ctx->props.find(name);
        if (it != g_got_ctx->props.end()) {
            strncpy(value, it->second.c_str(), 91);
            value[91] = '\0';
            return (int)strlen(value);
        }
    }
    return g_got_ctx->orig_get(name, value);
}

static void hooked_read_cb(void* cookie, const char* name, const char* value, uint32_t serial) {
    if (!g_app_callback) return;
    if (g_got_ctx && name && g_got_ctx->ready.load()) {
        auto it = g_got_ctx->props.find(name);
        if (it != g_got_ctx->props.end()) {
            g_app_callback(cookie, name, it->second.c_str(), serial);
            return;
        }
    }
    g_app_callback(cookie, name, value, serial);
}

static void hooked_prop_read(const void* pi, prop_read_cb_t cb, void* cookie) {
    if (!g_got_ctx || !g_got_ctx->orig_read_cb) return;
    g_app_callback = cb;
    g_got_ctx->orig_read_cb(pi, hooked_read_cb, cookie);
}

static const void* hooked_prop_find(const char* name) {
    if (!g_got_ctx || !g_got_ctx->orig_find) return nullptr;
    return g_got_ctx->orig_find(name);
}

static void hooked_prop_read_old(const void* pi, unsigned* serial, char* name, char* value) {
    if (!g_got_ctx || !g_got_ctx->orig_read_old) return;
    g_got_ctx->orig_read_old(pi, serial, name, value);
    if (name && value && g_got_ctx->ready.load()) {
        auto it = g_got_ctx->props.find(name);
        if (it != g_got_ctx->props.end()) {
            strncpy(value, it->second.c_str(), 91);
            value[91] = '\0';
        }
    }
}

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
    std::unordered_map<std::string, std::string> prop_overrides;
};

struct PackageFlags {
    bool needs_device_spoof = false;
    bool needs_cpu_mount = false;    // with_cpu
    bool needs_cpu_unmount = false;  // blocked
    bool needs_got_hook = false;     // got
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
// ELF Parser (64-bit)
// ─────────────────────────────────────────
#if IS_64BIT
static uintptr_t findFromSections64(FILE* f, const Elf64_Ehdr& ehdr, const char* symbol) {
    std::vector<Elf64_Shdr> shdrs(ehdr.e_shnum);
    fseek(f, ehdr.e_shoff, SEEK_SET);
    if (fread(shdrs.data(), sizeof(Elf64_Shdr), ehdr.e_shnum, f) != (size_t)ehdr.e_shnum) return 0;

    Elf64_Shdr& ss = shdrs[ehdr.e_shstrndx];
    std::vector<char> shstrtab(ss.sh_size);
    fseek(f, ss.sh_offset, SEEK_SET);
    fread(shstrtab.data(), 1, ss.sh_size, f);

    Elf64_Shdr *rela_plt = nullptr, *rela_dyn = nullptr, *dynsym_h = nullptr, *dynstr_h = nullptr;
    for (auto& s : shdrs) {
        const char* n = shstrtab.data() + s.sh_name;
        if (!strcmp(n, ".rela.plt")) rela_plt = &s;
        else if (!strcmp(n, ".rela.dyn")) rela_dyn = &s;
        else if (!strcmp(n, ".dynsym")) dynsym_h = &s;
        else if (!strcmp(n, ".dynstr")) dynstr_h = &s;
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
            if (!strcmp(dynstr.data() + syms[idx].st_name, symbol))
                return (uintptr_t)r.r_offset;
        }
        return 0;
    };

    uintptr_t off = search(rela_plt);
    if (!off) off = search(rela_dyn);
    return off;
}

static uintptr_t findFromDynamic64(FILE* f, const Elf64_Ehdr& ehdr, const char* symbol) {
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

    uintptr_t strtab_va = 0, symtab_va = 0;
    uintptr_t plt_va = 0, plt_sz = 0, rela_va = 0, rela_sz = 0;
    uintptr_t syment = sizeof(Elf64_Sym);

    for (auto& d : dyns) {
        switch (d.d_tag) {
            case DT_STRTAB: strtab_va = d.d_un.d_ptr; break;
            case DT_SYMTAB: symtab_va = d.d_un.d_ptr; break;
            case DT_SYMENT: syment = d.d_un.d_val; break;
            case DT_JMPREL: plt_va = d.d_un.d_ptr; break;
            case DT_PLTRELSZ: plt_sz = d.d_un.d_val; break;
            case DT_RELA: rela_va = d.d_un.d_ptr; break;
            case DT_RELASZ: rela_sz = d.d_un.d_val; break;
        }
    }
    if (!strtab_va || !symtab_va) return 0;
    if (!plt_va && !rela_va) return 0;

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
    if (!strtab_off || !symtab_off) return 0;

    std::vector<char> strtab(65536);
    fseek(f, strtab_off, SEEK_SET);
    fread(strtab.data(), 1, strtab.size(), f);

    auto searchRela = [&](uintptr_t va, uintptr_t sz) -> uintptr_t {
        if (!va || !sz) return 0;
        uintptr_t off = va2off(va);
        if (!off) return 0;

        size_t cnt = sz / sizeof(Elf64_Rela);
        std::vector<Elf64_Rela> relas(cnt);
        fseek(f, off, SEEK_SET);
        fread(relas.data(), sizeof(Elf64_Rela), cnt, f);

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
    };

    uintptr_t off = searchRela(plt_va, plt_sz);
    if (!off) off = searchRela(rela_va, rela_sz);
    return off;
}
#endif

// ─────────────────────────────────────────
// ELF Parser (32-bit)
// ─────────────────────────────────────────
#if IS_32BIT
static uintptr_t findFromSections32(FILE* f, const Elf32_Ehdr& ehdr, const char* symbol) {
    std::vector<Elf32_Shdr> shdrs(ehdr.e_shnum);
    fseek(f, ehdr.e_shoff, SEEK_SET);
    if (fread(shdrs.data(), sizeof(Elf32_Shdr), ehdr.e_shnum, f) != (size_t)ehdr.e_shnum) return 0;

    Elf32_Shdr& shstr_hdr = shdrs[ehdr.e_shstrndx];
    std::vector<char> shstrtab(shstr_hdr.sh_size);
    fseek(f, shstr_hdr.sh_offset, SEEK_SET);
    fread(shstrtab.data(), 1, shstr_hdr.sh_size, f);

    Elf32_Shdr *rel_plt = nullptr, *rel_dyn = nullptr, *dynsym_h = nullptr, *dynstr_h = nullptr;
    for (auto& s : shdrs) {
        const char* name = shstrtab.data() + s.sh_name;
        if (!strcmp(name, ".rel.plt")) rel_plt = &s;
        else if (!strcmp(name, ".rel.dyn")) rel_dyn = &s;
        else if (!strcmp(name, ".dynsym")) dynsym_h = &s;
        else if (!strcmp(name, ".dynstr")) dynstr_h = &s;
    }
    if (!dynsym_h || !dynstr_h) return 0;

    std::vector<char> dynstr(dynstr_h->sh_size);
    fseek(f, dynstr_h->sh_offset, SEEK_SET);
    fread(dynstr.data(), 1, dynstr_h->sh_size, f);

    size_t sym_count = dynstr_h->sh_size / sizeof(Elf32_Sym);
    std::vector<Elf32_Sym> syms(sym_count);
    fseek(f, dynstr_h->sh_offset, SEEK_SET);
    fread(syms.data(), sizeof(Elf32_Sym), sym_count, f);

    auto searchRel = [&](Elf32_Shdr* rel_hdr) -> uintptr_t {
        if (!rel_hdr) return 0;
        size_t rel_count = rel_hdr->sh_size / sizeof(Elf32_Rel);
        std::vector<Elf32_Rel> rels(rel_count);
        fseek(f, rel_hdr->sh_offset, SEEK_SET);
        fread(rels.data(), sizeof(Elf32_Rel), rel_count, f);

        for (auto& rel : rels) {
            uint32_t sym_idx = ELF32_R_SYM(rel.r_info);
            if (sym_idx >= sym_count) continue;
            if (!strcmp(dynstr.data() + syms[sym_idx].st_name, symbol))
                return (uintptr_t)rel.r_offset;
        }
        return 0;
    };

    uintptr_t off = searchRel(rel_plt);
    if (!off) off = searchRel(rel_dyn);
    return off;
}

static uintptr_t findFromDynamic32(FILE* f, const Elf32_Ehdr& ehdr, const char* symbol) {
    std::vector<Elf32_Phdr> phdrs(ehdr.e_phnum);
    fseek(f, ehdr.e_phoff, SEEK_SET);
    fread(phdrs.data(), sizeof(Elf32_Phdr), ehdr.e_phnum, f);

    Elf32_Phdr* dyn_ph = nullptr;
    for (auto& ph : phdrs)
        if (ph.p_type == PT_DYNAMIC) { dyn_ph = &ph; break; }
    if (!dyn_ph) return 0;

    size_t dyn_cnt = dyn_ph->p_filesz / sizeof(Elf32_Dyn);
    std::vector<Elf32_Dyn> dyns(dyn_cnt);
    fseek(f, dyn_ph->p_offset, SEEK_SET);
    fread(dyns.data(), sizeof(Elf32_Dyn), dyn_cnt, f);

    uint32_t strtab_va = 0, symtab_va = 0;
    uint32_t rel_va = 0, rel_sz = 0;
    uint32_t jmprel_va = 0, jmprel_sz = 0;
    uint32_t syment = sizeof(Elf32_Sym);

    for (auto& d : dyns) {
        switch (d.d_tag) {
            case DT_STRTAB: strtab_va = d.d_un.d_ptr; break;
            case DT_SYMTAB: symtab_va = d.d_un.d_ptr; break;
            case DT_REL: rel_va = d.d_un.d_ptr; break;
            case DT_RELSZ: rel_sz = d.d_un.d_val; break;
            case DT_JMPREL: jmprel_va = d.d_un.d_ptr; break;
            case DT_PLTRELSZ: jmprel_sz = d.d_un.d_val; break;
            case DT_SYMENT: syment = d.d_un.d_val; break;
        }
    }
    if (!strtab_va || !symtab_va) return 0;

    auto va2off = [&](uint32_t va) -> uint32_t {
        for (auto& ph : phdrs) {
            if (ph.p_type != PT_LOAD) continue;
            if (va >= ph.p_vaddr && va < ph.p_vaddr + ph.p_filesz)
                return va - ph.p_vaddr + ph.p_offset;
        }
        return 0;
    };

    uint32_t strtab_off = va2off(strtab_va);
    uint32_t symtab_off = va2off(symtab_va);
    if (!strtab_off || !symtab_off) return 0;

    std::vector<char> strtab(65536);
    fseek(f, strtab_off, SEEK_SET);
    fread(strtab.data(), 1, strtab.size(), f);

    auto searchRel32 = [&](uint32_t va, uint32_t sz) -> uintptr_t {
        if (!va || !sz) return 0;
        uint32_t off = va2off(va);
        if (!off) return 0;

        size_t cnt = sz / sizeof(Elf32_Rel);
        std::vector<Elf32_Rel> rels(cnt);
        fseek(f, off, SEEK_SET);
        fread(rels.data(), sizeof(Elf32_Rel), cnt, f);

        for (auto& r : rels) {
            uint32_t sym_idx = ELF32_R_SYM(r.r_info);
            Elf32_Sym sym;
            fseek(f, symtab_off + sym_idx * syment, SEEK_SET);
            fread(&sym, sizeof(sym), 1, f);
            if (sym.st_name >= strtab.size()) continue;
            if (!strcmp(strtab.data() + sym.st_name, symbol))
                return (uintptr_t)r.r_offset;
        }
        return 0;
    };

    uintptr_t off = searchRel32(jmprel_va, jmprel_sz);
    if (!off) off = searchRel32(rel_va, rel_sz);
    return off;
}
#endif

static uintptr_t findGotOffset(const char* path, const char* sym) {
    FILE* f = fopen(path, "rb");
    if (!f) return 0;

    unsigned char ident[EI_NIDENT];
    if (fread(ident, 1, EI_NIDENT, f) != EI_NIDENT) {
        fclose(f);
        return 0;
    }
    rewind(f);

    uintptr_t off = 0;

#if IS_64BIT
    Elf64_Ehdr ehdr;
    if (fread(&ehdr, sizeof(ehdr), 1, f) == 1 && memcmp(ehdr.e_ident, ELFMAG, SELFMAG) == 0) {
        if (ehdr.e_shoff) off = findFromSections64(f, ehdr, sym);
        if (!off) off = findFromDynamic64(f, ehdr, sym);
    }
#elif IS_32BIT
    Elf32_Ehdr ehdr;
    if (fread(&ehdr, sizeof(ehdr), 1, f) == 1 && memcmp(ehdr.e_ident, ELFMAG, SELFMAG) == 0) {
        if (ehdr.e_shoff) off = findFromSections32(f, ehdr, sym);
        if (!off) off = findFromDynamic32(f, ehdr, sym);
    }
#endif

    fclose(f);
    return off;
}

// ─────────────────────────────────────────
// Library Directory Finding
// ─────────────────────────────────────────
static std::string findLibDir(const std::string& pkg) {
    const char* base = "/data/app";
    DIR* d1 = opendir(base);
    if (!d1) return "";

    std::string result;
    struct dirent* e1;

    while ((e1 = readdir(d1)) && result.empty()) {
        if (e1->d_name[0] == '.') continue;
        std::string tier1 = std::string(base) + "/" + e1->d_name;
        DIR* d2 = opendir(tier1.c_str());
        if (!d2) continue;

        struct dirent* e2;
        while ((e2 = readdir(d2)) && result.empty()) {
            if (e2->d_name[0] == '.') continue;
            if (strncmp(e2->d_name, pkg.c_str(), pkg.size()) != 0) continue;
            if (e2->d_name[pkg.size()] != '-') continue;

            for (int i = 0; ABI_DIRS[i]; i++) {
                std::string candidate = tier1 + "/" + e2->d_name + "/lib/" + ABI_DIRS[i];
                struct stat st;
                if (stat(candidate.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
                    result = candidate;
                    break;
                }
            }
        }
        closedir(d2);
    }
    closedir(d1);

    if (!result.empty()) return result;

    std::string cmd = "pm path " + pkg + " 2>/dev/null";
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return "";

    char buf[512] = {0};
    fgets(buf, sizeof(buf), pipe);
    pclose(pipe);

    char* path_start = strstr(buf, "package:");
    if (!path_start) return "";
    path_start += 8;

    size_t len = strlen(path_start);
    while (len > 0 && (path_start[len-1] == '\n' || path_start[len-1] == '\r'))
        path_start[--len] = '\0';

    char* last_slash = strrchr(path_start, '/');
    if (!last_slash) return "";
    *last_slash = '\0';

    for (int i = 0; ABI_DIRS[i]; i++) {
        std::string candidate = std::string(path_start) + "/lib/" + ABI_DIRS[i];
        struct stat st;
        if (stat(candidate.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
            return candidate;
        }
    }
    return "";
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
        else if (command.substr(0, 9) == "got_scan:") {
            std::string pkg = command.substr(9);
            COMPANION_LOG("GOT scan for: %s", pkg.c_str());
            
            std::string lib_dir = findLibDir(pkg);
            if (lib_dir.empty()) {
                uint32_t zero = 0;
                write(fd, &zero, 4);
                close(fd);
                return;
            }
            
            struct ScanResult {
                std::string lib_name;
                std::string symbol;
                uintptr_t got_offset;
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
                        uintptr_t off = findGotOffset(path.c_str(), TARGET_SYMBOLS[i]);
                        if (!off) continue;
                        results.push_back({e->d_name, TARGET_SYMBOLS[i], off});
                    }
                }
                closedir(d);
            }
            
            uint32_t count = (uint32_t)results.size();
            write(fd, &count, 4);
            for (auto& r : results) {
                ipc_writeStr(fd, r.lib_name);
                ipc_writeStr(fd, r.symbol);
                ipc_writeU64(fd, (uint64_t)r.got_offset);
            }
            COMPANION_LOG("GOT scan found %zu targets", results.size());
        }
    }
    close(fd);
}

// ─────────────────────────────────────────
// Main Module
// ─────────────────────────────────────────
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
            }

            if (flags.needs_cpu_unmount) {
                executeCompanionCommand("unmount_spoof");
            } else if (flags.needs_cpu_mount) {
                executeCompanionCommand("mount_spoof");
            }
        }

        // GOT Hook preparation
        if (flags.needs_got_hook) {
            PKG_LOG("%s: Requesting GOT scan", package_name);
            int cfd = api->connectCompanion();
            if (cfd >= 0) {
                std::string scan_cmd = std::string("got_scan:") + package_name;
                write(cfd, scan_cmd.c_str(), scan_cmd.size());
                
                uint32_t count = 0;
                if (read(cfd, &count, 4) == 4 && count > 0) {
                    for (uint32_t i = 0; i < count; i++) {
                        GotHookEntry e;
                        uint64_t off = 0;
                        if (!ipc_readStr(cfd, e.lib_name)) break;
                        if (!ipc_readStr(cfd, e.symbol)) break;
                        if (!ipc_readU64(cfd, off)) break;
                        e.got_offset = (uintptr_t)off;
                        precomputed_hooks.push_back(e);
                    }
                    GOT_LOG("Got %zu GOT hook targets", precomputed_hooks.size());
                }
                close(cfd);
            }
            needs_got_hook = true;
            current_pkg_name = package_name;
        }

        // Stealth Mode: If no GOT hook is needed, close module immediately
        if (!needs_got_hook) {
            PKG_LOG("%s: No GOT hook needed, closing module for stealth", package_name);
            api->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
        }
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs* args) override {
        if (needs_got_hook && !precomputed_hooks.empty()) {
            applyGotHooksAsync();
        }
    }

private:
    zygisk::Api* api;
    JNIEnv* env;
    // ✅ CORRECT STRUCTURE: DeviceInfo + map of package_name -> PackageFlags
    std::vector<std::pair<DeviceInfo, std::unordered_map<std::string, PackageFlags>>> device_packages;
    std::vector<GotHookEntry> precomputed_hooks;
    bool needs_got_hook = false;
    std::string current_pkg_name;

    // ─────────────────────────────────────────
    // GOT Hook helpers (Async)
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
            if (sscanf(line, "%lx-%lx %s %lx", &start, &end, perms, &offset) != 4) continue;
            if (offset == 0) { base = start; break; }
        }
        fclose(maps);
        return base;
    }

    static bool applyGotHook(GotHookEntry& e) {
        uintptr_t got = e.load_base + e.got_offset;
        e.original = *(void**)got;
        if (!e.original) return false;

        void* hook = nullptr;
        if (e.symbol == "__system_property_get") hook = (void*)hooked_prop_get;
        else if (e.symbol == "__system_property_read_callback") hook = (void*)hooked_prop_read;
        else if (e.symbol == "__system_property_find") hook = (void*)hooked_prop_find;
        else if (e.symbol == "__system_property_read") hook = (void*)hooked_prop_read_old;
        if (!hook) return false;

        size_t ps = getpagesize();
        uintptr_t pg = got & ~(ps - 1);
        if (mprotect((void*)pg, ps, PROT_READ | PROT_WRITE) != 0) return false;

        *(void**)got = hook;
        mprotect((void*)pg, ps, PROT_READ);

        e.hooked = (*(void**)got == hook);
        if (e.hooked) GOT_LOG("✅ Hooked: %s -> %s", e.symbol.c_str(), e.lib_name.c_str());
        return e.hooked;
    }

    void applyGotHooksAsync() {
        std::vector<GotHookEntry> hooks_to_apply = precomputed_hooks;
        std::string pkg_name = current_pkg_name;
        DeviceInfo info_copy = current_info;
        
        std::thread([hooks_to_apply, pkg_name, info_copy]() mutable {
            GOT_LOG("Hook thread started for %s", pkg_name.c_str());
            
            std::lock_guard<std::mutex> lock(info_mutex);
            g_got_ctx = new GotProcessContext();
            g_got_ctx->props = info_copy.prop_overrides;
            g_got_ctx->package_name = pkg_name;
            
            if (g_got_ctx->props.empty()) {
                if (!info_copy.model.empty()) g_got_ctx->props["ro.product.model"] = info_copy.model;
                if (!info_copy.brand.empty()) g_got_ctx->props["ro.product.brand"] = info_copy.brand;
                if (!info_copy.manufacturer.empty()) g_got_ctx->props["ro.product.manufacturer"] = info_copy.manufacturer;
                if (!info_copy.device.empty()) g_got_ctx->props["ro.product.device"] = info_copy.device;
                if (!info_copy.fingerprint.empty()) g_got_ctx->props["ro.build.fingerprint"] = info_copy.fingerprint;
                if (!info_copy.product.empty()) g_got_ctx->props["ro.product.name"] = info_copy.product;
            }
            
            for (int w = 0; w < 30; w++) {
                bool any_loaded = false;
                for (auto& h : hooks_to_apply) {
                    if (getLoadBase(h.lib_name.c_str())) {
                        any_loaded = true;
                        break;
                    }
                }
                if (any_loaded) {
                    sleep(1);
                    break;
                }
                sleep(1);
            }
            
            for (auto& h : hooks_to_apply) {
                h.load_base = getLoadBase(h.lib_name.c_str());
                if (!h.load_base) continue;
                
                if (applyGotHook(h)) {
                    if (h.symbol == "__system_property_get") g_got_ctx->orig_get = (prop_get_t)h.original;
                    else if (h.symbol == "__system_property_read_callback") g_got_ctx->orig_read_cb = (prop_read_t)h.original;
                    else if (h.symbol == "__system_property_find") g_got_ctx->orig_find = (prop_find_t)h.original;
                    else if (h.symbol == "__system_property_read") g_got_ctx->orig_read_old = (prop_read_old_t)h.original;
                    g_got_ctx->hooks.push_back(h);
                }
            }
            
            g_got_ctx->ready.store(true);
            GOT_LOG("✅ Installed %zu GOT hooks for %s", g_got_ctx->hooks.size(), pkg_name.c_str());
        }).detach();
    }

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
        if (tags.find("got") != tags.end()) {
            flags.needs_got_hook = true;
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
        
        SPOOF_LOG("Device spoofed: %s (%s)", info.model.c_str(), info.brand.c_str());
    }
};

REGISTER_ZYGISK_MODULE(COPGModule)
REGISTER_ZYGISK_COMPANION(companion)
