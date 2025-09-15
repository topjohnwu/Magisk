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

int SDK_INT = -1;

static struct stat self_st;

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

static bool is_client(pid_t pid) {
    // Verify caller is the same as server
    char path[32];
    sprintf(path, "/proc/%d/exe", pid);
    struct stat st{};
    return !(stat(path, &st) || st.st_dev != self_st.st_dev || st.st_ino != self_st.st_ino);
}

static void handle_request(owned_fd client) {
    // Verify client credentials
    ucred cred{};

    socklen_t len = sizeof(cred);
    // Client died
    if (getsockopt(client, SOL_SOCKET, SO_PEERCRED, &cred, &len) != 0)
        return;
    char context[256];
    len = sizeof(context);
    if (getsockopt(client, SOL_SOCKET, SO_PEERSEC, context, &len) != 0)
        len = 0;
    context[len] = '\0';

    bool is_root = cred.uid == AID_ROOT;
    bool is_zygote = context == "u:r:zygote:s0"sv;

    if (!is_root && !is_zygote && !is_client(cred.pid)) {
        // Unsupported client state
        write_int(client, +RespondCode::ACCESS_DENIED);
        return;
    }

    int code = read_int(client);
    if (code < 0 || code >= +RequestCode::END ||
        code == +RequestCode::_SYNC_BARRIER_ ||
        code == +RequestCode::_STAGE_BARRIER_) {
        // Unknown request code
        return;
    }

    // Check client permissions
    switch (code) {
    case +RequestCode::POST_FS_DATA:
    case +RequestCode::LATE_START:
    case +RequestCode::BOOT_COMPLETE:
    case +RequestCode::ZYGOTE_RESTART:
    case +RequestCode::SQLITE_CMD:
    case +RequestCode::DENYLIST:
    case +RequestCode::STOP_DAEMON:
        if (!is_root) {
            write_int(client, +RespondCode::ROOT_REQUIRED);
            return;
        }
        break;
    case +RequestCode::REMOVE_MODULES:
        if (!is_root && cred.uid != AID_SHELL) {
            write_int(client, +RespondCode::ACCESS_DENIED);
            return;
        }
        break;
    case +RequestCode::ZYGISK:
        if (!is_zygote) {
            // Invalid client context
            write_int(client, +RespondCode::ACCESS_DENIED);
            return;
        }
        break;
    default:
        break;
    }

    write_int(client, +RespondCode::OK);

    if (code < +RequestCode::_SYNC_BARRIER_) {
        MagiskD::Get().handle_request_sync(client.release(), code);
    } else if (code < +RequestCode::_STAGE_BARRIER_) {
        exec_task([=, fd = client.release()] {
            MagiskD::Get().handle_request_async(fd, code, cred);
        });
    } else {
        exec_task([=, fd = client.release()] {
            MagiskD::Get().boot_stage_handler(fd, code);
        });
    }
}

[[noreturn]] static void daemon_entry() {
    // Change process name
    set_nice_name("magiskd");

    rust::daemon_entry();
    SDK_INT = MagiskD::Get().sdk_int();

    // Get self stat
    xstat("/proc/self/exe", &self_st);

    int sockfd = xsocket(AF_LOCAL, SOCK_STREAM | SOCK_CLOEXEC, 0);
    sockaddr_un addr = {.sun_family = AF_LOCAL};
    ssprintf(addr.sun_path, sizeof(addr.sun_path), "%s/" MAIN_SOCKET, get_magisk_tmp());
    unlink(addr.sun_path);
    if (xbind(sockfd, (sockaddr *) &addr, sizeof(addr)))
        exit(1);
    chmod(addr.sun_path, 0666);
    setfilecon(addr.sun_path, MAGISK_FILE_CON);
    xlisten(sockfd, 10);

    // Loop forever to listen for requests
    for (;;) {
        int client = xaccept4(sockfd, nullptr, nullptr, SOCK_CLOEXEC);
        handle_request(client);
    }
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

int connect_daemon(int req, bool create) {
    int fd = xsocket(AF_LOCAL, SOCK_STREAM | SOCK_CLOEXEC, 0);
    sockaddr_un addr = {.sun_family = AF_LOCAL};
    const char *tmp = get_magisk_tmp();
    ssprintf(addr.sun_path, sizeof(addr.sun_path), "%s/" MAIN_SOCKET, tmp);
    if (connect(fd, (sockaddr *) &addr, sizeof(addr))) {
        if (!create || getuid() != AID_ROOT) {
            LOGE("No daemon is currently running!\n");
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
            daemon_entry();
        }

        while (connect(fd, (sockaddr *) &addr, sizeof(addr)))
            usleep(10000);
    }
    write_int(fd, req);
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
