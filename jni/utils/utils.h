/* util.h - Header for all utility functions
 */

#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdio.h>
#include <dirent.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>

#include "vector.h"

#define UID_SHELL  (get_shell_uid())
#define UID_ROOT   0
#define UID_SYSTEM (get_system_uid())
#define UID_RADIO  (get_radio_uid())

extern int quit_signals[];

// xwrap.c

FILE *xfopen(const char *pathname, const char *mode);
FILE *xfdopen(int fd, const char *mode);
#define GET_MACRO(_1, _2, _3, NAME, ...) NAME
#define xopen(...) GET_MACRO(__VA_ARGS__, xopen3, xopen2)(__VA_ARGS__)
int xopen2(const char *pathname, int flags);
int xopen3(const char *pathname, int flags, mode_t mode);
ssize_t xwrite(int fd, const void *buf, size_t count);
ssize_t xread(int fd, void *buf, size_t count);
ssize_t xxread(int fd, void *buf, size_t count);
int xpipe(int pipefd[2]);
int xsetns(int fd, int nstype);
DIR *xopendir(const char *name);
struct dirent *xreaddir(DIR *dirp);
pid_t xsetsid();
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
int xpthread_create(pthread_t *thread, const pthread_attr_t *attr,
	void *(*start_routine) (void *), void *arg);
int xsocketpair(int domain, int type, int protocol, int sv[2]);
int xstat(const char *pathname, struct stat *buf);
int xlstat(const char *pathname, struct stat *buf);
int xdup2(int oldfd, int newfd);
ssize_t xreadlink(const char *pathname, char *buf, size_t bufsiz);
int xsymlink(const char *target, const char *linkpath);
int xmount(const char *source, const char *target,
	const char *filesystemtype, unsigned long mountflags,
	const void *data);
int xumount(const char *target);
int xumount2(const char *target, int flags);
int xchmod(const char *pathname, mode_t mode);
int xrename(const char *oldpath, const char *newpath);
int xmkdir(const char *pathname, mode_t mode);
void *xmmap(void *addr, size_t length, int prot, int flags,
	int fd, off_t offset);
ssize_t xsendfile(int out_fd, int in_fd, off_t *offset, size_t count);
int xmkdir_p(const char *pathname, mode_t mode);

// misc.c

unsigned get_shell_uid();
unsigned get_system_uid();
unsigned get_radio_uid();
int check_data();
int file_to_vector(const char* filename, struct vector *v);
int vector_to_file(const char* filename, struct vector *v);
int isNum(const char *s);
ssize_t fdgets(char *buf, size_t size, int fd);
void ps(void (*func)(int));
void ps_filter_proc_name(const char *filter, void (*func)(int));
int create_links(const char *bin, const char *path);
void unlock_blocks();
void setup_sighandlers(void (*handler)(int));
int run_command(int *fd, const char *path, char *const argv[]);
int mkdir_p(const char *pathname, mode_t mode);
int bind_mount(const char *from, const char *to);
int open_new(const char *filename);
int cp_afc(const char *source, const char *target);
int clone_dir(const char *source, const char *target);
int rm_rf(const char *target);
void clone_attr(const char *source, const char *target);
void get_client_cred(int fd, struct ucred *cred);

#endif
