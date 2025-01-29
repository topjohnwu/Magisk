#include <libgen.h>
#include <dlfcn.h>
#include <sys/prctl.h>
#include <sys/mount.h>
#include <android/log.h>
#include <android/dlext.h>

#include <base.hpp>
#include <consts.hpp>

#include "zygisk.hpp"
#include "module.hpp"

using namespace std;

string native_bridge = "0";

static bool is_compatible_with(uint32_t) {
    zygisk_logging();
    hook_entry();
    ZLOGD("load success\n");
    return false;
}

extern "C" [[maybe_unused]] NativeBridgeCallbacks NativeBridgeItf {
    .version = 2,
    .padding = {},
    .isCompatibleWith = &is_compatible_with,
};

// The following code runs in magiskd

static pthread_mutex_t zygiskd_lock = PTHREAD_MUTEX_INITIALIZER;
static int zygiskd_sockets[] = { -1, -1 };
#define zygiskd_socket zygiskd_sockets[is_64_bit]

void MagiskD::connect_zygiskd(int client) const noexcept {
    mutex_guard g(zygiskd_lock);

    bool is_64_bit = read_int(client);
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
            char exe[64];
#if defined(__LP64__)
            ssprintf(exe, sizeof(exe), "%s/magisk%s", get_magisk_tmp(), (is_64_bit ? "" : "32"));
#else
            ssprintf(exe, sizeof(exe), "%s/magisk", get_magisk_tmp());
#endif
            // This fd has to survive exec
            fcntl(fds[1], F_SETFD, 0);
            char buf[16];
            ssprintf(buf, sizeof(buf), "%d", fds[1]);
            execl(exe, "", "zygisk", "companion", buf, (char *) nullptr);
            exit(-1);
        }
        close(fds[1]);
        rust::Vec<int> module_fds = get_module_fds(is_64_bit);
        send_fds(zygiskd_socket, module_fds.data(), module_fds.size());
        // Wait for ack
        if (read_int(zygiskd_socket) != 0) {
            LOGE("zygiskd startup error\n");
            return;
        }
    }
    send_fd(zygiskd_socket, client);
}

void reset_zygisk(bool restore) {
    if (!zygisk_enabled) return;
    static atomic_uint zygote_start_count{1};
    if (!restore) {
        close(zygiskd_sockets[0]);
        close(zygiskd_sockets[1]);
        zygiskd_sockets[0] = zygiskd_sockets[1] = -1;
    }
    if (restore) {
        zygote_start_count = 1;
    } else if (zygote_start_count.fetch_add(1) > 3) {
        LOGW("zygote crashes too many times, rolling-back\n");
        restore = true;
    }
    if (restore) {
        string native_bridge_orig = "0";
        if (native_bridge.length() > strlen(ZYGISKLDR)) {
            native_bridge_orig = native_bridge.substr(strlen(ZYGISKLDR));
        }
        set_prop(NBPROP, native_bridge_orig.data());
    } else {
        set_prop(NBPROP, native_bridge.data());
    }
}
