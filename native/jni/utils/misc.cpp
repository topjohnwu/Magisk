/* misc.cpp - Store all functions that are unable to be catagorized clearly
 */
 
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <pwd.h>
#include <unistd.h>
#include <syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/sysmacros.h>

#include "logging.h"
#include "utils.h"

unsigned get_shell_uid() {
	struct passwd* ppwd = getpwnam("shell");
	if (nullptr == ppwd)
		return 2000;

	return ppwd->pw_uid;
}

unsigned get_system_uid() {
	struct passwd* ppwd = getpwnam("system");
	if (nullptr == ppwd)
		return 1000;

	return ppwd->pw_uid;
}

unsigned get_radio_uid() {
	struct passwd* ppwd = getpwnam("radio");
	if (nullptr == ppwd)
		return 1001;

	return ppwd->pw_uid;
}

/* Check if the string only contains digits */
int is_num(const char *s) {
	int len = strlen(s);
	for (int i = 0; i < len; ++i)
		if (s[i] < '0' || s[i] > '9')
			return 0;
	return 1;
}

/* Read a whole line from file descriptor */
ssize_t fdgets(char *buf, const size_t size, int fd) {
	ssize_t len = 0;
	buf[0] = '\0';
	while (len < size - 1) {
		int ret = read(fd, buf + len, 1);
		if (ret < 0)
			return -1;
		if (ret == 0)
			break;
		if (buf[len] == '\0' || buf[len++] == '\n') {
			buf[len] = '\0';
			break;
		}
	}
	buf[size - 1] = '\0';
	return len;
}

int fork_dont_care() {
	int pid = xfork();
	if (pid) {
		waitpid(pid, nullptr, 0);
		return pid;
	} else if ((pid = xfork())) {
		exit(0);
	}
	return 0;
}

void gen_rand_str(char *buf, int len) {
	const char base[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	int urandom;
	if (access("/dev/urandom", R_OK) == 0) {
		urandom = xopen("/dev/urandom", O_RDONLY | O_CLOEXEC);
	} else {
		mknod("/urandom", S_IFCHR | 0666, makedev(1, 9));
		urandom = xopen("/urandom", O_RDONLY | O_CLOEXEC);
		unlink("/urandom");
	}
	xxread(urandom, buf, len - 1);
	close(urandom);
	for (int i = 0; i < len - 1; ++i) {
		buf[i] = base[buf[i] % (sizeof(base) - 1)];
	}
	buf[len - 1] = '\0';
}

int strend(const char *s1, const char *s2) {
	size_t l1 = strlen(s1);
	size_t l2 = strlen(s2);
	return strcmp(s1 + l1 - l2, s2);
}

/* Original source: https://opensource.apple.com/source/cvs/cvs-19/cvs/lib/getline.c
 * License: GPL 2 or later
 * Adjusted to match POSIX */
#define MIN_CHUNK 64
ssize_t __getdelim(char **lineptr, size_t *n, int delim, FILE *stream) {
	size_t nchars_avail;
	char *read_pos;

	if (!lineptr || !n || !stream) {
		errno = EINVAL;
		return -1;
	}

	if (!*lineptr) {
		*n = MIN_CHUNK;
		*lineptr = (char *) malloc(*n);
		if (!*lineptr) {
			errno = ENOMEM;
			return -1;
		}
	}

	nchars_avail = *n;
	read_pos = *lineptr;

	while (1) {
		int save_errno;
		int c = getc(stream);

		save_errno = errno;

		if (nchars_avail < 2) {
			if (*n > MIN_CHUNK)
				*n *= 2;
			else
				*n += MIN_CHUNK;

			nchars_avail = *n + *lineptr - read_pos;
			*lineptr = (char *) realloc(*lineptr, *n);
			if (!*lineptr) {
				errno = ENOMEM;
				return -1;
			}
			read_pos = *n - nchars_avail + *lineptr;
		}

		if (ferror(stream)) {
			errno = save_errno;
			return -1;
		}

		if (c == EOF) {
			if (read_pos == *lineptr)
				return -1;
			else
				break;
		}

		*read_pos++ = c;
		nchars_avail--;

		if (c == delim)
			break;
	}

	*read_pos = '\0';

	return read_pos - *lineptr;
}

ssize_t __getline(char **lineptr, size_t *n, FILE *stream) {
	return __getdelim(lineptr, n, '\n', stream);
}

int __fsetxattr(int fd, const char *name, const void *value, size_t size, int flags) {
	return (int) syscall(__NR_fsetxattr, fd, name, value, size, flags);
}

/*
   fd == NULL -> Ignore output
  *fd < 0     -> Open pipe and set *fd to the read end
  *fd >= 0    -> STDOUT (or STDERR) will be redirected to *fd
  *cb         -> A callback function which calls after forking
*/
int exec_array(int err, int *fd, void (*cb)(void), const char *argv[]) {
	int pipefd[2], write_end = -1;

	if (fd) {
		if (*fd < 0) {
			if (xpipe2(pipefd, O_CLOEXEC) == -1)
				return -1;
			write_end = pipefd[1];
		} else {
			write_end = *fd;
		}
	}

	int pid = xfork();
	if (pid != 0) {
		if (fd && *fd < 0) {
			// Give the read end and close write end
			*fd = pipefd[0];
			close(pipefd[1]);
		}
		return pid;
	}

	if (fd) {
		xdup2(write_end, STDOUT_FILENO);
		if (err)
			xdup2(write_end, STDERR_FILENO);
	}

	// Setup environment
	if (cb)
		cb();

	execvp(argv[0], (char **) argv);
	PLOGE("execvp %s", argv[0]);
	return -1;
}

static int v_exec_command(int err, int *fd, void (*cb)(void), const char *argv0, va_list argv) {
	// Collect va_list into vector
	Vector<const char *> args;
	args.push_back(argv0);
	for (const char *arg = va_arg(argv, char*); arg; arg = va_arg(argv, char*))
		args.push_back(arg);
	args.push_back(nullptr);
	int pid = exec_array(err, fd, cb, args.data());
	return pid;
}

int exec_command_sync(const char *argv0, ...) {
	va_list argv;
	va_start(argv, argv0);
	int pid, status;
	pid = v_exec_command(0, NULL, NULL, argv0, argv);
	va_end(argv);
	if (pid < 0)
		return pid;
	waitpid(pid, &status, 0);
	return WEXITSTATUS(status);
}

int exec_command(int err, int *fd, void (*cb)(void), const char *argv0, ...) {
	va_list argv;
	va_start(argv, argv0);
	int pid = v_exec_command(err, fd, cb, argv0, argv);
	va_end(argv);
	return pid;
}

char *strdup2(const char *s, size_t *size) {
	size_t l = strlen(s) + 1;
	char *buf = new char[l];
	memcpy(buf, s, l);
	if (size) *size = l;
	return buf;
}
