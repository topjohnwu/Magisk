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
static atomic<int> active_threads = -1;

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
    // If deny failed, do not unload or else it will cause SIGSEGV
    if (!unhook_functions())
        return;
    new_daemon_thread(reinterpret_cast<thread_entry>(&dlclose), self_handle);
    active_threads--;
}

static void *unload_first_stage(void *v) {
    // Setup 1ms
    timespec ts = { .tv_sec = 0, .tv_nsec = 1000000L };

    while (getenv(INJECT_ENV_1))
        nanosleep(&ts, nullptr);

    // Wait another 1ms to make sure all threads left our code
    nanosleep(&ts, nullptr);

    char *path = static_cast<char *>(v);
    unmap_all(path);
    active_threads--;
    return nullptr;
}

// Make sure /proc/self/environ is sanitized
// Filter env and reset MM_ENV_END
static void sanitize_environ() {
    char *cur = environ[0];

    for (int i = 0; environ[i]; ++i) {
        if (str_starts(environ[i], INJECT_ENV_1 "=")) {
            // This specific env has to live in heap
            environ[i] = strdup(environ[i]);
            continue;
        }

        // Copy all filtered env onto the original stack
        int len = strlen(environ[i]);
        memmove(cur, environ[i], len + 1);
        environ[i] = cur;
        cur += len + 1;
    }

    prctl(PR_SET_MM, PR_SET_MM_ENV_END, cur, 0, 0);
}

__attribute__((destructor))
static void inject_cleanup_wait() {
    if (active_threads < 0)
        return;

    // Setup 1ms
    timespec ts = { .tv_sec = 0, .tv_nsec = 1000000L };

    // Check flag in busy loop
    while (active_threads)
        nanosleep(&ts, nullptr);

    // Wait another 1ms to make sure all threads left our code
    nanosleep(&ts, nullptr);
}

__attribute__((constructor))
static void inject_init() {
    if (char *env = getenv(INJECT_ENV_2)) {
        zygisk_logging();
        LOGD("zygisk: inject 2nd stage\n");
        active_threads = 1;
        unsetenv(INJECT_ENV_2);

        // Get our own handle
        self_handle = dlopen(env, RTLD_LAZY);
        dlclose(self_handle);

        hook_functions();

        // Update path to 1st stage lib
        *(strrchr(env, '.') - 1) = '1';

        active_threads++;
        new_daemon_thread(&unload_first_stage, env);
    } else if (getenv(INJECT_ENV_1)) {
        android_logging();
        LOGD("zygisk: inject 1st stage\n");

        string ld = getenv("LD_PRELOAD");
        char *path;
        if (char *c = strrchr(ld.data(), ':')) {
            *c = '\0';
            setenv("LD_PRELOAD", ld.data(), 1);  // Restore original LD_PRELOAD
            path = c + 1;
        } else {
            unsetenv("LD_PRELOAD");
            path = ld.data();
        }
        sanitize_environ();

        // Update path to 2nd stage lib
        *(strrchr(path, '.') - 1) = '2';

        // Setup second stage
        setenv(INJECT_ENV_2, path, 1);
        dlopen(path, RTLD_LAZY);

        unsetenv(INJECT_ENV_1);
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

static void get_app_info(int client) {
    AppInfo info{};
    int uid = read_int(client);
    string process = read_string(client);
    if (to_app_id(uid) == get_manager_app_id()) {
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
