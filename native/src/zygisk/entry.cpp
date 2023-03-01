#include <libgen.h>
#include <dlfcn.h>
#include <sys/prctl.h>
#include <sys/mount.h>
#include <android/log.h>
#include <android/dlext.h>

#include <base.hpp>
#include <daemon.hpp>
#include <magisk.hpp>
#include <selinux.hpp>

#include "zygisk.hpp"
#include "module.hpp"
#include "deny/deny.hpp"
#include "resetprop.hpp"

using namespace std;

void *self_handle = nullptr;

[[gnu::destructor]] [[maybe_unused]]
static void zygisk_cleanup_wait() {
    if (self_handle) {
        // Wait 10us to make sure none of our code is executing
        timespec ts = { .tv_sec = 0, .tv_nsec = 10000L };
        nanosleep(&ts, nullptr);
    }
}

extern "C" void zygisk_inject_entry(void *handle) {
    self_handle = handle;
    // we need to check if we are running in zygote process
    // if not, we still need to load the original native bridge if exists
    bool zygote = false;
    if (auto fp = open_file("/proc/self/attr/current", "r")) {
        char buf[16];
        fscanf(fp.get(), "%15s", buf);
        zygote = (buf == "u:r:zygote:s0"sv && getppid() == 1 && getuid() == 0);
    }

    if (zygote) {
        zygisk_logging();
    } else {
        android_logging();
    }
    ZLOGD("load success\n");

    hook_functions(zygote);
}

// The following code runs in zygote/app process

extern "C" void zygisk_log_write(int prio, const char *msg, int len) {
    // If we don't have the log pipe set, request magiskd for it. This could actually happen
    // multiple times in the zygote daemon (parent process) because we had to close this
    // file descriptor to prevent crashing.
    //
    // For some reason, zygote sanitizes and checks FDs *before* forking. This results in the fact
    // that *every* time before zygote forks, it has to close all logging related FDs in order
    // to pass FD checks, just to have it re-initialized immediately after any
    // logging happens ¯\_(ツ)_/¯.
    //
    // To be consistent with this behavior, we also have to close the log pipe to magiskd
    // to make zygote NOT crash if necessary. For nativeForkAndSpecialize, we can actually
    // add this FD into fds_to_ignore to pass the check. For other cases, we accomplish this by
    // hooking __android_log_close and closing it at the same time as the rest of logging FDs.

    if (logd_fd < 0) {
        android_logging();
        if (int fd = zygisk_request(ZygiskRequest::GET_LOG_PIPE); fd >= 0) {
            int log_pipe = -1;
            if (read_int(fd) == 0) {
                log_pipe = recv_fd(fd);
            }
            close(fd);
            if (log_pipe >= 0) {
                // Only re-enable zygisk logging if possible
                logd_fd = log_pipe;
                zygisk_logging();
            }
        } else {
            return;
        }
    }

    // Block SIGPIPE
    sigset_t mask;
    sigset_t orig_mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGPIPE);
    pthread_sigmask(SIG_BLOCK, &mask, &orig_mask);

    magisk_log_write(prio, msg, len);

    // Consume SIGPIPE if exists, then restore mask
    timespec ts{};
    sigtimedwait(&mask, nullptr, &ts);
    pthread_sigmask(SIG_SETMASK, &orig_mask, nullptr);
}

static inline bool should_load_modules(uint32_t flags) {
    return (flags & UNMOUNT_MASK) != UNMOUNT_MASK &&
           (flags & PROCESS_IS_MAGISK_APP) != PROCESS_IS_MAGISK_APP;
}

int remote_get_info(int uid, const char *process, const char *app_data_dir, uint32_t *flags, vector<int> &fds) {
    if (int fd = zygisk_request(ZygiskRequest::GET_INFO); fd >= 0) {
        write_int(fd, uid);
        write_string(fd, process);
        write_string(fd, app_data_dir);
        xxread(fd, flags, sizeof(*flags));
        if (should_load_modules(*flags)) {
            fds = recv_fds(fd);
        }
        return fd;
    }
    return -1;
}

// The following code runs in magiskd

static vector<int> get_module_fds(bool is_64_bit) {
    vector<int> fds;
    // All fds passed to send_fds have to be valid file descriptors.
    // To workaround this issue, send over STDOUT_FILENO as an indicator of an
    // invalid fd as it will always be /dev/null in magiskd
    if (is_64_bit) {
#if defined(__LP64__)
        std::transform(module_list->begin(), module_list->end(), std::back_inserter(fds),
            [](const module_info &info) { return info.z64 < 0 ? STDOUT_FILENO : info.z64; });
#endif
    } else {
        std::transform(module_list->begin(), module_list->end(), std::back_inserter(fds),
            [](const module_info &info) { return info.z32 < 0 ? STDOUT_FILENO : info.z32; });
    }
    return fds;
}

static bool get_exe(int pid, char *buf, size_t sz) {
    char exe[128];
    if (ssprintf(exe, sizeof(exe), "/proc/%d/exe", pid) < 0)
        return false;
    return xreadlink(exe, buf, sz) > 0;
}

static pthread_mutex_t zygiskd_lock = PTHREAD_MUTEX_INITIALIZER;
static int zygiskd_sockets[] = { -1, -1 };
#define zygiskd_socket zygiskd_sockets[is_64_bit]

static void connect_companion(int client, bool is_64_bit) {
    mutex_guard g(zygiskd_lock);

    if (zygiskd_socket >= 0) {
        // Make sure the socket is still valid
        pollfd pfd = { zygiskd_socket, 0, 0 };
        poll(&pfd, 1, 0);
        if (pfd.revents) {
            // Any revent means error
            close(zygiskd_socket);
            zygiskd_socket = -1;
        }
    }
    if (zygiskd_socket < 0) {
        int fds[2];
        socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, fds);
        zygiskd_socket = fds[0];
        if (fork_dont_care() == 0) {
            string exe = "/system/bin/magisk"s + (is_64_bit ? "64" : "32");
            // This fd has to survive exec
            fcntl(fds[1], F_SETFD, 0);
            char buf[16];
            ssprintf(buf, sizeof(buf), "%d", fds[1]);
            execl(exe.data(), "", "zygisk", "companion", buf, (char *) nullptr);
            exit(-1);
        }
        close(fds[1]);
        vector<int> module_fds = get_module_fds(is_64_bit);
        send_fds(zygiskd_socket, module_fds.data(), module_fds.size());
        // Wait for ack
        if (read_int(zygiskd_socket) != 0) {
            LOGE("zygiskd startup error\n");
            return;
        }
    }
    send_fd(zygiskd_socket, client);
}

extern bool uid_granted_root(int uid);
static void get_process_info(int client, const sock_cred *cred) {
    int uid = read_int(client);
    string process = read_string(client);
    string app_data_dir = read_string(client);

    uint32_t flags = 0;

    check_pkg_refresh();
    if (is_deny_target(uid, process)) {
        flags |= PROCESS_ON_DENYLIST;
    }
    int manager_app_id = get_manager();
    if (to_app_id(uid) == manager_app_id) {
        flags |= PROCESS_IS_MAGISK_APP;
    }
    if (denylist_enforced) {
        flags |= DENYLIST_ENFORCING;
    }
    if (uid_granted_root(uid)) {
        flags |= PROCESS_GRANTED_ROOT;
    }
    // We assume all zygotes have launched when SystemUI is launching
    // So it's safe to reset native bridge at this time
    if (getprop(NBPROP) == ZYGISKLDR && app_data_dir.ends_with("/com.android.systemui")) {
        setprop(NBPROP, "0");
    }
    xwrite(client, &flags, sizeof(flags));

    if (should_load_modules(flags)) {
        char buf[256];
        if (!get_exe(cred->pid, buf, sizeof(buf))) {
            LOGW("zygisk: remote process %d probably died, abort\n", cred->pid);
            send_fd(client, -1);
            return;
        }
        vector<int> fds = get_module_fds(str_ends(buf, "64"));
        send_fds(client, fds.data(), fds.size());
    }

    if (uid != 1000 || process != "system_server")
        return;

    // Collect module status from system_server
    int slots = read_int(client);
    dynamic_bitset bits;
    for (int i = 0; i < slots; ++i) {
        dynamic_bitset::slot_type l = 0;
        xxread(client, &l, sizeof(l));
        bits.emplace_back(l);
    }
    for (int id = 0; id < module_list->size(); ++id) {
        if (!as_const(bits)[id]) {
            // Either not a zygisk module, or incompatible
            char buf[4096];
            ssprintf(buf, sizeof(buf), MODULEROOT "/%s/zygisk",
                module_list->operator[](id).name.data());
            if (int dirfd = open(buf, O_RDONLY | O_CLOEXEC); dirfd >= 0) {
                close(xopenat(dirfd, "unloaded", O_CREAT | O_RDONLY, 0644));
                close(dirfd);
            }
        }
    }
}

static void send_log_pipe(int fd) {
    // There is race condition here, but we can't really do much about it...
    if (logd_fd >= 0) {
        write_int(fd, 0);
        send_fd(fd, logd_fd);
    } else {
        write_int(fd, 1);
    }
}

static void get_moddir(int client) {
    int id = read_int(client);
    char buf[4096];
    ssprintf(buf, sizeof(buf), MODULEROOT "/%s", module_list->operator[](id).name.data());
    int dfd = xopen(buf, O_RDONLY | O_CLOEXEC);
    send_fd(client, dfd);
    close(dfd);
}

void zygisk_handler(int client, const sock_cred *cred) {
    int code = read_int(client);
    char buf[256];
    switch (code) {
    case ZygiskRequest::GET_INFO:
        get_process_info(client, cred);
        break;
    case ZygiskRequest::GET_LOG_PIPE:
        send_log_pipe(client);
        break;
    case ZygiskRequest::CONNECT_COMPANION:
        if (get_exe(cred->pid, buf, sizeof(buf))) {
            connect_companion(client, str_ends(buf, "64"));
        } else {
            LOGW("zygisk: remote process %d probably died, abort\n", cred->pid);
        }
        break;
    case ZygiskRequest::GET_MODDIR:
        get_moddir(client);
        break;
    default:
        // Unknown code
        break;
    }
    close(client);
}
