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
    unsigned long inode;
    std::string path;

    map_info() : start(0), end(0), off(0), perms(0), inode(0), path() {}
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

        if (sscanf(line, "%" PRIxPTR "-%" PRIxPTR " %4s %" PRIxPTR " %*x:%*x %lu %n%*s",
                   &info.start, &info.end, perm, &info.off, &info.inode, &path_off) != 5)
            return true;

        // Parse permissions
        if (perm[0] != '-')
            info.perms |= PROT_READ;
        if (perm[1] != '-')
            info.perms |= PROT_WRITE;
        if (perm[2] != '-')
            info.perms |= PROT_EXEC;

        info.path = line + path_off;

        return fn(std::move(info));
    });
}

static vector<map_info> find_maps(const char *name) {
    vector<map_info> maps;
    parse_maps(getpid(), [=, &maps](map_info &&info) -> bool {
        if (info.path == name)
            maps.emplace_back(std::move(info));
        return true;
    });
    return maps;
}

std::tuple<void *, size_t> find_map_range(const char *name, unsigned long inode) {
    vector<map_info> maps = find_maps(name);
    uintptr_t start = 0u;
    uintptr_t end = 0u;
    for (const auto &map : maps) {
        if (map.inode == inode) {
            if (start == 0) {
                start = map.start;
                end = map.end;
            } else if (map.start == end) {
                end = map.end;
            }
        }
    }
    LOGD("found map %s with start = %zx, end = %zx\n", name, start, end);
    return {reinterpret_cast<void *>(start), end - start};
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
    parse_maps(pid, [=, &off](map_info &&info) -> bool {
        if (addr >= info.start && addr < info.end) {
            if (lib)
                strcpy(lib, info.path.data());
            off = addr - info.start + info.off;
            return false;
        }
        return true;
    });
    return off;
}

uintptr_t get_function_addr(int pid, const char *lib, uintptr_t off) {
    uintptr_t addr = 0;
    parse_maps(pid, [=, &addr](map_info &&info) -> bool {
        if (info.path == lib && (info.perms & PROT_EXEC)) {
            addr = info.start - info.off + off;
            return false;
        }
        return true;
    });
    return addr;
}
