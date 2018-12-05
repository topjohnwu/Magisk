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
   fd == nullptr -> Ignore output
  *fd < 0     -> Open pipe and set *fd to the read end
  *fd >= 0    -> STDOUT (or STDERR) will be redirected to *fd
  *pre_exec   -> A callback function called after forking, before execvp
*/
int exec_array(bool err, int *fd, void (*pre_exec)(void), const char **argv) {
	int pipefd[2], outfd = -1;

	if (fd) {
		if (*fd < 0) {
			if (xpipe2(pipefd, O_CLOEXEC) == -1)
				return -1;
			outfd = pipefd[1];
		} else {
			outfd = *fd;
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

	if (outfd >= 0) {
		xdup2(outfd, STDOUT_FILENO);
		if (err)
			xdup2(outfd, STDERR_FILENO);
		close(outfd);
	}

	// Call the pre-exec callback
	if (pre_exec)
		pre_exec();

	execve(argv[0], (char **) argv, environ);
	PLOGE("execve %s", argv[0]);
	return -1;
}

static int v_exec_command(bool err, int *fd, void (*cb)(void), const char *argv0, va_list argv) {
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
	pid = v_exec_command(false, nullptr, nullptr, argv0, argv);
	va_end(argv);
	if (pid < 0)
		return pid;
	waitpid(pid, &status, 0);
	return WEXITSTATUS(status);
}

int exec_command(bool err, int *fd, void (*cb)(void), const char *argv0, ...) {
	va_list argv;
	va_start(argv, argv0);
	int pid = v_exec_command(err, fd, cb, argv0, argv);
	va_end(argv);
	return pid;
}

char *strdup2(const char *s, size_t *size) {
	size_t len = strlen(s) + 1;
	char *buf = new char[len];
	memcpy(buf, s, len);
	if (size) *size = len;
	return buf;
}
