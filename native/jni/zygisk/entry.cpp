#include <libgen.h>
#include <dlfcn.h>
#include <sys/prctl.h>
#include <android/log.h>
#include <android/dlext.h>

#include <utils.hpp>
#include <daemon.hpp>
#include <magisk.hpp>
#include <db.hpp>

#include "zygisk.hpp"
#include "module.hpp"
#include "deny/deny.hpp"

using namespace std;

void *self_handle = nullptr;

static int zygisk_log(int prio, const char *fmt, va_list ap);

#define zlog(prio) [](auto fmt, auto ap){ return zygisk_log(ANDROID_LOG_##prio, fmt, ap); }
static void zygisk_logging() {
    log_cb.d = zlog(DEBUG);
    log_cb.i = zlog(INFO);
    log_cb.w = zlog(WARN);
    log_cb.e = zlog(ERROR);
    log_cb.ex = nop_ex;
}

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

static void second_stage_entry() {
    zygisk_logging();
    ZLOGD("inject 2nd stage\n");

    char path[PATH_MAX];
    MAGISKTMP = getenv(MAGISKTMP_ENV);
    int fd = parse_int(getenv(MAGISKFD_ENV));

    snprintf(path, sizeof(path), "/proc/self/fd/%d", fd);
    xreadlink(path, path, PATH_MAX);
    android_dlextinfo info {
        .flags = ANDROID_DLEXT_USE_LIBRARY_FD,
        .library_fd = fd,
    };
    self_handle = android_dlopen_ext(path, RTLD_LAZY, &info);
    dlclose(self_handle);
    close(fd);
    unsetenv(MAGISKTMP_ENV);
    unsetenv(MAGISKFD_ENV);
    sanitize_environ();
    hook_functions();
}

static void first_stage_entry() {
    android_logging();
    ZLOGD("inject 1st stage\n");

    char path[PATH_MAX];
    char buf[256];
    char *ld = getenv("LD_PRELOAD");
    if (char *c = strrchr(ld, ':')) {
        *c = '\0';
        strlcpy(path, c + 1, sizeof(path));
        setenv("LD_PRELOAD", ld, 1);  // Restore original LD_PRELOAD
    } else {
        unsetenv("LD_PRELOAD");
        strlcpy(path, ld, sizeof(path));
    }

    // Force the linker to load the library on top of ourselves, so we do not
    // need to unmap the 1st stage library that was loaded with LD_PRELOAD.

    int fd = xopen(path, O_RDONLY | O_CLOEXEC);
    // Use fd here instead of path to make sure inode is the same as 2nd stage
    snprintf(buf, sizeof(buf), "%d", fd);
    setenv(MAGISKFD_ENV, buf, 1);
    struct stat s{};
    xfstat(fd, &s);

    android_dlextinfo info {
        .flags = ANDROID_DLEXT_FORCE_LOAD | ANDROID_DLEXT_USE_LIBRARY_FD,
        .library_fd = fd,
    };
    auto [addr, size] = find_map_range(path, s.st_ino);
    if (addr && size) {
        info.flags |= ANDROID_DLEXT_RESERVED_ADDRESS;
        info.reserved_addr = addr;
        // The existing address is guaranteed to fit, as 1st stage and 2nd stage
        // are exactly the same ELF (same inode). However, the linker could over
        // estimate the required size and refuse to dlopen. Add 2 more page_sizes
        // (one at the beginning and one at the end) as a safety measure.
        info.reserved_size = size + 2 * 4096;
    }

    setenv(INJECT_ENV_2, "1", 1);
    // Force dlopen ourselves to make ourselves dlclose-able.
    // After this call, all global variables will be reset.
    android_dlopen_ext(path, RTLD_LAZY, &info);
}

[[gnu::constructor]] [[maybe_unused]]
static void zygisk_init() {
    if (getenv(INJECT_ENV_1)) {
        unsetenv(INJECT_ENV_1);
        first_stage_entry();
    } else if (getenv(INJECT_ENV_2)) {
        unsetenv(INJECT_ENV_2);
        second_stage_entry();
    }
}

// The following code runs in zygote/app process

static int zygisk_log(int prio, const char *fmt, va_list ap) {
    // If we don't have log pipe set, ask magiskd for it
    // This could happen multiple times in zygote because it was closed to prevent crashing
    if (logd_fd < 0) {
        // Change logging temporarily to prevent infinite recursion and stack overflow
        android_logging();
        if (int fd = connect_daemon(); fd >= 0) {
            write_int(fd, ZYGISK_REQUEST);
            write_int(fd, ZYGISK_GET_LOG_PIPE);
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
    int ret = magisk_log(prio, fmt, ap);
    if (sig) {
        timespec ts{};
        sigtimedwait(&mask, nullptr, &ts);
        pthread_sigmask(SIG_SETMASK, &orig_mask, nullptr);
    }
    return ret;
}

static inline bool should_load_modules(uint32_t flags) {
    return (flags & UNMOUNT_MASK) != UNMOUNT_MASK &&
           (flags & PROCESS_IS_MAGISK_APP) != PROCESS_IS_MAGISK_APP;
}

int remote_get_info(int uid, const char *process, uint32_t *flags, vector<int> &fds) {
    if (int fd = connect_daemon(); fd >= 0) {
        write_int(fd, ZYGISK_REQUEST);
        write_int(fd, ZYGISK_GET_INFO);

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

    write_int(client, 0);
    send_fd(client, is_64_bit ? app_process_64 : app_process_32);
    write_string(client, MAGISKTMP);
}

static void magiskd_passthrough(int client) {
    bool is_64_bit = read_int(client);
    write_int(client, 0);
    send_fd(client, is_64_bit ? app_process_64 : app_process_32);
}

atomic<int> cached_manager_app_id = -1;

extern bool uid_granted_root(int uid);
static void get_process_info(int client, const sock_cred *cred) {
    int uid = read_int(client);
    string process = read_string(client);

    uint32_t flags = 0;

    // This function is called on every single zygote process specialization,
    // so performance is critical. get_manager_app_id() is expensive as it goes
    // through a SQLite query and potentially multiple filesystem stats, so we
    // really want to cache the app ID value. inotify will invalidate the app ID
    // cache for us.

    int manager_app_id = cached_manager_app_id;

    if (manager_app_id < 0) {
        manager_app_id = get_manager_app_id();
        cached_manager_app_id = manager_app_id;
    }

    if (to_app_id(uid) == manager_app_id) {
        flags |= PROCESS_IS_MAGISK_APP;
    }
    if (denylist_enforced) {
        flags |= DENYLIST_ENFORCING;
    }
    if (is_deny_target(uid, process)) {
        flags |= PROCESS_ON_DENYLIST;
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
    case ZYGISK_SETUP:
        setup_files(client, cred);
        break;
    case ZYGISK_PASSTHROUGH:
        magiskd_passthrough(client);
        break;
    case ZYGISK_GET_INFO:
        get_process_info(client, cred);
        break;
    case ZYGISK_GET_LOG_PIPE:
        send_log_pipe(client);
        break;
    case ZYGISK_CONNECT_COMPANION:
        get_exe(cred->pid, buf, sizeof(buf));
        connect_companion(client, str_ends(buf, "64"));
        break;
    case ZYGISK_GET_MODDIR:
        get_moddir(client);
        break;
    }
    close(client);
}
