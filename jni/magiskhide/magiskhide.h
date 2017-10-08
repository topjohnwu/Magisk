#ifndef MAGISK_HIDE_H
#define MAGISK_HIDE_H

#include <pthread.h>

// Kill process
void kill_proc(int pid);

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

extern int hideEnabled;
extern struct vector *hide_list;
extern pthread_mutex_t hide_lock, file_lock;

#endif
