#include <libgen.h>
#include <dlfcn.h>
#include <sys/mount.h>
#include <sys/sendfile.h>
#include <sys/prctl.h>

#include <utils.hpp>
#include <daemon.hpp>
#include <magisk.hpp>

#include "inject.hpp"
#include "../magiskhide/magiskhide.hpp"

using namespace std;

static void *self_handle = nullptr;
static atomic<int> active_threads = -1;

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

// Make sure /proc/self/environ does not reveal our secrets
// Copy all env to a contiguous memory and set the memory region as MM_ENV
static void sanitize_environ() {
    static string env;

    for (int i = 0; environ[i]; ++i) {
        if (str_starts(environ[i], INJECT_ENV_1 "="))
            continue;
        env += environ[i];
        env += '\0';
    }

    for (int i = 0; i < 2; ++i) {
        bool success = true;
        success &= (0 <= prctl(PR_SET_MM, PR_SET_MM_ENV_START, env.data(), 0, 0));
        success &= (0 <= prctl(PR_SET_MM, PR_SET_MM_ENV_END, env.data() + env.size(), 0, 0));
        if (success)
            break;
    }
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
        magisk_logging();
        LOGD("zygisk: inject 2nd stage\n");
        active_threads = 1;
        unsetenv(INJECT_ENV_2);

        // Get our own handle
        self_handle = dlopen(env, RTLD_LAZY);
        dlclose(self_handle);

        hook_functions();

        // Update path to 1st stage lib
        *(strrchr(env, '.') - 1) = '1';

        // Some cleanup
        sanitize_environ();
        active_threads++;
        new_daemon_thread(&unload_first_stage, env);
    } else if (getenv(INJECT_ENV_1)) {
        magisk_logging();
        LOGD("zygisk: inject 1st stage\n");

        char *ld = getenv("LD_PRELOAD");
        char *path;
        if (char *c = strrchr(ld, ':')) {
            *c = '\0';
            setenv("LD_PRELOAD", ld, 1);  // Restore original LD_PRELOAD
            path = c + 1;
        } else {
            unsetenv("LD_PRELOAD");
            path = ld;
        }

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
    magisk_logging();

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

bool remote_check_denylist(int uid, const char *process) {
    if (int fd = connect_daemon(); fd >= 0) {
        write_int(fd, ZYGISK_REQUEST);
        write_int(fd, ZYGISK_CHECK_DENYLIST);

        int ret = -1;
        if (read_int(fd) == 0) {
            write_int(fd, uid);
            write_string(fd, process);
            ret = read_int(fd);
        }
        close(fd);
        return ret >= 0 && ret;
    }
    return false;
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
    LOGD("zygisk: setup files\n");

    char buf[PATH_MAX];
    sprintf(buf, "/proc/%d/exe", cred->pid);
    if (realpath(buf, buf) == nullptr) {
        write_int(client, 1);
        return;
    }

    write_int(client, 0);

    string path = MAGISKTMP + "/zygisk." + basename(buf);
    cp_afc(buf, (path + ".1.so").data());
    cp_afc(buf, (path + ".2.so").data());

    write_string(client, path);
}

static void check_denylist(int client) {
    if (!hide_enabled()) {
        write_int(client, HIDE_NOT_ENABLED);
        return;
    }
    write_int(client, 0);
    int uid = read_int(client);
    string process = read_string(client);
    write_int(client, is_hide_target(uid, process));
}

static void do_unmount(int client, ucred *cred) {
    LOGD("zygisk: cleanup mount namespace for pid=[%d]\n", cred->pid);
    if (hide_enabled()) {
        hide_daemon(cred->pid, client);
    } else {
        write_int(client, HIDE_NOT_ENABLED);
    }
}

void zygisk_handler(int client, ucred *cred) {
    int code = read_int(client);
    switch (code) {
    case ZYGISK_SETUP:
        setup_files(client, cred);
        break;
    case ZYGISK_CHECK_DENYLIST:
        check_denylist(client);
        break;
    case ZYGISK_UNMOUNT:
        do_unmount(client, cred);
        break;
    }
    close(client);
}
