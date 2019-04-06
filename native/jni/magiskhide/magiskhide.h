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

#include <daemon.h>

#define SIGTERMTHRD SIGUSR1

#define SAFETYNET_COMPONENT  "com.google.android.gms/.droidguard.DroidGuardService"
#define SAFETYNET_PROCESS    "com.google.android.gms.unstable"
#define SAFETYNET_PKG        "com.google.android.gms"
#define MICROG_SAFETYNET     "org.microg.gms.droidguard"

#define WEVENT(s) (((s) & 0xffff0000) >> 16)

// CLI entries
void launch_magiskhide(int client);
int stop_magiskhide();
int add_list(int client);
int rm_list(int client);
void ls_list(int client);

// Process monitoring
void *update_uid_map(void * p = nullptr);
void proc_monitor();

// Utility functions
void manage_selinux();
void clean_magisk_props();
void crawl_procfs(const std::function<bool (int)> &fn);
void crawl_procfs(DIR *dir, const std::function<bool (int)> &fn);
bool proc_name_match(int pid, const char *name);

extern bool hide_enabled;
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
	HIDE_NO_NS
};
