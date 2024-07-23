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

// The following code runs in zygote/app process

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
#if defined(__LP64__)
    if (is_64_bit) {
        std::transform(module_list->begin(), module_list->end(), std::back_inserter(fds),
            [](const module_info &info) { return info.z64 < 0 ? STDOUT_FILENO : info.z64; });
    } else
#endif
    {
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

    uint32_t flags = 0;

    if (is_deny_target(uid, process)) {
        flags |= PROCESS_ON_DENYLIST;
    }
    if (get_manager(to_user_id(uid)) == uid) {
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
