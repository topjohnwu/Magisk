#ifndef MAGISK_HIDE_H
#define MAGISK_HIDE_H

#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>
#include <string>
#include <set>
#include <functional>
#include <map>

#include "daemon.h"

#define TERM_THREAD SIGUSR1

#define SAFETYNET_COMPONENT  "com.google.android.gms/.droidguard.DroidGuardService"
#define SAFETYNET_PROCESS    "com.google.android.gms.unstable"
#define SAFETYNET_PKG        "com.google.android.gms"

// Daemon entries
void launch_magiskhide(int client);
int stop_magiskhide();
int add_list(int client);
int rm_list(int client);
void ls_list(int client);

// Update APK list for inotify
void update_inotify_mask();

// Process monitor
void proc_monitor();

// Utility functions
void manage_selinux();
void clean_magisk_props();
void crawl_procfs(const std::function<bool (int)> &fn);
bool proc_name_match(int pid, const char *name);

static inline int get_uid(const int pid) {
	char path[16];
	struct stat st;

	sprintf(path, "/proc/%d", pid);
	if (stat(path, &st) == -1)
		return -1;

	// We don't care about multiuser
	return st.st_uid % 100000;
}

/*
 * Bionic's atoi runs through strtol().
 * Use our own implementation for faster conversion.
 */
static inline int parse_int(const char *s) {
	int val = 0;
	char c;
	while ((c = *(s++))) {
		if (c > '9' || c < '0')
			return -1;
		val = val * 10 + c - '0';
	}
	return val;
}

extern bool hide_enabled;
extern pthread_mutex_t map_lock;
extern std::map<std::string, std::string> hide_map;

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

#endif
