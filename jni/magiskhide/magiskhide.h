#ifndef MAGISK_HIDE_H
#define MAGISK_HIDE_H

#define HIDELIST		"/magisk/.core/magiskhide/hidelist"
#define DUMMYPATH		"/dev/magisk/dummy"
#define ENFORCE_FILE 	"/sys/fs/selinux/enforce"
#define POLICY_FILE 	"/sys/fs/selinux/policy"

// Kill process
void kill_proc(int pid);

// Hide daemon
int hide_daemon();

// Process monitor
void *proc_monitor(void *args);

// Preprocess
void manage_selinux();
void hide_sensitive_props();
void relink_sbin();

extern int sv[2], hide_pid, isEnabled;
extern struct vector *hide_list, *new_list;

#endif
