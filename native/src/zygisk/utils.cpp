#include <cinttypes>
#include <base.hpp>

#include "zygisk.hpp"
#include <lsplt.hpp>

using namespace std;
static vector<lsplt::MapInfo> find_maps(const char *name) {
    auto maps = lsplt::MapInfo::Scan();
    for (auto iter = maps.begin(); iter != maps.end();) {
        if (iter->path != name) {
            iter = maps.erase(iter);
        } else {
            ++iter;
        }
    }
    return maps;
}

void unmap_all(const char *name) {
    auto maps = find_maps(name);
    for (auto &info : maps) {
        void *addr = reinterpret_cast<void *>(info.start);
        size_t size = info.end - info.start;
        if (info.perms & PROT_READ) {
            // Make sure readable pages are still readable
            void *dummy = xmmap(nullptr, size, PROT_READ, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
            mremap(dummy, size, size, MREMAP_MAYMOVE | MREMAP_FIXED, addr);
        } else {
            munmap(addr, size);
        }
    }
}

void remap_all(const char *name) {
    auto maps = find_maps(name);
    for (auto &info : maps) {
        void *addr = reinterpret_cast<void *>(info.start);
        size_t size = info.end - info.start;
        void *copy = xmmap(nullptr, size, PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
        if ((info.perms & PROT_READ) == 0) {
            mprotect(addr, size, PROT_READ);
        }
        memcpy(copy, addr, size);
        mremap(copy, size, size, MREMAP_MAYMOVE | MREMAP_FIXED, addr);
        mprotect(addr, size, info.perms);
    }
}

uintptr_t get_function_off(int pid, uintptr_t addr, char *lib) {
    for (auto &info : lsplt::MapInfo::Scan()) {
        if (addr >= info.start && addr < info.end) {
            if (lib)
                strcpy(lib, info.path.data());
            return addr - info.start + info.offset;
        }
    }
    return 0;
}

uintptr_t get_function_addr(int pid, const char *lib, uintptr_t off) {
    for (auto &info : lsplt::MapInfo::Scan()) {
        if (info.path == lib && (info.perms & PROT_EXEC)) {
            return info.start - info.offset + off;
        }
    }
    return 0;
}
