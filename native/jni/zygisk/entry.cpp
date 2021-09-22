#include <libgen.h>
#include <dlfcn.h>
#include <sys/mount.h>
#include <sys/sendfile.h>
#include <sys/prctl.h>
#include <android/log.h>

#include <utils.hpp>
#include <daemon.hpp>
#include <magisk.hpp>
#include <db.hpp>

#include "inject.hpp"
#include "deny/deny.hpp"

using namespace std;

static void *self_handle = nullptr;

static int zygisk_log(int prio, const char *fmt, va_list ap);

#define zlog(prio) [](auto fmt, auto ap){ return zygisk_log(ANDROID_LOG_##prio, fmt, ap); }
static void zygisk_logging() {
    log_cb.d = zlog(DEBUG);
    log_cb.i = zlog(INFO);
    log_cb.w = zlog(WARN);
    log_cb.e = zlog(ERROR);
    log_cb.ex = nop_ex;
}

void self_unload() {
    LOGD("zygisk: Request to self unload\n");
    // If unhooking failed, do not unload or else it will cause SIGSEGV
    if (!unhook_functions())
        return;
    new_daemon_thread(reinterpret_cast<thread_entry>(&dlclose), self_handle);
}

static void unload_first_stage(int, siginfo_t *info, void *) {
    auto path = static_cast<char *>(info->si_value.sival_ptr);
    unmap_all(path);
    free(path);
    struct sigaction action{};
    action.sa_handler = SIG_DFL;
    sigaction(SIGUSR1, &action, nullptr);
}

// Make sure /proc/self/environ is sanitized
// Filter env and reset MM_ENV_END
static void sanitize_environ() {
    char *cur = environ[0];

    for (int i = 0; environ[i]; ++i) {
        // Copy all env onto the original stack
        int len = strlen(environ[i]);
        memmove(cur, environ[i], len + 1);
        environ[i] = cur;
        cur += len + 1;
    }

    prctl(PR_SET_MM, PR_SET_MM_ENV_END, cur, 0, 0);
}

__attribute__((destructor))
static void zygisk_cleanup_wait() {
    // Wait 10us to make sure none of our code is executing
    timespec ts = { .tv_sec = 0, .tv_nsec = 10000L };
    nanosleep(&ts, nullptr);
}

#define SECOND_STAGE_PTR "ZYGISK_PTR"

static decltype(&unload_first_stage) second_stage_entry(void *handle) {
    self_handle = handle;
    unsetenv(INJECT_ENV_2);
    unsetenv(SECOND_STAGE_PTR);

    zygisk_logging();
    LOGD("zygisk: inject 2nd stage\n");
    hook_functions();
    return &unload_first_stage;
}

static void first_stage_entry() {
    android_logging();
    LOGD("zygisk: inject 1st stage\n");

    char *ld = getenv("LD_PRELOAD");
    char *path;
    if (char *c = strrchr(ld, ':')) {
        *c = '\0';
        setenv("LD_PRELOAD", ld, 1);  // Restore original LD_PRELOAD
        path = strdup(c + 1);
    } else {
        unsetenv("LD_PRELOAD");
        path = strdup(ld);
    }
    unsetenv(INJECT_ENV_1);
    sanitize_environ();

    // Update path to 2nd stage lib
    *(strrchr(path, '.') - 1) = '2';

    // Load second stage
    setenv(INJECT_ENV_2, "1", 1);
    void *handle = dlopen(path, RTLD_LAZY);

    // Revert path to 1st stage lib
    *(strrchr(path, '.') - 1) = '1';

    // Run second stage entry
    char *env = getenv(SECOND_STAGE_PTR);
    decltype(&second_stage_entry) second_stage;
    sscanf(env, "%p", &second_stage);
    auto unload_handler = second_stage(handle);

    // Register signal handler to unload 1st stage
    struct sigaction action{};
    action.sa_sigaction = unload_handler;
    sigaction(SIGUSR1, &action, nullptr);

    // Schedule to unload 1st stage 10us later
    timer_t timer;
    sigevent_t event{};
    event.sigev_notify = SIGEV_SIGNAL;
    event.sigev_signo = SIGUSR1;
    event.sigev_value.sival_ptr = path;
    timer_create(CLOCK_MONOTONIC, &event, &timer);
    itimerspec time{};
    time.it_value.tv_nsec = 10000L;
    timer_settime(&timer, 0, &time, nullptr);
}

__attribute__((constructor))
static void zygisk_init() {
    if (getenv(INJECT_ENV_2)) {
        // Return function pointer to first stage
        char buf[128];
        snprintf(buf, sizeof(buf), "%p", &second_stage_entry);
        setenv(SECOND_STAGE_PTR, buf, 1);
    } else if (getenv(INJECT_ENV_1)) {
        first_stage_entry();
    }
}

// Start code for magiskd IPC

int app_process_main(int argc, char *argv[]) {
    android_logging();

    if (int fd = connect_daemon(); fd >= 0) {
        write_int(fd, ZYGISK_REQUEST);
        write_int(fd, ZYGISK_SETUP);

        if (read_int(fd) == 0) {
            string path = read_string(fd);
            string lib = path + ".1.so";
            if (char *ld = getenv("LD_PRELOAD")) {
                char env[256];
                sprintf(env, "%s:%s", ld, lib.data());
                setenv("LD_PRELOAD", env, 1);
            } else {
                setenv("LD_PRELOAD", lib.data(), 1);
            }
            setenv(INJECT_ENV_1, "1", 1);
        }
        close(fd);
    }

    // Execute real app_process
    char buf[256];
    xreadlink("/proc/self/exe", buf, sizeof(buf));
    xumount2("/proc/self/exe", MNT_DETACH);
    execve(buf, argv, environ);
    return 1;
}

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

void remote_get_app_info(int uid, const char *process, AppInfo *info) {
    if (int fd = connect_daemon(); fd >= 0) {
        write_int(fd, ZYGISK_REQUEST);
        write_int(fd, ZYGISK_GET_APPINFO);

        write_int(fd, uid);
        write_string(fd, process);
        xxread(fd, info, sizeof(*info));

        close(fd);
    }
}

int remote_request_unmount() {
    if (int fd = connect_daemon(); fd >= 0) {
        write_int(fd, ZYGISK_REQUEST);
        write_int(fd, ZYGISK_UNMOUNT);
        int ret = read_int(fd);
        close(fd);
        return ret;
    }
    return DAEMON_ERROR;
}

// The following code runs in magiskd

static void setup_files(int client, ucred *cred) {
    LOGD("zygisk: setup files for pid=[%d]\n", cred->pid);

    char buf[256];
    snprintf(buf, sizeof(buf), "/proc/%d/exe", cred->pid);
    if (xreadlink(buf, buf, sizeof(buf)) < 0) {
        write_int(client, 1);
        return;
    }

    write_int(client, 0);

    string path = MAGISKTMP + "/" ZYGISKBIN "/zygisk." + basename(buf);
    cp_afc(buf, (path + ".1.so").data());
    cp_afc(buf, (path + ".2.so").data());

    write_string(client, path);
}

int cached_manager_app_id = -1;
static time_t last_modified = 0;

static void get_app_info(int client) {
    AppInfo info{};
    int uid = read_int(client);
    string process = read_string(client);

    // This function is called on every single zygote app process specialization,
    // so performance is critical. get_manager_app_id() is expensive as it goes
    // through a SQLite query and potentially multiple filesystem stats, so we
    // really want to cache the app ID value. Check the last modify timestamp of
    // packages.xml and only re-fetch the manager app ID if something changed since
    // we last checked. Granularity in seconds is good enough.
    // If denylist is enabled, inotify will invalidate the app ID cache for us.
    // In this case, we can skip the timestamp check all together.

    int manager_app_id = cached_manager_app_id;

    // Denylist not enabled, check packages.xml timestamp
    if (!denylist_enabled && manager_app_id > 0) {
        struct stat st{};
        stat("/data/system/packages.xml", &st);
        if (st.st_atim.tv_sec > last_modified) {
            manager_app_id = -1;
            last_modified = st.st_atim.tv_sec;
        }
    }

    if (manager_app_id < 0) {
        manager_app_id = get_manager_app_id();
        cached_manager_app_id = manager_app_id;
    }

    if (to_app_id(uid) == manager_app_id) {
        info.is_magisk_app = true;
    } else if (denylist_enabled) {
        info.on_denylist = is_deny_target(uid, process);
    }
    xwrite(client, &info, sizeof(info));
}

static void do_unmount(int client, ucred *cred) {
    if (denylist_enabled) {
        LOGD("zygisk: cleanup mount namespace for pid=[%d]\n", cred->pid);
        revert_daemon(cred->pid, client);
    } else {
        write_int(client, DENY_NOT_ENFORCED);
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

void zygisk_handler(int client, ucred *cred) {
    int code = read_int(client);
    switch (code) {
    case ZYGISK_SETUP:
        setup_files(client, cred);
        break;
    case ZYGISK_GET_APPINFO:
        get_app_info(client);
        break;
    case ZYGISK_UNMOUNT:
        do_unmount(client, cred);
        break;
    case ZYGISK_GET_LOG_PIPE:
        send_log_pipe(client);
        break;
    }
    close(client);
}
