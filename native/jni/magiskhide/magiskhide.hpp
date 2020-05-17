#pragma once

#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <unistd.h>
#include <dirent.h>
#include <string>
#include <functional>
#include <map>
#include <set>

#include <daemon.hpp>

#define SIGTERMTHRD SIGUSR1
#define SIGZYGOTE   SIGUSR2

// CLI entries
int launch_magiskhide();
int stop_magiskhide();
int add_list(int client);
int rm_list(int client);
void ls_list(int client);
[[noreturn]] void test_proc_monitor();

// Process monitoring
void proc_monitor();
void update_uid_map();

// Utility functions
void crawl_procfs(const std::function<bool (int)> &fn);
void crawl_procfs(DIR *dir, const std::function<bool (int)> &fn);
bool hide_enabled();
void set_hide_state(bool state);

// Hide policies
void hide_daemon(int pid);
void hide_unmount(int pid = getpid());
void hide_sensitive_props();
void hide_late_sensitive_props();

extern pthread_mutex_t monitor_lock;
extern std::set<std::pair<std::string, std::string>> hide_set;

enum {
	LAUNCH_MAGISKHIDE,
	STOP_MAGISKHIDE,
	ADD_HIDELIST,
	RM_HIDELIST,
	LS_HIDELIST,
	HIDE_STATUS,
};

enum {
	HIDE_IS_ENABLED = DAEMON_LAST,
	HIDE_NOT_ENABLED,
	HIDE_ITEM_EXIST,
	HIDE_ITEM_NOT_EXIST,
	HIDE_NO_NS,
	HIDE_INVALID_PKG
};
