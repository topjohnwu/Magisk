#pragma once

#include <pthread.h>
#include <string>

#include <socket.hpp>

// Daemon command codes
enum {
    START_DAEMON,
    SUPERUSER,
    CHECK_VERSION,
    CHECK_VERSION_CODE,
    POST_FS_DATA,
    LATE_START,
    BOOT_COMPLETE,
    MAGISKHIDE,
    SQLITE_CMD,
    REMOVE_MODULES,
    GET_PATH,
    DAEMON_CODE_END,
};

// Return codes for daemon
enum {
    DAEMON_ERROR = -1,
    DAEMON_SUCCESS = 0,
    ROOT_REQUIRED,
    DAEMON_LAST
};

// Daemon state
enum {
    STATE_NONE,
    STATE_POST_FS_DATA,
    STATE_POST_FS_DATA_DONE,
    STATE_LATE_START_DONE,
    STATE_BOOT_COMPLETE
};

int connect_daemon(bool create = false);

// Daemon handlers
void post_fs_data(int client);
void late_start(int client);
void boot_complete(int client);
void magiskhide_handler(int client, ucred *cred);
void su_daemon_handler(int client, ucred *credential);

// MagiskHide
void auto_start_magiskhide(bool late_props);
int stop_magiskhide();

#if ENABLE_INJECT
// For injected process to access daemon
int remote_check_hide(int uid, const char *process);
void remote_request_hide();
#endif
