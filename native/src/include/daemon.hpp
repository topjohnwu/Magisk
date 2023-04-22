#pragma once

#include <pthread.h>
#include <poll.h>
#include <string>
#include <limits>
#include <atomic>
#include <functional>

#include <socket.hpp>
#include <core-rs.hpp>

#define AID_ROOT   0
#define AID_SHELL  2000
#define AID_APP_START 10000
#define AID_APP_END 19999
#define AID_USER_OFFSET 100000

#define to_app_id(uid)  (uid % AID_USER_OFFSET)
#define to_user_id(uid) (uid / AID_USER_OFFSET)

// Daemon command codes
namespace MainRequest {
enum : int {
    START_DAEMON,
    CHECK_VERSION,
    CHECK_VERSION_CODE,
    GET_PATH,
    STOP_DAEMON,

    _SYNC_BARRIER_,

    SUPERUSER,
    ZYGOTE_RESTART,
    DENYLIST,
    SQLITE_CMD,
    REMOVE_MODULES,
    ZYGISK,
    ZYGISK_PASSTHROUGH,

    _STAGE_BARRIER_,

    POST_FS_DATA,
    LATE_START,
    BOOT_COMPLETE,

    END,
};
}

// Return codes for daemon
namespace MainResponse {
enum : int {
    ERROR = -1,
    OK = 0,
    ROOT_REQUIRED,
    ACCESS_DENIED,
    END
};
}

struct module_info {
    std::string name;
    int z32 = -1;
#if defined(__LP64__)
    int z64 = -1;
#endif
};

extern bool zygisk_enabled;
extern int app_process_32;
extern int app_process_64;
extern std::vector<module_info> *module_list;

int connect_daemon(int req, bool create = false);

// Poll control
using poll_callback = void(*)(pollfd*);
void register_poll(const pollfd *pfd, poll_callback callback);
void unregister_poll(int fd, bool auto_close);
void clear_poll();

// Thread pool
void exec_task(std::function<void()> &&task);

// Logging
extern std::atomic<int> logd_fd;
extern "C" void magisk_log_write(int prio, const char *msg, int len);

// Daemon handlers
void boot_stage_handler(int client, int code);
void denylist_handler(int client, const sock_cred *cred);
void su_daemon_handler(int client, const sock_cred *cred);
void zygisk_handler(int client, const sock_cred *cred);

// Package
void preserve_stub_apk();
void check_pkg_refresh();
std::vector<bool> get_app_no_list();
// Call check_pkg_refresh() before calling get_manager(...)
// to make sure the package state is invalidated!
int get_manager(int user_id = 0, std::string *pkg = nullptr, bool install = false);
void prune_su_access();

// Denylist
extern std::atomic_flag skip_pkg_rescan;
void initialize_denylist();
int denylist_cli(int argc, char **argv);
