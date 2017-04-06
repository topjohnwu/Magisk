/* xwrap.c - wrappers around existing library functions.
 *
 * Functions with the x prefix are wrappers that either succeed or kill the
 * program with an error message, but never return failure. They usually have
 * the same arguments and return value as the function they wrap.
 *
 */

#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "utils.h"

FILE *xfopen(const char *pathname, const char *mode) {
	FILE *fp = fopen(pathname, mode);
	if (fp == NULL) {
		PLOGE("fopen");
	}
	return fp;
}

int xopen(const char *pathname, int flags) {
	int fd = open(pathname, flags);
	if (fd < 0) {
		PLOGE("open");
	}
	return fd;
}

ssize_t xwrite(int fd, const void *buf, size_t count) {
	if (count != write(fd, buf, count)) {
		PLOGE("write");
	}
	return count;
}

// Read error other than EOF
ssize_t xread(int fd, void *buf, size_t count) {
	int ret = read(fd, buf, count);
	if (ret < 0) {
		PLOGE("read");
	}
	return ret;
}

// Read exact same size as count
ssize_t xxread(int fd, void *buf, size_t count) {
	if (count != read(fd, buf, count)) {
		PLOGE("read");
	}
	return count;
}

int xpipe(int pipefd[2]) {
	if (pipe(pipefd) == -1) {
		PLOGE("pipe");
	}
	return 0;
}

int xsetns(int fd, int nstype) {
	if (setns(fd, nstype) == -1) {
		PLOGE("setns");
	}
	return 0;
}

DIR *xopendir(const char *name) {
	DIR *d = opendir(name);
	if (d == NULL) {
		PLOGE("opendir");
	}
	return d;
}

struct dirent *xreaddir(DIR *dirp) {
	errno = 0;
	struct dirent *e = readdir(dirp);
	if (errno && e == NULL) {
		PLOGE("readdir");
	}
	return e;
}

