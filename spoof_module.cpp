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

static const char* ABI_DIRS[] = {
    "arm64",        // arm64-v8a
    "arm64-v8a",    // alias
    "arm",          // armeabi-v7a
    "armeabi-v7a",  // alias
    "x86_64",       // x86_64
    "x86",          // x86
    nullptr
};

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
static thread_local prop_read_cb_t g_app_callback = nullptr;

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

static uintptr_t findFromSections64(FILE* f, const Elf64_Ehdr& ehdr,
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

static uintptr_t findFromDynamic64(FILE* f, const Elf64_Ehdr& ehdr,
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

    uintptr_t strtab_va=0, symtab_va=0;
    uintptr_t plt_va=0, plt_sz=0, rela_va=0, rela_sz=0;
    uintptr_t syment = sizeof(Elf64_Sym);

    for (auto& d : dyns) {
        switch(d.d_tag) {
            case DT_STRTAB:    strtab_va = d.d_un.d_ptr; break;
            case DT_SYMTAB:    symtab_va = d.d_un.d_ptr; break;
            case DT_SYMENT:    syment    = d.d_un.d_val; break;
            case DT_JMPREL:    plt_va    = d.d_un.d_ptr; break;
            case DT_PLTRELSZ:  plt_sz    = d.d_un.d_val; break;
            case DT_RELA:      rela_va   = d.d_un.d_ptr; break;
            case DT_RELASZ:    rela_sz   = d.d_un.d_val; break;
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

    // اولویت با JMPREL (PLT) سپس RELA
    uintptr_t off = searchRela(plt_va, plt_sz);
    if (!off) off = searchRela(rela_va, rela_sz);
    return off;
}

static uintptr_t findFromSections32(FILE* f, const Elf32_Ehdr& ehdr,
                                     const char* symbol) {
    std::vector<Elf32_Shdr> shdrs(ehdr.e_shnum);
    fseek(f, ehdr.e_shoff, SEEK_SET);
    if (fread(shdrs.data(), sizeof(Elf32_Shdr), ehdr.e_shnum, f)
        != (size_t)ehdr.e_shnum) return 0;

    Elf32_Shdr& shstr_hdr = shdrs[ehdr.e_shstrndx];
    std::vector<char> shstrtab(shstr_hdr.sh_size);
    fseek(f, shstr_hdr.sh_offset, SEEK_SET);
    fread(shstrtab.data(), 1, shstr_hdr.sh_size, f);

    Elf32_Shdr *rel_plt=nullptr, *rel_dyn=nullptr,
               *dynsym_h=nullptr, *dynstr_h=nullptr;
    for (auto& s : shdrs) {
        const char* name = shstrtab.data() + s.sh_name;
        if      (!strcmp(name, ".rel.plt"))  rel_plt  = &s;
        else if (!strcmp(name, ".rel.dyn"))  rel_dyn  = &s;
        else if (!strcmp(name, ".dynsym"))   dynsym_h = &s;
        else if (!strcmp(name, ".dynstr"))   dynstr_h = &s;
    }
    if (!dynsym_h || !dynstr_h) return 0;

    std::vector<char> dynstr(dynstr_h->sh_size);
    fseek(f, dynstr_h->sh_offset, SEEK_SET);
    fread(dynstr.data(), 1, dynstr_h->sh_size, f);

    size_t sym_count = dynsym_h->sh_size / sizeof(Elf32_Sym);
    std::vector<Elf32_Sym> syms(sym_count);
    fseek(f, dynsym_h->sh_offset, SEEK_SET);
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

static uintptr_t findFromDynamic32(FILE* f, const Elf32_Ehdr& ehdr,
                                    const char* symbol) {
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

    uint32_t strtab_va=0, symtab_va=0;
    uint32_t rel_va=0,    rel_sz=0;
    uint32_t jmprel_va=0, jmprel_sz=0;
    uint32_t syment = sizeof(Elf32_Sym);

    for (auto& d : dyns) {
        switch(d.d_tag) {
            case DT_STRTAB:    strtab_va  = d.d_un.d_ptr; break;
            case DT_SYMTAB:    symtab_va  = d.d_un.d_ptr; break;
            case DT_REL:       rel_va     = d.d_un.d_ptr; break;
            case DT_RELSZ:     rel_sz     = d.d_un.d_val; break;
            case DT_JMPREL:    jmprel_va  = d.d_un.d_ptr; break;
            case DT_PLTRELSZ:  jmprel_sz  = d.d_un.d_val; break;
            case DT_SYMENT:    syment     = d.d_un.d_val; break;
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

static uintptr_t findGotOffset(const char* path, const char* sym) {
    FILE* f = fopen(path, "rb");
    if (!f) return 0;

    unsigned char ident[EI_NIDENT];
    if (fread(ident, 1, EI_NIDENT, f) != EI_NIDENT) {
        fclose(f);
        return 0;
    }
    rewind(f);

    bool is64 = (ident[EI_CLASS] == ELFCLASS64);
    uintptr_t off = 0;

    if (is64) {
        Elf64_Ehdr ehdr;
        if (fread(&ehdr, sizeof(ehdr), 1, f) == 1 &&
            memcmp(ehdr.e_ident, ELFMAG, SELFMAG) == 0) {
            if (ehdr.e_shoff) off = findFromSections64(f, ehdr, sym);
            if (!off)         off = findFromDynamic64(f, ehdr, sym);
        }
    } else {
        Elf32_Ehdr ehdr;
        if (fread(&ehdr, sizeof(ehdr), 1, f) == 1 &&
            memcmp(ehdr.e_ident, ELFMAG, SELFMAG) == 0) {
            if (ehdr.e_shoff) off = findFromSections32(f, ehdr, sym);
            if (!off)         off = findFromDynamic32(f, ehdr, sym);
        }
    }

    fclose(f);
    return off;
}

static std::string findLibDirFromDataApp(const std::string& pkg) {
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
            if (strncmp(e2->d_name, pkg.c_str(), pkg.size()) != 0)
                continue;
            if (e2->d_name[pkg.size()] != '-') continue;

            for (int i = 0; ABI_DIRS[i]; i++) {
                std::string candidate = tier1 + "/" + e2->d_name
                                      + "/lib/" + ABI_DIRS[i];
                struct stat st;
                if (stat(candidate.c_str(), &st) == 0
                    && S_ISDIR(st.st_mode)) {
                    result = candidate;
                    LOGI("[companion] found lib dir (%s): %s",
                         ABI_DIRS[i], candidate.c_str());
                    break;
                }
            }
        }
        closedir(d2);
    }
    closedir(d1);
    return result;
}

static std::string findLibDirFromPm(const std::string& pkg) {
    std::string cmd = "pm path " + pkg + " 2>/dev/null";
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return "";

    char buf[512] = {0};
    fgets(buf, sizeof(buf), pipe);
    pclose(pipe);

    const char* prefix = "package:";
    char* path_start = strstr(buf, prefix);
    if (!path_start) return "";
    path_start += strlen(prefix);

    size_t len = strlen(path_start);
    while (len > 0 && (path_start[len-1] == '\n' || path_start[len-1] == '\r'))
        path_start[--len] = '\0';

    char* last_slash = strrchr(path_start, '/');
    if (!last_slash) return "";
    *last_slash = '\0';

    for (int i = 0; ABI_DIRS[i]; i++) {
        std::string candidate = std::string(path_start)
                              + "/lib/" + ABI_DIRS[i];
        struct stat st;
        if (stat(candidate.c_str(), &st) == 0
            && S_ISDIR(st.st_mode)) {
            LOGI("[companion] pm fallback found (%s): %s",
                 ABI_DIRS[i], candidate.c_str());
            return candidate;
        }
    }
    return "";
}

static std::string findLibDir(const std::string& pkg) {
    std::string dir = findLibDirFromDataApp(pkg);
    if (!dir.empty()) return dir;

    LOGI("[companion] /data/app scan failed, trying pm...");
    dir = findLibDirFromPm(pkg);
    if (!dir.empty()) return dir;

    LOGE("[companion] lib dir not found for %s", pkg.c_str());
    return "";
}

static void companion(int fd) {
    LOGI("[companion] started");

    std::string pkg;
    if (!readStr(fd, pkg)) {
        LOGE("[companion] read pkg failed");
        return;
    }
    LOGI("[companion] scanning: %s", pkg.c_str());

    std::string lib_dir = findLibDir(pkg);
    if (lib_dir.empty()) {
        uint32_t zero = 0;
        write(fd, &zero, 4);
        return;
    }

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
                uintptr_t off = findGotOffset(path.c_str(), TARGET_SYMBOLS[i]);
                if (!off) continue;

                LOGI("[companion] %s in %s @ 0x%lx",
                     TARGET_SYMBOLS[i], e->d_name, off);
                results.push_back({e->d_name, TARGET_SYMBOLS[i], off});
            }
        }
        closedir(d);
    }

    LOGI("[companion] found %zu targets", results.size());

    uint32_t count = (uint32_t)results.size();
    write(fd, &count, 4);

    for (auto& r : results) {
        writeStr(fd, r.lib_name);
        writeStr(fd, r.symbol);
        writeU64(fd, (uint64_t)r.got_offset);
    }
}

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

        const char* raw = env->GetStringUTFChars(args->nice_name, nullptr);
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

        int cfd = api->connectCompanion();
        if (cfd < 0) {
            LOGE("connectCompanion failed");
            api->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
            return;
        }

        writeStr(cfd, pkg);

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

        LOGI("Got %zu hook targets", precomputed.size());

        current_pkg   = pkg;
        current_props = it->second;
        needs_hook    = true;
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs* args) override {
        if (!needs_hook) {
            api->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
            return;
        }

        g_ctx = new ProcessContext();
        g_ctx->props        = current_props;
        g_ctx->package_name = current_pkg;

        std::vector<HookEntry> to_hook = precomputed;

        std::thread([to_hook]() mutable {
            LOGI("Hook thread: %zu targets", to_hook.size());

            for (int w = 0; w < 30; w++) {
                bool any = false;
                for (auto& h : to_hook)
                    if (getLoadBase(h.lib_name.c_str())) { any = true; break; }
                if (any) { sleep(1); break; }
                sleep(1);
            }

            for (auto& h : to_hook) {
                h.load_base = getLoadBase(h.lib_name.c_str());
                if (!h.load_base) continue;

                if (applyHook(h)) {
                    if (h.symbol == "__system_property_get")
                        g_ctx->orig_get = (prop_get_t)h.original;
                    else if (h.symbol == "__system_property_read_callback")
                        g_ctx->orig_read_cb = (prop_read_t)h.original;
                    else if (h.symbol == "__system_property_find")
                        g_ctx->orig_find = (prop_find_t)h.original;
                    else if (h.symbol == "__system_property_read")
                        g_ctx->orig_read_old = (prop_read_old_t)h.original;
                    g_ctx->hooks.push_back(h);
                }
            }

            g_ctx->ready.store(true);
            LOGI("Done: %zu hooks for %s",
                 g_ctx->hooks.size(), g_ctx->package_name.c_str());
        }).detach();
    }

private:
    zygisk::Api* api;
    JNIEnv*      env;
    bool         needs_hook = false;
    std::string  current_pkg;
    std::vector<HookEntry> precomputed;
    std::unordered_map<std::string, std::string> current_props;
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> package_props;

    void loadConfig() {
        std::ifstream f("/data/adb/modules/COPG/COPG.json");
        if (!f.is_open()) return;
        try {
            json cfg = json::parse(f);
            if (!cfg.contains("got_hooks")) return;
            for (auto& [pkg, data] : cfg["got_hooks"].items()) {
                if (!data.contains("props")) continue;
                std::unordered_map<std::string, std::string> props;
                for (auto& [k, v] : data["props"].items())
                    props[k] = v.get<std::string>();
                package_props[pkg] = props;
                LOGI("Config loaded: %s (%zu props)", pkg.c_str(), props.size());
            }
        } catch(...) { LOGE("Config error"); }
    }
};

REGISTER_ZYGISK_MODULE(COPGGotHook)
REGISTER_ZYGISK_COMPANION(companion)
