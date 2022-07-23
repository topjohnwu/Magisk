#include <libgen.h>
#include <dlfcn.h>
#include <sys/prctl.h>
#include <sys/mount.h>
#include <android/log.h>
#include <android/dlext.h>

#include <base.hpp>
#include <daemon.hpp>
#include <magisk.hpp>

#include "zygisk.hpp"
#include "module.hpp"
#include "deny/deny.hpp"

using namespace std;

void *self_handle = nullptr;

// Make sure /proc/self/environ is sanitized
// Filter env and reset MM_ENV_END
static void sanitize_environ() {
    char *cur = environ[0];

    for (int i = 0; environ[i]; ++i) {
        // Copy all env onto the original stack
        size_t len = strlen(environ[i]);
        memmove(cur, environ[i], len + 1);
        environ[i] = cur;
        cur += len + 1;
    }

    prctl(PR_SET_MM, PR_SET_MM_ENV_END, cur, 0, 0);
}

[[gnu::destructor]] [[maybe_unused]]
static void zygisk_cleanup_wait() {
    if (self_handle) {
        // Wait 10us to make sure none of our code is executing
        timespec ts = { .tv_sec = 0, .tv_nsec = 10000L };
        nanosleep(&ts, nullptr);
    }
}

static void *unload_first_stage(void *) {
    // Wait 10us to make sure 1st stage is done
    timespec ts = { .tv_sec = 0, .tv_nsec = 10000L };
    nanosleep(&ts, nullptr);
    unmap_all(HIJACK_BIN);
    xumount2(HIJACK_BIN, MNT_DETACH);
    return nullptr;
}

static void second_stage_entry() {
    zygisk_logging();
    ZLOGD("inject 2nd stage\n");

    MAGISKTMP = getenv(MAGISKTMP_ENV);
#if defined(__LP64__)
    self_handle = dlopen("/system/bin/app_process", RTLD_NOLOAD);
#else
    self_handle = dlopen("/system/bin/app_process32", RTLD_NOLOAD);
#endif
    dlclose(self_handle);

    unsetenv(MAGISKTMP_ENV);
    sanitize_environ();
    hook_functions();
    new_daemon_thread(&unload_first_stage, nullptr);
}

static void first_stage_entry() {
    ZLOGD("inject 1st stage\n");

    char *ld = getenv("LD_PRELOAD");
    if (char *c = strrchr(ld, ':')) {
        *c = '\0';
        setenv("LD_PRELOAD", ld, 1);  // Restore original LD_PRELOAD
    } else {
        unsetenv("LD_PRELOAD");
    }

    // Load second stage
    setenv(INJECT_ENV_2, "1", 1);
#if defined(__LP64__)
    dlopen("/system/bin/app_process", RTLD_LAZY);
#else
    dlopen("/system/bin/app_process32", RTLD_LAZY);
#endif
}

[[gnu::constructor]] [[maybe_unused]]
static void zygisk_init() {
    android_logging();
    if (getenv(INJECT_ENV_1)) {
        unsetenv(INJECT_ENV_1);
        first_stage_entry();
    } else if (getenv(INJECT_ENV_2)) {
        unsetenv(INJECT_ENV_2);
        second_stage_entry();
    }
}

// The following code runs in zygote/app process

extern "C" void zygisk_log_write(int prio, const char *msg, int len) {
    // If we don't have log pipe set, ask magiskd for it
    // This could happen multiple times in zygote because it was closed to prevent crashing
    if (logd_fd < 0) {
        // Change logging temporarily to prevent infinite recursion and stack overflow
        android_logging();
        if (int fd = zygisk_request(ZygiskRequest::GET_LOG_PIPE); fd >= 0) {
            if (read_int(fd) == 0) {
                logd_fd = recv_fd(fd);
            }
            close(fd);
        }
        zygisk_logging();
    }

    sigset_t mask;
    sigset_t orig_mask;
    bool sig = false;
    // Make sure SIGPIPE won't crash zygote
    if (logd_fd >= 0) {
        sig = true;
        sigemptyset(&mask);
        sigaddset(&mask, SIGPIPE);
        pthread_sigmask(SIG_BLOCK, &mask, &orig_mask);
    }
    magisk_log_write(prio, msg, len);
    if (sig) {
        timespec ts{};
        sigtimedwait(&mask, nullptr, &ts);
        pthread_sigmask(SIG_SETMASK, &orig_mask, nullptr);
    }
}

static inline bool should_load_modules(uint32_t flags) {
    return (flags & UNMOUNT_MASK) != UNMOUNT_MASK &&
           (flags & PROCESS_IS_MAGISK_APP) != PROCESS_IS_MAGISK_APP;
}

int remote_get_info(int uid, const char *process, uint32_t *flags, vector<int> &fds) {
    if (int fd = zygisk_request(ZygiskRequest::GET_INFO); fd >= 0) {
        write_int(fd, uid);
        write_string(fd, process);
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
    snprintf(buf, sz, "/proc/%d/exe", pid);
    return xreadlink(buf, buf, sz) > 0;
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
            string exe = MAGISKTMP + "/magisk" + (is_64_bit ? "64" : "32");
            // This fd has to survive exec
            fcntl(fds[1], F_SETFD, 0);
            char buf[16];
            snprintf(buf, sizeof(buf), "%d", fds[1]);
            execl(exe.data(), "zygisk", "companion", buf, (char *) nullptr);
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

static timespec last_zygote_start;
static int zygote_start_counts[] = { 0, 0 };
#define zygote_start_count zygote_start_counts[is_64_bit]
#define zygote_started (zygote_start_counts[0] + zygote_start_counts[1])
#define zygote_start_reset(val) { zygote_start_counts[0] = val; zygote_start_counts[1] = val; }

static void setup_files(int client, const sock_cred *cred) {
    LOGD("zygisk: setup files for pid=[%d]\n", cred->pid);

    char buf[256];
    if (!get_exe(cred->pid, buf, sizeof(buf))) {
        write_int(client, 1);
        return;
    }

    bool is_64_bit = str_ends(buf, "64");

    if (!zygote_started) {
        // First zygote launch, record time
        clock_gettime(CLOCK_MONOTONIC, &last_zygote_start);
    }

    if (zygote_start_count) {
        // This zygote ABI had started before, kill existing zygiskd
        close(zygiskd_sockets[0]);
        close(zygiskd_sockets[1]);
        zygiskd_sockets[0] = -1;
        zygiskd_sockets[1] = -1;
    }
    ++zygote_start_count;

    if (zygote_start_count >= 5) {
        // Bootloop prevention
        timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        if (ts.tv_sec - last_zygote_start.tv_sec > 60) {
            // This is very likely manual soft reboot
            memcpy(&last_zygote_start, &ts, sizeof(ts));
            zygote_start_reset(1);
        } else {
            // If any zygote relaunched more than 5 times within a minute,
            // don't do any setups further to prevent bootloop.
            zygote_start_reset(999);
            write_int(client, 1);
            return;
        }
    }

    // Hijack some binary in /system/bin to host 1st stage
    const char *hbin;
    string mbin;
    int app_fd;
    if (is_64_bit) {
        hbin = HIJACK_BIN64;
        mbin = MAGISKTMP + "/" ZYGISKBIN "/magisk64";
        app_fd = app_process_64;
    } else {
        hbin = HIJACK_BIN32;
        mbin = MAGISKTMP + "/" ZYGISKBIN "/magisk32";
        app_fd = app_process_32;
    }
    xmount(mbin.data(), hbin, nullptr, MS_BIND, nullptr);

    write_int(client, 0);
    send_fd(client, app_fd);
    write_string(client, MAGISKTMP);
}

static void magiskd_passthrough(int client) {
    bool is_64_bit = read_int(client);
    write_int(client, 0);
    send_fd(client, is_64_bit ? app_process_64 : app_process_32);
}

extern bool uid_granted_root(int uid);
static void get_process_info(int client, const sock_cred *cred) {
    int uid = read_int(client);
    string process = read_string(client);

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

    xwrite(client, &flags, sizeof(flags));

    if (should_load_modules(flags)) {
        char buf[256];
        get_exe(cred->pid, buf, sizeof(buf));
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
            snprintf(buf, sizeof(buf), MODULEROOT "/%s/zygisk",
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
    snprintf(buf, sizeof(buf), MODULEROOT "/%s", module_list->operator[](id).name.data());
    int dfd = xopen(buf, O_RDONLY | O_CLOEXEC);
    send_fd(client, dfd);
    close(dfd);
}

void zygisk_handler(int client, const sock_cred *cred) {
    int code = read_int(client);
    char buf[256];
    switch (code) {
    case ZygiskRequest::SETUP:
        setup_files(client, cred);
        break;
    case ZygiskRequest::PASSTHROUGH:
        magiskd_passthrough(client);
        break;
    case ZygiskRequest::GET_INFO:
        get_process_info(client, cred);
        break;
    case ZygiskRequest::GET_LOG_PIPE:
        send_log_pipe(client);
        break;
    case ZygiskRequest::CONNECT_COMPANION:
        get_exe(cred->pid, buf, sizeof(buf));
        connect_companion(client, str_ends(buf, "64"));
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
