#include <libgen.h>
#include <dlfcn.h>
#include <sys/mount.h>
#include <sys/sendfile.h>
#include <android/log.h>
#include <atomic>

#include <utils.hpp>

#include "inject.hpp"

using namespace std;

static void *self_handle = nullptr;
static atomic<int> active_threads = -1;

#define alog(prio) [](auto fmt, auto ap){ \
return __android_log_vprint(ANDROID_LOG_##prio, "Magisk", fmt, ap); }
static void inject_logging() {
    log_cb.d = alog(DEBUG);
    log_cb.i = alog(INFO);
    log_cb.w = alog(WARN);
    log_cb.e = alog(ERROR);
    log_cb.ex = nop_ex;
}

__attribute__((destructor))
static void inject_cleanup() {
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

static inline void self_unload() {
    new_daemon_thread(reinterpret_cast<void *(*)(void *)>(&dlclose), self_handle);
    active_threads--;
}

static void *unload_first_stage(void *) {
    // Setup 1ms
    timespec ts = { .tv_sec = 0, .tv_nsec = 1000000L };

    while (getenv(INJECT_ENV_2))
        nanosleep(&ts, nullptr);

    // Wait another 1ms to make sure all threads left our code
    nanosleep(&ts, nullptr);

    unmap_all(INJECT_LIB_1);
    active_threads--;
    return nullptr;
}

__attribute__((constructor))
static void inject_init() {
    inject_logging();
    if (char *env = getenv(INJECT_ENV_1)) {
        LOGD("zygote: inject 1st stage\n");

        if (env[0] == '1')
            unsetenv("LD_PRELOAD");
        else
            setenv("LD_PRELOAD", env, 1);  // Restore original LD_PRELOAD
        unsetenv(INJECT_ENV_1);

        // Setup second stage
        setenv(INJECT_ENV_2, "1", 1);
        cp_afc(INJECT_LIB_1, INJECT_LIB_2);
        dlopen(INJECT_LIB_2, RTLD_LAZY);
    } else if (getenv(INJECT_ENV_2)) {
        LOGD("zygote: inject 2nd stage\n");

        active_threads = 1;

        // Get our own handle
        self_handle = dlopen(INJECT_LIB_2, RTLD_LAZY);
        dlclose(self_handle);

        // Cleanup 1st stage maps
        active_threads++;
        new_daemon_thread(&unload_first_stage);
        unsetenv(INJECT_ENV_2);

        // TODO: actually inject stuffs, for now we demonstrate clean self unloading
        self_unload();
    }
}

int app_process_main(int argc, char *argv[]) {
    inject_logging();
    char buf[4096];
    if (realpath("/proc/self/exe", buf) == nullptr)
        return 1;

    int in = xopen(buf, O_RDONLY);
    int out = xopen(INJECT_LIB_1, O_CREAT | O_WRONLY | O_TRUNC, 0777);
    sendfile(out, in, nullptr, INT_MAX);
    close(in);
    close(out);

    if (char *ld = getenv("LD_PRELOAD")) {
        char env[128];
        sprintf(env, "%s:" INJECT_LIB_1, ld);
        setenv("LD_PRELOAD", env, 1);
        setenv(INJECT_ENV_1, ld, 1);  // Backup original LD_PRELOAD
    } else {
        setenv("LD_PRELOAD", INJECT_LIB_1, 1);
        setenv(INJECT_ENV_1, "1", 1);
    }

    // Execute real app_process
    xumount2(buf, MNT_DETACH);
    execve(buf, argv, environ);
    exit(1);
}
