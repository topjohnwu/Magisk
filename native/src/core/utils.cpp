module;
#include <csignal>
#include <libgen.h>
#include <sys/mount.h>
#include <sys/sysmacros.h>
#include <linux/input.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <rust/cxx.h>

export module magisk.core;
export import magisk.base;
export import magisk.sqlite;

#define PLOGE(fmt, args...) LOGE(fmt " failed with %d: %s\n", ##args, errno, ::strerror(errno))

export {
#include "core-rs.hpp"

inline constexpr int AID_ROOT = 0;
inline constexpr int AID_SHELL = 2000;
inline constexpr int AID_USER_OFFSET = 100000;
constexpr auto to_app_id(auto uid) { return uid % AID_USER_OFFSET; }
constexpr auto to_user_id(auto uid) { return uid / AID_USER_OFFSET; }
inline int SDK_INT() { return MagiskD::Get().sdk_int(); }
inline const char *APP_DATA_DIR() { return SDK_INT() >= 24 ? "/data/user_de" : "/data/user"; }

inline int connect_daemon(RequestCode req) {
    return connect_daemon(req, false);
}

// Utils

template<typename T> requires(std::is_trivially_copyable_v<T>)
T read_any(int fd) {
    T val;
    if (xxread(fd, &val, sizeof(val)) != sizeof(val))
        return -1;
    return val;
}
template<typename T> requires(std::is_trivially_copyable_v<T>)
void write_any(int fd, T val) {
    if (fd < 0) return;
    xwrite(fd, &val, sizeof(val));
}
inline int read_int(int fd) { return read_any<int>(fd); }
inline void write_int(int fd, int val) { write_any(fd, val); }

template<typename T> requires(std::is_trivially_copyable_v<T>)
void write_vector(int fd, const std::vector<T> &vec) {
    write_int(fd, vec.size());
    xwrite(fd, vec.data(), vec.size() * sizeof(T));
}
template<typename T> requires(std::is_trivially_copyable_v<T>)
bool read_vector(int fd, std::vector<T> &vec) {
    int size = read_int(fd);
    vec.resize(size);
    return xread(fd, vec.data(), size * sizeof(T)) == size * sizeof(T);
}

// Rust bindings

inline rust::String resolve_preinit_dir_rs(Utf8CStr base_dir) {
    return resolve_preinit_dir(base_dir.c_str());
}
}

using namespace std;

export bool read_string(int fd, std::string &str) {
    str.clear();
    int len = read_int(fd);
    str.resize(len);
    return xxread(fd, str.data(), len) == len;
}

export string read_string(int fd) {
    string str;
    read_string(fd, str);
    return str;
}

export void write_string(int fd, string_view str) {
    if (fd < 0) return;
    write_int(fd, str.size());
    xwrite(fd, str.data(), str.size());
}

export const char *get_magisk_tmp() {
    static const char *path = nullptr;
    if (path == nullptr) {
        if (access(concat<"/debug_ramdisk/", INTLROOT>.value, F_OK) == 0) {
            path = "/debug_ramdisk";
        } else if (access(concat<"/sbin/", INTLROOT>.value, F_OK) == 0) {
            path = "/sbin";
        } else {
            path = "";
        }
    }
    return path;
}

export void unlock_blocks() {
    int fd, dev, OFF = 0;

    auto dir = xopen_dir("/dev/block");
    if (!dir)
        return;
    dev = dirfd(dir.get());

    for (dirent *entry; (entry = readdir(dir.get()));) {
        if (entry->d_type == DT_BLK) {
            if ((fd = openat(dev, entry->d_name, O_RDONLY | O_CLOEXEC)) < 0)
                continue;
            if (ioctl(fd, BLKROSET, &OFF) < 0)
                PLOGE("unlock %s", entry->d_name);
            close(fd);
        }
    }
}

#define test_bit(bit, array) (array[bit / 8] & (1 << (bit % 8)))

export bool check_key_combo() {
    uint8_t bitmask[(KEY_MAX + 1) / 8];
    vector<owned_fd> events;
    constexpr char name[] = "/dev/.ev";

    // First collect candidate events that accepts volume down
    for (int minor = 64; minor < 96; ++minor) {
        if (xmknod(name, S_IFCHR | 0444, makedev(13, minor)))
            continue;
        int fd = open(name, O_RDONLY | O_CLOEXEC);
        unlink(name);
        if (fd < 0)
            continue;
        memset(bitmask, 0, sizeof(bitmask));
        ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(bitmask)), bitmask);
        if (test_bit(KEY_VOLUMEDOWN, bitmask))
            events.emplace_back(fd);
        else
            close(fd);
    }
    if (events.empty())
        return false;

    // Check if volume down key is held continuously for more than 3 seconds
    for (int i = 0; i < 300; ++i) {
        bool pressed = false;
        for (int fd : events) {
            memset(bitmask, 0, sizeof(bitmask));
            ioctl(fd, EVIOCGKEY(sizeof(bitmask)), bitmask);
            if (test_bit(KEY_VOLUMEDOWN, bitmask)) {
                pressed = true;
                break;
            }
        }
        if (!pressed)
            return false;
        // Check every 10ms
        usleep(10000);
    }
    LOGD("KEY_VOLUMEDOWN detected: enter safe mode\n");
    return true;
}

export inline Utf8CStr get_magisk_tmp_rs() { return get_magisk_tmp(); }
