#include <utils.hpp>

#include "inject.hpp"

using namespace std;

namespace {

struct map_info {
    uintptr_t start;
    uintptr_t end;
    int perms;
    char *path;

    map_info() : start(0), end(0), perms(0), path(nullptr) {}

    enum {
        EXEC = (1 << 0),
        WRITE = (1 << 1),
        READ = (1 << 2),
    };
};

} // namespace

template<typename Func>
static void parse_maps(int pid, Func fn) {
    char file[32];

    // format: start-end perms offset dev inode path
    sprintf(file, "/proc/%d/maps", pid);
    file_readline(true, file, [=](string_view l) -> bool {
        char *pos = (char *) l.data();
        map_info info;

        // Parse address hex strings
        info.start = strtoul(pos, &pos, 16);
        info.end = strtoul(++pos, &pos, 16);

        // Parse permissions
        if (*(++pos) != '-')
            info.perms |= map_info::READ;
        if (*(++pos) != '-')
            info.perms |= map_info::WRITE;
        if (*(++pos) != '-')
            info.perms |= map_info::EXEC;
        pos += 3;

        // Skip everything except path
        int path_off;
        sscanf(pos, "%*s %*s %*s %n%*s", &path_off);
        pos += path_off;
        info.path = pos;

        return fn(info);
    });
}

void unmap_all(const char *name) {
    vector<map_info> unmaps;
    parse_maps(getpid(), [=, &unmaps](map_info &info) -> bool {
        if (strcmp(info.path, name) == 0)
            unmaps.emplace_back(info);
        return true;
    });
    for (map_info &info : unmaps) {
        void *addr = reinterpret_cast<void *>(info.start);
        size_t size = info.end - info.start;
        munmap(addr, size);
        if (info.perms & map_info::READ) {
            // Make sure readable pages are still readable
            xmmap(addr, size, PROT_READ, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
        }
    }
}

uintptr_t get_function_lib(uintptr_t addr, char *lib) {
    uintptr_t base = 0;
    parse_maps(getpid(), [=, &base](map_info &info) -> bool {
        if (addr >= info.start && addr < info.end) {
            if (lib)
                strcpy(lib, info.path);
            base = info.start;
            return false;
        }
        return true;
    });
    return base;
}

uintptr_t get_remote_lib(int pid, const char *lib) {
    uintptr_t base = 0;
    parse_maps(pid, [=, &base](map_info &info) -> bool {
        if (strcmp(info.path, lib) == 0 && (info.perms & map_info::EXEC)) {
            base = info.start;
            return false;
        }
        return true;
    });
    return base;
}
