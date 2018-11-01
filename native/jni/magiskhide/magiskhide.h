#ifndef MAGISK_HIDE_H
#define MAGISK_HIDE_H

#include <pthread.h>

#include "daemon.h"
#include "array.h"

#define TERM_THREAD SIGUSR1

// Daemon entries
extern "C" {
int launch_magiskhide();
}
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
int add_list(char *proc);
int init_list();
int destroy_list();

extern int hide_enabled;
extern pthread_mutex_t list_lock;
extern Array<char *> hide_list;

enum {
	LAUNCH_MAGISKHIDE,
	STOP_MAGISKHIDE,
	ADD_HIDELIST,
	RM_HIDELIST,
	LS_HIDELIST
};

enum {
	LOGCAT_DISABLED = DAEMON_LAST,
	HIDE_IS_ENABLED,
	HIDE_NOT_ENABLED,
	HIDE_ITEM_EXIST,
	HIDE_ITEM_NOT_EXIST
};

#endif
