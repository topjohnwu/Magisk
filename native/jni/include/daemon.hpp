#pragma once

#include <pthread.h>
#include <string>
#include <vector>

#include <socket.hpp>

// Daemon command codes
enum {
	DO_NOTHING = 0,
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
	STATE_POST_FS_DATA,
	STATE_LATE_START,
	STATE_BOOT_COMPLETE,
	STATE_UNKNOWN
};

extern int SDK_INT;
extern bool RECOVERY_MODE;
extern int DAEMON_STATE;
#define APP_DATA_DIR (SDK_INT >= 24 ? "/data/user_de" : "/data/user")

// Daemon handlers
void post_fs_data(int client);
void late_start(int client);
void boot_complete(int client);
void magiskhide_handler(int client);
void su_daemon_handler(int client, ucred *credential);

// Misc
int connect_daemon(bool create = false);
void unlock_blocks();
void reboot();

// Module stuffs
void handle_modules();
void magic_mount();
void foreach_modules(const char *name);
void exec_module_scripts(const char *stage);

// MagiskHide
void auto_start_magiskhide();
int stop_magiskhide();

// Scripting
void exec_script(const char *script);
void exec_common_scripts(const char *stage);
void exec_module_scripts(const char *stage, const std::vector<std::string> &module_list);
void install_apk(const char *apk);
