#include <sys/mount.h>
#include <android/dlext.h>
#include <dlfcn.h>
#include <poll.h>

#include <base.hpp>
#include <core.hpp>

#include "zygisk.hpp"

using namespace std;

using comp_entry = void(*)(int);
extern "C" void exec_companion_entry(int, comp_entry);

static void zygiskd(int socket) {
    if (getuid() != 0 || fcntl(socket, F_GETFD) < 0)
        exit(-1);

    // Prefer the per-install nickname the parent daemon exports via MAGISK_NICKNAME so
    // that /proc/*/comm never carries the literal "zygiskd64" / "zygiskd32" strings.
    // Fall back to the historical names only if the env var is missing (e.g. someone
    // launched the applet by hand for debugging).
#if defined(__LP64__)
    constexpr const char *fallback = "zygiskd64";
    constexpr const char *arch = "64";
#else
    constexpr const char *fallback = "zygiskd32";
    constexpr const char *arch = "32";
#endif
    const char *nick = getenv("MAGISK_NICKNAME");
    char name_buf[16] = {};
    if (nick && *nick) {
        ssprintf(name_buf, sizeof(name_buf), "%s%s", nick, arch);
        set_nice_name(name_buf);
        LOGI("* Launching %s\n", name_buf);
    } else {
        set_nice_name(fallback);
        LOGI("* Launching %s\n", fallback);
    }
    unsetenv("MAGISK_NICKNAME");

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
extern "C" [[maybe_unused]] NativeBridgeCallbacks NativeBridgeItf {
    .version = 2,
    .padding = {},
    .isCompatibleWith = [](auto) {
        zygisk_logging();
        hook_entry();
        ZLOGD("load success\n");
        return false;
    },
};
