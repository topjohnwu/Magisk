#pragma once

#include <pthread.h>
#include <poll.h>
#include <string>
#include <limits>
#include <atomic>
#include <functional>

#include <socket.hpp>

// Daemon command codes
enum class DaemonRequest: int {
    START_DAEMON,
    CHECK_VERSION,
    CHECK_VERSION_CODE,
    GET_PATH,
    STOP_DAEMON,

    _SYNC_BARRIER_,

    SUPERUSER,
    POST_FS_DATA,
    LATE_START,
    BOOT_COMPLETE,
    DENYLIST,
    SQLITE_CMD,
    REMOVE_MODULES,
    ZYGISK_REQUEST,
    END,
};

// Return codes for daemon
enum class DaemonResponse: int {
    ERROR = -1,
    OK = 0,
    ROOT_REQUIRED,
    INVALID_REQUEST,
    END
};

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

int connect_daemon(DaemonRequest req, bool create = false);

// Poll control
using poll_callback = void(*)(pollfd*);
void register_poll(const pollfd *pfd, poll_callback callback);
void unregister_poll(int fd, bool auto_close);
void clear_poll();

// Thread pool
void exec_task(std::function<void()> &&task);

// Logging
extern std::atomic<int> logd_fd;
int magisk_log(int prio, const char *fmt, va_list ap);
void android_logging();

// Daemon handlers
void post_fs_data(int client);
void late_start(int client);
void boot_complete(int client);
void denylist_handler(int client, const sock_cred *cred);
void su_daemon_handler(int client, const sock_cred *cred);
void zygisk_handler(int client, const sock_cred *cred);

// Denylist
void initialize_denylist();
int denylist_cli(int argc, char **argv);
