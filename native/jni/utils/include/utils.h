/* util.h - Header for all utility functions
 */

#pragma once

#include <stdio.h>
#include <dirent.h>
#include <pthread.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/mman.h>

#ifdef __cplusplus
extern "C" {
#endif

#define UID_SHELL  (get_shell_uid())
#define UID_ROOT   0

// xwrap.cpp

FILE *xfopen(const char *pathname, const char *mode);
FILE *xfdopen(int fd, const char *mode);
#define GET_MACRO(_1, _2, _3, NAME, ...) NAME
#define xopen(...) GET_MACRO(__VA_ARGS__, xopen3, xopen2)(__VA_ARGS__)
int xopen2(const char *pathname, int flags);
int xopen3(const char *pathname, int flags, mode_t mode);
int xopenat(int dirfd, const char *pathname, int flags);
ssize_t xwrite(int fd, const void *buf, size_t count);
ssize_t xread(int fd, void *buf, size_t count);
ssize_t xxread(int fd, void *buf, size_t count);
int xpipe2(int pipefd[2], int flags);
int xsetns(int fd, int nstype);
int xunshare(int flags);
DIR *xopendir(const char *name);
DIR *xfdopendir(int fd);
struct dirent *xreaddir(DIR *dirp);
pid_t xsetsid();
int xsocket(int domain, int type, int protocol);
int xbind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int xconnect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int xlisten(int sockfd, int backlog);
int xaccept4(int sockfd, struct sockaddr *addr, socklen_t *addrlen, int flags);
void *xmalloc(size_t size);
void *xcalloc(size_t nmemb, size_t size);
void *xrealloc(void *ptr, size_t size);
ssize_t xsendmsg(int sockfd, const struct msghdr *msg, int flags);
ssize_t xrecvmsg(int sockfd, struct msghdr *msg, int flags);
int xpthread_create(pthread_t *thread, const pthread_attr_t *attr,
	void *(*start_routine) (void *), void *arg);
int xstat(const char *pathname, struct stat *buf);
int xlstat(const char *pathname, struct stat *buf);
int xfstat(int fd, struct stat *buf);
int xdup2(int oldfd, int newfd);
int xdup3(int oldfd, int newfd, int flags);
ssize_t xreadlink(const char *pathname, char *buf, size_t bufsiz);
ssize_t xreadlinkat(int dirfd, const char *pathname, char *buf, size_t bufsiz);
int xsymlink(const char *target, const char *linkpath);
int xsymlinkat(const char *target, int newdirfd, const char *linkpath);
int xlinkat(int olddirfd, const char *oldpath, int newdirfd, const char *newpath, int flags);
int xmount(const char *source, const char *target,
	const char *filesystemtype, unsigned long mountflags,
	const void *data);
int xumount(const char *target);
int xumount2(const char *target, int flags);
int xrename(const char *oldpath, const char *newpath);
int xmkdir(const char *pathname, mode_t mode);
int xmkdirs(const char *pathname, mode_t mode);
int xmkdirat(int dirfd, const char *pathname, mode_t mode);
void *xmmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
ssize_t xsendfile(int out_fd, int in_fd, off_t *offset, size_t count);
pid_t xfork();
int xpoll(struct pollfd *fds, nfds_t nfds, int timeout);
int xinotify_init1(int flags);

// misc.cpp

unsigned get_shell_uid();
int fork_dont_care();
int fork_no_zombie();
void gen_rand_str(char *buf, int len);
int strend(const char *s1, const char *s2);
char *rtrim(char *str);
void init_argv0(int argc, char **argv);
void set_nice_name(const char *name);
int parse_int(const char *s);

#define getline __getline
#define getdelim __getdelim

ssize_t __getline(char **lineptr, size_t *n, FILE *stream);
ssize_t __getdelim(char **lineptr, size_t *n, int delim, FILE *stream);

// file.cpp

#define do_align(p, a)  (((p) + (a) - 1) / (a) * (a))
#define align_off(p, a) (do_align(p, a) - (p))

struct file_attr {
	struct stat st;
	char con[128];
};

ssize_t fd_path(int fd, char *path, size_t size);
int fd_pathat(int dirfd, const char *name, char *path, size_t size);
int mkdirs(const char *pathname, mode_t mode);
void rm_rf(const char *path);
void mv_f(const char *source, const char *destination);
void mv_dir(int src, int dest);
void cp_afc(const char *source, const char *destination);
void link_dir(int src, int dest);
int getattr(const char *path, struct file_attr *a);
int getattrat(int dirfd, const char *name, struct file_attr *a);
int fgetattr(int fd, struct file_attr *a);
int setattr(const char *path, struct file_attr *a);
int setattrat(int dirfd, const char *name, struct file_attr *a);
int fsetattr(int fd, struct file_attr *a);
void fclone_attr(int sourcefd, int targetfd);
void clone_attr(const char *source, const char *target);
void mmap_ro(const char *filename, void **buf, size_t *size);
void fd_full_read(int fd, void **buf, size_t *size);
void full_read(const char *filename, void **buf, size_t *size);
void write_zero(int fd, size_t size);

#ifdef __cplusplus
}

#include <string>
#include <vector>
#include <functional>
#include <string_view>

#define str_contains(s, ss) ((ss) != nullptr && (s).find(ss) != std::string::npos)
#define str_starts(s, ss) ((ss) != nullptr && (s).compare(0, strlen(ss), ss) == 0)

// RAII

class MutexGuard {
public:
	explicit MutexGuard(pthread_mutex_t &m): mutex(&m) {
		pthread_mutex_lock(mutex);
	}

	explicit MutexGuard(pthread_mutex_t *m): mutex(m) {
		pthread_mutex_lock(mutex);
	}

	~MutexGuard() {
		pthread_mutex_unlock(mutex);
	}

private:
	pthread_mutex_t *mutex;
};

class RunFinally {
public:
	explicit RunFinally(std::function<void()> &&fn): fn(std::move(fn)) {}

	void disable() { fn = nullptr; }

	~RunFinally() { if (fn) fn(); }

private:
	std::function<void ()> fn;
};

// file.cpp

void file_readline(const char *file, const std::function<bool (std::string_view)> &fn, bool trim = false);
void parse_prop_file(const char *file, const std::function
        <bool(std::string_view, std::string_view)> &fn);
void *__mmap(const char *filename, size_t *size, bool rw);
void frm_rf(int dirfd, std::initializer_list<const char *> excl = std::initializer_list<const char *>());
void clone_dir(int src, int dest, bool overwrite = true);

template <typename B>
void mmap_ro(const char *filename, B &buf, size_t &sz) {
	buf = (B) __mmap(filename, &sz, false);
}

template <typename B, typename L>
void mmap_ro(const char *filename, B &buf, L &sz) {
	size_t __sz;
	buf = (B) __mmap(filename, &__sz, false);
	sz = __sz;
}

template <typename B>
void mmap_rw(const char *filename, B &buf, size_t &sz) {
	buf = (B) __mmap(filename, &sz, true);
}

template <typename B, typename L>
void mmap_rw(const char *filename, B &buf, L &sz) {
	size_t __sz;
	buf = (B) __mmap(filename, &__sz, true);
	sz = __sz;
}

// misc.cpp

static inline int parse_int(char *s) { return parse_int((const char *) s); }

template <class S>
int parse_int(S __s) { return parse_int(__s.data()); }

int new_daemon_thread(void *(*start_routine) (void *), void *arg = nullptr,
		const pthread_attr_t *attr = nullptr);

struct exec_t {
	bool err = false;
	int fd = -2;
	void (*pre_exec)() = nullptr;
	int (*fork)() = xfork;
	const char **argv = nullptr;
};

int exec_command(exec_t &exec);
template <class ...Args>
int exec_command(exec_t &exec, Args &&...args) {
	const char *argv[] = {args..., nullptr};
	exec.argv = argv;
	return exec_command(exec);
}
int exec_command_sync(exec_t &exec);
template <class ...Args>
int exec_command_sync(exec_t &exec, Args &&...args) {
	const char *argv[] = {args..., nullptr};
	exec.argv = argv;
	return exec_command_sync(exec);
}
template <class ...Args>
int exec_command_sync(Args &&...args) {
	exec_t exec{};
	return exec_command_sync(exec, args...);
}

bool ends_with(const std::string_view &s1, const std::string_view &s2);

#endif
