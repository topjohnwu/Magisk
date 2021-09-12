#pragma once

#include <pthread.h>
#include <string>
#include <limits>
#include <atomic>

#include <socket.hpp>

// Daemon command code flags/masks
enum : int {
    SYNC_FLAG = (1 << 30),
    DAEMON_CODE_MASK = std::numeric_limits<int>::max() >> 1
};

// Daemon command codes
enum : int {
    START_DAEMON = SYNC_FLAG | 0,
    CHECK_VERSION = SYNC_FLAG | 1,
    CHECK_VERSION_CODE = SYNC_FLAG | 2,
    GET_PATH = SYNC_FLAG | 3,
    STOP_DAEMON = SYNC_FLAG | 4,
    SUPERUSER = 5,
    POST_FS_DATA,
    LATE_START,
    BOOT_COMPLETE,
    DENYLIST,
    SQLITE_CMD,
    REMOVE_MODULES,
    ZYGISK_REQUEST,
    DAEMON_CODE_END,
};

// Return codes for daemon
enum : int {
    DAEMON_ERROR = -1,
    DAEMON_SUCCESS = 0,
    ROOT_REQUIRED,
    DAEMON_LAST
};

// Daemon state
enum : int {
    STATE_NONE,
    STATE_POST_FS_DATA,
    STATE_POST_FS_DATA_DONE,
    STATE_LATE_START_DONE,
    STATE_BOOT_COMPLETE
};

int connect_daemon(bool create = false);

extern std::atomic<int> logd_fd;
int magisk_log(int prio, const char *fmt, va_list ap);
void android_logging();

// Daemon handlers
void post_fs_data(int client);
void late_start(int client);
void boot_complete(int client);
void denylist_handler(int client, ucred *cred);
void su_daemon_handler(int client, ucred *credential);
void zygisk_handler(int client, ucred *cred);

// Denylist
void check_enforce_denylist();
int disable_deny();
int denylist_cli(int argc, char **argv);
