#ifndef MAGISK_HIDE_H
#define MAGISK_HIDE_H

#define HIDELIST		"/magisk/.core/magiskhide/hidelist"
#define DUMMYPATH		"/dev/magisk/dummy"
#define ENFORCE_FILE 	"/sys/fs/selinux/enforce"
#define POLICY_FILE 	"/sys/fs/selinux/policy"

// Hide daemon
void hide_daemon();

// Process monitor
void *proc_monitor(void *args);

extern int pipefd[2];
extern struct vector *hide_list, *new_list;

#endif
