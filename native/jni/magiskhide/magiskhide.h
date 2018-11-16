#ifndef MAGISK_HIDE_H
#define MAGISK_HIDE_H

#include <pthread.h>

#include "daemon.h"
#include "Vector.h"
#include "CharArray.h"

#define TERM_THREAD SIGUSR1

// Daemon entries
int launch_magiskhide(int client);
int stop_magiskhide();
int add_list(int client);
int rm_list(int client);
void ls_list(int client);

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
extern Vector<CharArray> hide_list;

enum {
	LAUNCH_MAGISKHIDE,
	STOP_MAGISKHIDE,
	ADD_HIDELIST,
	RM_HIDELIST,
	LS_HIDELIST,
	HIDE_STATUS,
};

enum {
	LOGCAT_DISABLED = DAEMON_LAST,
	HIDE_IS_ENABLED,
	HIDE_NOT_ENABLED,
	HIDE_ITEM_EXIST,
	HIDE_ITEM_NOT_EXIST
};

#endif
