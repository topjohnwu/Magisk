#ifndef MAGISK_HIDE_H
#define MAGISK_HIDE_H

#define _GNU_SOURCE
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>

#include <sys/mount.h>
#include <sys/inotify.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/resource.h>

#define LOGFILE 	"/cache/magisk.log"
#define HIDELIST 	"/magisk/.core/magiskhide/hidelist"
#define DUMMYPATH	"/dev/magisk/dummy"

// Main thread
void monitor_proc();

// Forked process for namespace setting
int hideMagisk();

// List monitor thread
void update_list(const char *listpath);
void quit_pthread(int sig);
void *monitor_list(void *path);

// Util functions
char **file_to_str_arr(FILE *fp, int *size);
void read_namespace(const int pid, char* target, const size_t size);
void lazy_unmount(const char* mountpoint);
void run_as_daemon();

// Global variable sharing through process/threads
extern FILE *logfile;
extern int i, list_size, pipefd[2];
extern char **hide_list, buffer[512];
extern pthread_t list_monitor;
extern pthread_mutex_t mutex;

#endif
