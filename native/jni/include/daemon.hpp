#pragma once

#include <pthread.h>
#include <string>
#include <vector>

#include <socket.hpp>

// Commands require connecting to daemon
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

extern int SDK_INT;
extern bool RECOVERY_MODE;
extern std::vector<std::string> module_list;
#define APP_DATA_DIR (SDK_INT >= 24 ? "/data/user_de" : "/data/user")

// Daemon handlers
void post_fs_data(int client);
void late_start(int client);
void boot_complete(int client);
void magiskhide_handler(int client);
void su_daemon_handler(int client, struct ucred *credential);
void foreach_modules(const char *name);

// Misc
int connect_daemon(bool create = false);
void unlock_blocks();
void handle_modules();
void magic_mount();
void reboot();

// MagiskHide
void auto_start_magiskhide();
void hide_sensitive_props_late();
int stop_magiskhide();

// Scripting
void exec_script(const char *script);
void exec_common_script(const char *stage);
void exec_module_script(const char *stage, const std::vector<std::string> &module_list);
void install_apk(const char *apk);
