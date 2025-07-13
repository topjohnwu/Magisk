#include <csignal>
#include <libgen.h>
#include <sys/un.h>
#include <sys/mount.h>
#include <sys/sysmacros.h>
#include <linux/input.h>

#include <consts.hpp>
#include <base.hpp>
#include <core.hpp>
#include <flags.h>

using namespace std;

int SDK_INT = -1;

static struct stat self_st;

static map<int, poll_callback> *poll_map;
static vector<pollfd> *poll_fds;
static int poll_ctrl;

enum {
    POLL_CTRL_NEW,
    POLL_CTRL_RM,
};

void register_poll(const pollfd *pfd, poll_callback callback) {
    if (gettid() == getpid()) {
        // On main thread, directly modify
        poll_map->try_emplace(pfd->fd, callback);
        poll_fds->emplace_back(*pfd);
    } else {
        // Send it to poll_ctrl
        write_int(poll_ctrl, POLL_CTRL_NEW);
        xwrite(poll_ctrl, pfd, sizeof(*pfd));
        xwrite(poll_ctrl, &callback, sizeof(callback));
    }
}

void unregister_poll(int fd, bool auto_close) {
    if (fd < 0)
        return;

    if (gettid() == getpid()) {
        // On main thread, directly modify
        poll_map->erase(fd);
        for (auto &poll_fd : *poll_fds) {
            if (poll_fd.fd == fd) {
                if (auto_close) {
                    close(poll_fd.fd);
                }
                // Cannot modify while iterating, invalidate it instead
                // It will be removed in the next poll loop
                poll_fd.fd = -1;
                break;
            }
        }
    } else {
        // Send it to poll_ctrl
        write_int(poll_ctrl, POLL_CTRL_RM);
        write_int(poll_ctrl, fd);
        write_int(poll_ctrl, auto_close);
    }
}

void clear_poll() {
    if (poll_fds) {
        for (auto &poll_fd : *poll_fds) {
            close(poll_fd.fd);
        }
    }
    delete poll_fds;
    delete poll_map;
    poll_fds = nullptr;
    poll_map = nullptr;
}

static void poll_ctrl_handler(pollfd *pfd) {
    int code = read_int(pfd->fd);
    switch (code) {
    case POLL_CTRL_NEW: {
        pollfd new_fd{};
        poll_callback cb;
        xxread(pfd->fd, &new_fd, sizeof(new_fd));
        xxread(pfd->fd, &cb, sizeof(cb));
        register_poll(&new_fd, cb);
        break;
    }
    case POLL_CTRL_RM: {
        int fd = read_int(pfd->fd);
        bool auto_close = read_int(pfd->fd);
        unregister_poll(fd, auto_close);
        break;
    }
    default:
        __builtin_unreachable();
    }
}

[[noreturn]] static void poll_loop() {
    // Register poll_ctrl
    auto pipefd = array<int, 2>{-1, -1};
    xpipe2(pipefd, O_CLOEXEC);
    poll_ctrl = pipefd[1];
    pollfd poll_ctrl_pfd = { pipefd[0], POLLIN, 0 };
    register_poll(&poll_ctrl_pfd, poll_ctrl_handler);

    for (;;) {
        if (poll(poll_fds->data(), poll_fds->size(), -1) <= 0)
            continue;

        // MUST iterate with index because any poll_callback could add new elements to poll_fds
        for (int i = 0; i < poll_fds->size();) {
            auto &pfd = (*poll_fds)[i];
            if (pfd.revents) {
                if (pfd.revents & POLLERR || pfd.revents & POLLNVAL) {
                    poll_map->erase(pfd.fd);
                    poll_fds->erase(poll_fds->begin() + i);
                    continue;
                }
                if (auto it = poll_map->find(pfd.fd); it != poll_map->end()) {
                    it->second(&pfd);
                }
            }
            ++i;
        }
    }
}

bool get_client_cred(int fd, sock_cred *cred) {
    socklen_t len = sizeof(ucred);
    if (getsockopt(fd, SOL_SOCKET, SO_PEERCRED, cred, &len) != 0)
        return false;
    char buf[4096];
    len = sizeof(buf);
    if (getsockopt(fd, SOL_SOCKET, SO_PEERSEC, buf, &len) != 0)
        len = 0;
    buf[len] = '\0';
    cred->context = buf;
    return true;
}

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

static void handle_request_async(int client, int code, const sock_cred &cred) {
    auto &daemon = MagiskD::Get();
    switch (code) {
    case +RequestCode::DENYLIST:
        denylist_handler(client, &cred);
        break;
    case +RequestCode::SUPERUSER:
        daemon.su_daemon_handler(client, cred);
        break;
    case +RequestCode::ZYGOTE_RESTART: {
        LOGI("** zygote restarted\n");
        daemon.prune_su_access();
        scan_deny_apps();
        daemon.zygisk_reset(false);
        close(client);
        break;
    }
    case +RequestCode::SQLITE_CMD:
        daemon.db_exec(client);
        break;
    case +RequestCode::REMOVE_MODULES: {
        int do_reboot = read_int(client);
        remove_modules();
        write_int(client, 0);
        close(client);
        if (do_reboot) {
            daemon.reboot();
        }
        break;
    }
    case +RequestCode::ZYGISK:
        daemon.zygisk_handler(client);
        break;
    default:
        __builtin_unreachable();
    }
}

static void handle_request_sync(int client, int code) {
    switch (code) {
    case +RequestCode::CHECK_VERSION:
#if MAGISK_DEBUG
        write_string(client, MAGISK_VERSION ":MAGISK:D");
#else
        write_string(client, MAGISK_VERSION ":MAGISK:R");
#endif
        break;
    case +RequestCode::CHECK_VERSION_CODE:
        write_int(client, MAGISK_VER_CODE);
        break;
    case +RequestCode::START_DAEMON:
        setup_logfile();
        break;
    case +RequestCode::STOP_DAEMON: {
        // Unmount all overlays
        denylist_handler(-1, nullptr);

        // Restore native bridge property
        restore_zygisk_prop();

        write_int(client, 0);

        // Terminate the daemon!
        exit(0);
    }
    default:
        __builtin_unreachable();
    }
}

static bool is_client(pid_t pid) {
    // Verify caller is the same as server
    char path[32];
    sprintf(path, "/proc/%d/exe", pid);
    struct stat st{};
    return !(stat(path, &st) || st.st_dev != self_st.st_dev || st.st_ino != self_st.st_ino);
}

static void handle_request(pollfd *pfd) {
    owned_fd client = xaccept4(pfd->fd, nullptr, nullptr, SOCK_CLOEXEC);

    // Verify client credentials
    sock_cred cred;
    bool is_root;
    bool is_zygote;
    int code;

    if (!get_client_cred(client, &cred)) {
        // Client died
        return;
    }
    is_root = cred.uid == AID_ROOT;
    is_zygote = cred.context == "u:r:zygote:s0";

    if (!is_root && !is_zygote && !is_client(cred.pid)) {
        // Unsupported client state
        write_int(client, +RespondCode::ACCESS_DENIED);
        return;
    }

    code = read_int(client);
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
        handle_request_sync(client, code);
    } else if (code < +RequestCode::_STAGE_BARRIER_) {
        exec_task([=, fd = client.release()] { handle_request_async(fd, code, cred); });
    } else {
        exec_task([=, fd = client.release()] {
            MagiskD::Get().boot_stage_handler(fd, code);
        });
    }
}

static void daemon_entry() {
    android_logging();

    // Block all signals
    sigset_t block_set;
    sigfillset(&block_set);
    pthread_sigmask(SIG_SETMASK, &block_set, nullptr);

    // Change process name
    set_nice_name("magiskd");

    int fd = xopen("/dev/null", O_WRONLY);
    xdup2(fd, STDOUT_FILENO);
    xdup2(fd, STDERR_FILENO);
    if (fd > STDERR_FILENO)
        close(fd);
    fd = xopen("/dev/zero", O_RDONLY);
    xdup2(fd, STDIN_FILENO);
    if (fd > STDERR_FILENO)
        close(fd);

    rust::daemon_entry();
    SDK_INT = MagiskD::Get().sdk_int();

    // Get self stat
    xstat("/proc/self/exe", &self_st);

    fd = xsocket(AF_LOCAL, SOCK_STREAM | SOCK_CLOEXEC, 0);
    sockaddr_un addr = {.sun_family = AF_LOCAL};
    ssprintf(addr.sun_path, sizeof(addr.sun_path), "%s/" MAIN_SOCKET, get_magisk_tmp());
    unlink(addr.sun_path);
    if (xbind(fd, (sockaddr *) &addr, sizeof(addr)))
        exit(1);
    chmod(addr.sun_path, 0666);
    setfilecon(addr.sun_path, MAGISK_FILE_CON);
    xlisten(fd, 10);

    default_new(poll_map);
    default_new(poll_fds);

    // Register handler for main socket
    pollfd main_socket_pfd = { fd, POLLIN, 0 };
    register_poll(&main_socket_pfd, handle_request);

    // Loop forever to listen for requests
    init_thread_pool();
    poll_loop();
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
        if (tmp[0] == '\0' || !str_starts(buf, tmp)) {
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

bool setup_magisk_env() {
    char buf[4096];

    LOGI("* Initializing Magisk environment\n");

    ssprintf(buf, sizeof(buf), "%s/0/%s/install", APP_DATA_DIR, JAVA_PACKAGE_NAME);
    // Alternative binaries paths
    const char *alt_bin[] = { "/cache/data_adb/magisk", "/data/magisk", buf };
    for (auto alt : alt_bin) {
        if (access(alt, F_OK) == 0) {
            rm_rf(DATABIN);
            cp_afc(alt, DATABIN);
            rm_rf(alt);
        }
    }
    rm_rf("/cache/data_adb");

    // Directories in /data/adb
    chmod(SECURE_DIR, 0700);
    xmkdir(DATABIN, 0755);
    xmkdir(MODULEROOT, 0755);
    xmkdir(SECURE_DIR "/post-fs-data.d", 0755);
    xmkdir(SECURE_DIR "/service.d", 0755);
    restorecon();

    if (access(DATABIN "/busybox", X_OK))
        return false;

    ssprintf(buf, sizeof(buf), "%s/" BBPATH "/busybox", get_magisk_tmp());
    mkdir(dirname(buf), 0755);
    cp_afc(DATABIN "/busybox", buf);
    exec_command_async(buf, "--install", "-s", dirname(buf));

    // magisk32 and magiskpolicy are not installed into ramdisk and has to be copied
    // from data to magisk tmp
    if (access(DATABIN "/magisk32", X_OK) == 0) {
        ssprintf(buf, sizeof(buf), "%s/magisk32", get_magisk_tmp());
        cp_afc(DATABIN "/magisk32", buf);
    }
    if (access(DATABIN "/magiskpolicy", X_OK) == 0) {
        ssprintf(buf, sizeof(buf), "%s/magiskpolicy", get_magisk_tmp());
        cp_afc(DATABIN "/magiskpolicy", buf);
    }

    return true;
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
    vector<int> events;
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
            events.push_back(fd);
        else
            close(fd);
    }
    if (events.empty())
        return false;

    run_finally fin([&]{ std::for_each(events.begin(), events.end(), close); });

    // Check if volume down key is held continuously for more than 3 seconds
    for (int i = 0; i < 300; ++i) {
        bool pressed = false;
        for (const int &fd : events) {
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
