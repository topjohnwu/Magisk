#ifndef _UTILS_H_
#define _UTILS_H_

#include <sys/types.h>
#include <stdio.h>

#include "magisk.h"

// xwrap.c

FILE *xfopen(const char *pathname, const char *mode);
int xopen(const char *pathname, int flags);
ssize_t xwrite(int fd, const void *buf, size_t count);
ssize_t xread(int fd, void *buf, size_t count);
ssize_t xxread(int fd, void *buf, size_t count);
int xpipe(int pipefd[2]);
int xsetns(int fd, int nstype);

// vector.c

#include "vector.h"

// log_monitor.c

void monitor_logs();

// misc.c

int check_data();
void file_to_vector(struct vector *v, FILE *fp);

#endif