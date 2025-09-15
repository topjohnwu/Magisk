#include <csignal>
#include <libgen.h>
#include <sys/un.h>
#include <sys/mount.h>
#include <sys/sysmacros.h>
#include <linux/input.h>
#include <map>

#include <consts.hpp>
#include <base.hpp>
#include <core.hpp>

using namespace std;

bool read_string(int fd, std::string &str) {
    str.clear();
    int len = read_int(fd);
    str.resize(len);
    return xxread(fd, str.data(), len) == len;
}

string read_string(int fd) {
    string str;
    read_string(fd, str);
    return str;
}

void write_string(int fd, string_view str) {
    if (fd < 0) return;
    write_int(fd, str.size());
    xwrite(fd, str.data(), str.size());
}

const char *get_magisk_tmp() {
    static const char *path = nullptr;
    if (path == nullptr) {
        if (access("/debug_ramdisk/" INTLROOT, F_OK) == 0) {
            path = "/debug_ramdisk";
        } else if (access("/sbin/" INTLROOT, F_OK) == 0) {
            path = "/sbin";
        } else {
            path = "";
        }
    }
    return path;
}

int connect_daemon(RequestCode req, bool create) {
    int fd = xsocket(AF_LOCAL, SOCK_STREAM | SOCK_CLOEXEC, 0);
    sockaddr_un addr = {.sun_family = AF_LOCAL};
    const char *tmp = get_magisk_tmp();
    ssprintf(addr.sun_path, sizeof(addr.sun_path), "%s/" MAIN_SOCKET, tmp);
    if (connect(fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr))) {
        if (!create || getuid() != AID_ROOT) {
            PLOGE("Cannot connect daemon at %s,", addr.sun_path);
            close(fd);
            return -1;
        }

        char buf[64];
        xreadlink("/proc/self/exe", buf, sizeof(buf));
        if (tmp[0] == '\0' || !string_view(buf).starts_with(tmp)) {
            LOGE("Start daemon on magisk tmpfs\n");
            close(fd);
            return -1;
        }

        if (fork_dont_care() == 0) {
            close(fd);
            set_nice_name("magiskd");
            daemon_entry();
        }

        while (connect(fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)))
            usleep(10000);
    }
    write_int(fd, +req);
    int res = read_int(fd);
    if (res < +RespondCode::ERROR || res >= +RespondCode::END)
        res = +RespondCode::ERROR;
    switch (res) {
    case +RespondCode::OK:
        break;
    case +RespondCode::ERROR:
        LOGE("Daemon error\n");
        close(fd);
        return -1;
    case +RespondCode::ROOT_REQUIRED:
        LOGE("Root is required for this operation\n");
        close(fd);
        return -1;
    case +RespondCode::ACCESS_DENIED:
        LOGE("Access denied\n");
        close(fd);
        return -1;
    default:
        __builtin_unreachable();
    }
    return fd;
}

void unlock_blocks() {
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

bool check_key_combo() {
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
