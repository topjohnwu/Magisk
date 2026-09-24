module;
#include <jni.h>
#include <sys/mount.h>
#include <android/dlext.h>
#include <dlfcn.h>
#include <poll.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <rust/cxx.h>

module magisk.zygisk;

#if defined(__LP64__)
#define ZLOGD(...) LOGD("zygisk64: " __VA_ARGS__)
#define ZLOGE(...) LOGE("zygisk64: " __VA_ARGS__)
#define ZLOGI(...) LOGI("zygisk64: " __VA_ARGS__)
#define ZLOGW(...) LOGW("zygisk64: " __VA_ARGS__)
#else
#define ZLOGD(...) LOGD("zygisk32: " __VA_ARGS__)
#define ZLOGE(...) LOGE("zygisk32: " __VA_ARGS__)
#define ZLOGI(...) LOGI("zygisk32: " __VA_ARGS__)
#define ZLOGW(...) LOGW("zygisk32: " __VA_ARGS__)
#endif

// Extreme verbose logging
// #define ZLOGV(...) ZLOGD(__VA_ARGS__)
#define ZLOGV(...) (void*)0

extern "C++" {
using namespace std;

using comp_entry = void(*)(int);
extern "C" void exec_companion_entry(int, comp_entry);

static void zygiskd(int socket) {
    if (getuid() != 0 || fcntl(socket, F_GETFD) < 0)
        exit(-1);

#if defined(__LP64__)
    set_nice_name("zygiskd64");
    LOGI("* Launching zygiskd64\n");
#else
    set_nice_name("zygiskd32");
    LOGI("* Launching zygiskd32\n");
#endif

    // Load modules
    vector<comp_entry> modules;
    {
        auto module_fds = recv_fds(socket);
        for (int fd : module_fds) {
            comp_entry entry = nullptr;
            struct stat s{};
            if (fstat(fd, &s) == 0 && S_ISREG(s.st_mode)) {
                android_dlextinfo info {
                    .flags = ANDROID_DLEXT_USE_LIBRARY_FD,
                    .library_fd = fd,
                };
                if (void *h = android_dlopen_ext("/jit-cache", RTLD_LAZY, &info)) {
                    *(void **) &entry = dlsym(h, "zygisk_companion_entry");
                } else {
                    LOGW("Failed to dlopen zygisk module: %s\n", dlerror());
                }
            }
            modules.push_back(entry);
            close(fd);
        }
    }

    // ack
    write_int(socket, 0);

    // Start accepting requests
    pollfd pfd = { socket, POLLIN, 0 };
    for (;;) {
        poll(&pfd, 1, -1);
        if (pfd.revents && !(pfd.revents & POLLIN)) {
            // Something bad happened in magiskd, terminate zygiskd
            exit(0);
        }
        int client = recv_fd(socket);
        if (client < 0) {
            // Something bad happened in magiskd, terminate zygiskd
            exit(0);
        }
        int module_id = read_int(client);
        if (module_id >= 0 && module_id < modules.size() && modules[module_id]) {
            exec_companion_entry(client, modules[module_id]);
        } else {
            close(client);
        }
    }
}

// Entrypoint where we need to re-exec ourselves
// This should only ever be called internally
int zygisk_main(int argc, char *argv[]) {
    android_logging();
    if (argc == 3 && argv[1] == "companion"sv) {
        zygiskd(parse_int(argv[2]));
    }
    return 0;
}

// Entrypoint of code injection
static bool initialize_zygisk(uint32_t) {
    zygisk_logging();
    hook_entry();
    ZLOGD("load success\n");
    return false;
}

extern "C" [[maybe_unused]] NativeBridgeCallbacks NativeBridgeItf {
    .version = 2,
    .padding = {},
    .isCompatibleWith = initialize_zygisk,
};
}
