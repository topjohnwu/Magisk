#ifndef MAGISK_HIDE_H
#define MAGISK_HIDE_H

#include <pthread.h>
#include "array.h"

#define TERM_THREAD SIGUSR1

// Process monitor
void proc_monitor();

// Utility functions
void manage_selinux();
void hide_sensitive_props();
void clean_magisk_props();

// List managements
int add_list(char *proc);
int rm_list(char *proc);
int init_list();
int destroy_list();

extern int hide_enabled;
extern pthread_mutex_t list_lock;
extern Array<char *> hide_list;

#endif
