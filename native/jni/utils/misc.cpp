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
#include <sys/prctl.h>
#include <sys/sysmacros.h>

#include <logging.h>
#include <utils.h>

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

int fork_no_zombie() {
	int pid = xfork();
	if (pid)
		return pid;
	// Unblock all signals
	sigset_t block_set;
	sigfillset(&block_set);
	pthread_sigmask(SIG_UNBLOCK, &block_set, nullptr);
	prctl(PR_SET_PDEATHSIG, SIGTERM);
	if (getppid() == 1)
		exit(1);
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

int exec_command(exec_t &exec) {
	int pipefd[2] = {-1, -1}, outfd = -1;

	if (exec.fd == -1) {
		if (xpipe2(pipefd, O_CLOEXEC) == -1)
			return -1;
		outfd = pipefd[1];
	} else if (exec.fd >= 0) {
		outfd = exec.fd;
	}

	int pid = exec.fork();
	if (pid < 0) {
		close(pipefd[0]);
		close(pipefd[1]);
		return -1;
	} else if (pid) {
		if (exec.fd == -1) {
			exec.fd = pipefd[0];
			close(pipefd[1]);
		}
		return pid;
	}

	if (outfd >= 0) {
		xdup2(outfd, STDOUT_FILENO);
		if (exec.err)
			xdup2(outfd, STDERR_FILENO);
		close(outfd);
	}

	// Call the pre-exec callback
	if (exec.pre_exec)
		exec.pre_exec();

	execve(exec.argv[0], (char **) exec.argv, environ);
	PLOGE("execve %s", exec.argv[0]);
	exit(-1);
}

int exec_command_sync(exec_t &exec) {
	int pid, status;
	pid = exec_command(exec);
	if (pid < 0)
		return -1;
	waitpid(pid, &status, 0);
	return WEXITSTATUS(status);
}

int new_daemon_thread(void *(*start_routine) (void *), void *arg, const pthread_attr_t *attr) {
	pthread_t thread;
	int ret = xpthread_create(&thread, attr, start_routine, arg);
	if (ret == 0)
		pthread_detach(thread);
	return ret;
}

