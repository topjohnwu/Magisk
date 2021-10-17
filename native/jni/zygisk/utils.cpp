#include <cinttypes>
#include <utils.hpp>

#include "zygisk.hpp"

using namespace std;

namespace {

struct map_info {
    uintptr_t start;
    uintptr_t end;
    uintptr_t off;
    int perms;
    const char *path;

    map_info() : start(0), end(0), off(0), perms(0), path(nullptr) {}
};

} // namespace

template<typename Func>
static void parse_maps(int pid, Func fn) {
    char file[32];

    // format: start-end perms offset dev inode path
    sprintf(file, "/proc/%d/maps", pid);
    file_readline(true, file, [=](string_view l) -> bool {
        char *line = (char *) l.data();
        map_info info;
        char perm[5];
        int path_off;

        if (sscanf(line, "%" PRIxPTR "-%" PRIxPTR " %4s %" PRIxPTR " %*x:%*x %*d %n%*s",
                   &info.start, &info.end, perm, &info.off, &path_off) != 4)
            return true;

        // Parse permissions
        if (perm[0] != '-')
            info.perms |= PROT_READ;
        if (perm[1] != '-')
            info.perms |= PROT_WRITE;
        if (perm[2] != '-')
            info.perms |= PROT_EXEC;

        info.path = line + path_off;

        return fn(info);
    });
}

static vector<map_info> find_maps(const char *name) {
    vector<map_info> maps;
    parse_maps(getpid(), [=, &maps](map_info &info) -> bool {
        if (strcmp(info.path, name) == 0)
            maps.emplace_back(info);
        return true;
    });
    return maps;
}

void unmap_all(const char *name) {
    vector<map_info> maps = find_maps(name);
    for (map_info &info : maps) {
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
    vector<map_info> maps = find_maps(name);
    for (map_info &info : maps) {
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
    uintptr_t off = 0;
    parse_maps(pid, [=, &off](map_info &info) -> bool {
        if (addr >= info.start && addr < info.end) {
            if (lib)
                strcpy(lib, info.path);
            off = addr - info.start + info.off;
            return false;
        }
        return true;
    });
    return off;
}

uintptr_t get_function_addr(int pid, const char *lib, uintptr_t off) {
    uintptr_t addr = 0;
    parse_maps(pid, [=, &addr](map_info &info) -> bool {
        if (strcmp(info.path, lib) == 0 && (info.perms & PROT_EXEC)) {
            addr = info.start - info.off + off;
            return false;
        }
        return true;
    });
    return addr;
}
