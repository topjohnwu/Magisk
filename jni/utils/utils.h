/* util.h - Header for all utility functions
 */

#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "vector.h"

// xwrap.c

FILE *xfopen(const char *pathname, const char *mode);
int xopen(const char *pathname, int flags);
ssize_t xwrite(int fd, const void *buf, size_t count);
ssize_t xread(int fd, void *buf, size_t count);
ssize_t xxread(int fd, void *buf, size_t count);
int xpipe(int pipefd[2]);
int xsetns(int fd, int nstype);
DIR *xopendir(const char *name);
struct dirent *xreaddir(DIR *dirp);
pid_t xsetsid();
int xsetcon(char *context);
int xsocket(int domain, int type, int protocol);
int xbind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int xconnect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int xlisten(int sockfd, int backlog);
int xaccept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
void *xmalloc(size_t size);
void *xcalloc(size_t nmemb, size_t size);
void *xrealloc(void *ptr, size_t size);
ssize_t xsendmsg(int sockfd, const struct msghdr *msg, int flags);
ssize_t xrecvmsg(int sockfd, struct msghdr *msg, int flags);

// log_monitor.c

void monitor_logs();

// misc.c

int check_data();
void file_to_vector(struct vector *v, FILE *fp);
int isNum(const char *s);
ssize_t fdreadline(int fd, char *buf, size_t size);
void ps(void (*func)(int));
void ps_filter_proc_name(const char *filter, void (*func)(int));

#endif