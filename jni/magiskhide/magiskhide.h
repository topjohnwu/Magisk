#ifndef MAGISK_HIDE_H
#define MAGISK_HIDE_H

#include <pthread.h>

// Kill process
void kill_proc(int pid);

// Hide daemon
int hide_daemon();

// Process monitor
void proc_monitor();

// Preprocess
void manage_selinux();
void hide_sensitive_props();
void relink_sbin();

// List managements
int add_list(char *proc);
int rm_list(char *proc);
int init_list();
int destroy_list();

extern int sv[2], hide_pid, hideEnabled;
extern struct vector *hide_list;
extern pthread_mutex_t hide_lock, file_lock;

#endif
