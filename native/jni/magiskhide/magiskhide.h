#ifndef MAGISK_HIDE_H
#define MAGISK_HIDE_H

#include <pthread.h>
#include <vector>
#include <string>

#include "daemon.h"

#define TERM_THREAD SIGUSR1

// Daemon entries
int launch_magiskhide(int client);
int stop_magiskhide();
int add_list(int client);
int rm_list(int client);
void ls_list(int client);

// Update APK list for inotify
void update_apk_list();

// Process monitor
void proc_monitor();

// Utility functions
void manage_selinux();
void hide_sensitive_props();
void clean_magisk_props();

// List managements
int add_list(const char *proc);
bool init_list();

extern bool hide_enabled;
extern pthread_mutex_t list_lock;
extern std::vector<std::string> hide_list;

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
	HIDE_ITEM_NOT_EXIST
};

#endif
