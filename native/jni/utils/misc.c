/* misc.c - Store all functions that are unable to be catagorized clearly
 */

#define _GNU_SOURCE
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <fcntl.h>
#include <pwd.h>
#include <signal.h>
#include <sched.h>
#include <unistd.h>
#include <libgen.h>
#include <syscall.h>
#include <sys/types.h>
#include <sys/mount.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/inotify.h>
#include <sys/sysmacros.h>

#include "logging.h"
#include "utils.h"
#include "resetprop.h"

unsigned get_shell_uid() {
	struct passwd* ppwd = getpwnam("shell");
	if (NULL == ppwd)
		return 2000;

	return ppwd->pw_uid;
}

unsigned get_system_uid() {
	struct passwd* ppwd = getpwnam("system");
	if (NULL == ppwd)
		return 1000;

	return ppwd->pw_uid;
}

unsigned get_radio_uid() {
	struct passwd* ppwd = getpwnam("radio");
	if (NULL == ppwd)
		return 1001;

	return ppwd->pw_uid;
}

int check_data() {
	struct vector v;
	vec_init(&v);
	file_to_vector("/proc/mounts", &v);
	char *line;
	int mnt = 0;
	vec_for_each(&v, line) {
		if (strstr(line, " /data ")) {
			if (strstr(line, "tmpfs") == NULL)
				mnt = 1;
			break;
		}
	}
	vec_deep_destroy(&v);
	int data = 0;
	if (mnt) {
		char *crypto = getprop("ro.crypto.state");
		if (crypto != NULL) {
			if (strcmp(crypto, "unencrypted") == 0) {
				// Unencrypted, we can directly access data
				data = 1;
			} else {
				// Encrypted, check whether vold is started
				char *vold = getprop("init.svc.vold");
				if (vold != NULL) {
					free(vold);
					data = 1;
				}
			}
			free(crypto);
		} else {
			// ro.crypto.state is not set, assume it's unencrypted
			data = 1;
		}
	}
	return data;
}

/* All the string should be freed manually!! */
int file_to_vector(const char* filename, struct vector *v) {
	if (access(filename, R_OK) != 0)
		return 1;
	char *line = NULL;
	size_t len = 0;
	ssize_t read;

	FILE *fp = xfopen(filename, "r");
	if (fp == NULL)
		return 1;

	while ((read = getline(&line, &len, fp)) != -1) {
		// Remove end newline
		if (line[read - 1] == '\n')
			line[read - 1] = '\0';
		vec_push_back(v, line);
		line = NULL;
	}
	fclose(fp);
	return 0;
}

int vector_to_file(const char *filename, struct vector *v) {
	FILE *fp = xfopen(filename, "w");
	if (fp == NULL)
		return 1;
	char *line;
	vec_for_each(v, line) {
		fprintf(fp, "%s\n", line);
	}
	fclose(fp);
	return 0;
}

/* Check if the string only contains digits */
static int is_num(const char *s) {
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

/* Call func for each process */
void ps(void (*func)(int)) {
	DIR *dir;
	struct dirent *entry;

	if (!(dir = xopendir("/proc")))
		return;

	while ((entry = xreaddir(dir))) {
		if (entry->d_type == DT_DIR) {
			if (is_num(entry->d_name))
				func(atoi(entry->d_name));
		}
	}

	closedir(dir);
}

int check_proc_name(int pid, const char *name) {
	char buf[128];
	FILE *f;
	sprintf(buf, "/proc/%d/comm", pid);
	if ((f = fopen(buf, "r"))) {
		fgets(buf, sizeof(buf), f);
		if (strcmp(buf, name) == 0)
			return 1;
	} else {
		// The PID is already killed
		return 0;
	}
	fclose(f);

	sprintf(buf, "/proc/%d/cmdline", pid);
	f = fopen(buf, "r");
	fgets(buf, sizeof(buf), f);
	if (strcmp(basename(buf), name) == 0)
		return 1;
	fclose(f);

	sprintf(buf, "/proc/%d/exe", pid);
	if (access(buf, F_OK) != 0)
		return 0;
	xreadlink(buf, buf, sizeof(buf));
	if (strcmp(basename(buf), name) == 0)
		return 1;
	return 0;
}

void unlock_blocks() {
	DIR *dir;
	struct dirent *entry;
	int fd, dev, OFF = 0;

	if ((dev = xopen("/dev/block", O_RDONLY | O_CLOEXEC)) < 0)
		return;
	dir = xfdopendir(dev);

	while((entry = readdir(dir))) {
		if (entry->d_type == DT_BLK) {
			if ((fd = openat(dev, entry->d_name, O_RDONLY)) < 0)
				continue;
			if (ioctl(fd, BLKROSET, &OFF) == -1)
				PLOGE("unlock %s", entry->d_name);
			close(fd);
		}
	}
	close(dev);
}

void setup_sighandlers(void (*handler)(int)) {
	struct sigaction act;
	memset(&act, 0, sizeof(act));
	act.sa_handler = handler;
	for (int i = 0; quit_signals[i]; ++i) {
		sigaction(quit_signals[i], &act, NULL);
	}
}

/*
   fd == NULL -> Ignore output
  *fd < 0     -> Open pipe and set *fd to the read end
  *fd >= 0    -> STDOUT (or STDERR) will be redirected to *fd
  *setenv     -> A callback function which sets up a vector of environment variables
*/
int exec_array(int err, int *fd, void (*setenv)(struct vector *), char *const *argv) {
	int pipefd[2], writeEnd = -1;

	if (fd) {
		if (*fd < 0) {
			if (xpipe2(pipefd, O_CLOEXEC) == -1)
				return -1;
			writeEnd = pipefd[1];
		} else {
			writeEnd = *fd;
		}
	}

	// Setup environment
	char **envp;
	struct vector env;
	vec_init(&env);
	if (setenv) {
		setenv(&env);
		envp = (char **) vec_entry(&env);
	} else {
		extern char **environ;
		envp = environ;
	}

	int pid = xfork();
	if (pid != 0) {
		if (fd && *fd < 0) {
			// Give the read end and close write end
			*fd = pipefd[0];
			close(pipefd[1]);
		}
		vec_deep_destroy(&env);
		return pid;
	}

	if (fd) {
		xdup2(writeEnd, STDOUT_FILENO);
		if (err)
			xdup2(writeEnd, STDERR_FILENO);
	}

	environ = envp;
	execvp(argv[0], argv);
	PLOGE("execvp %s", argv[0]);
	return -1;
}

static int v_exec_command(int err, int *fd, void (*setenv)(struct vector*), const char *argv0, va_list argv) {
	// Collect va_list into vector
	struct vector args;
	vec_init(&args);
	vec_push_back(&args, strdup(argv0));
	for (void *arg = va_arg(argv, void*); arg; arg = va_arg(argv, void*))
		vec_push_back(&args, strdup(arg));
	vec_push_back(&args, NULL);
	int pid = exec_array(err, fd, setenv, (char **) vec_entry(&args));
	vec_deep_destroy(&args);
	return pid;
}

int exec_command_sync(char *const argv0, ...) {
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

int exec_command(int err, int *fd, void (*setenv)(struct vector*), const char *argv0, ...) {
	va_list argv;
	va_start(argv, argv0);
	int pid = v_exec_command(err, fd, setenv, argv0, argv);
	va_end(argv);
	return pid;
}

int bind_mount(const char *from, const char *to) {
	int ret = xmount(from, to, NULL, MS_BIND, NULL);
#ifdef MAGISK_DEBUG
	LOGI("bind_mount: %s <- %s\n", to, from);
#else
	LOGI("bind_mount: %s\n", to);
#endif
	return ret;
}

void get_client_cred(int fd, struct ucred *cred) {
	socklen_t ucred_length = sizeof(*cred);
	if(getsockopt(fd, SOL_SOCKET, SO_PEERCRED, cred, &ucred_length))
		PLOGE("getsockopt");
}

int switch_mnt_ns(int pid) {
	char mnt[32];
	snprintf(mnt, sizeof(mnt), "/proc/%d/ns/mnt", pid);
	if(access(mnt, R_OK) == -1) return 1; // Maybe process died..

	int fd, ret;
	fd = xopen(mnt, O_RDONLY);
	if (fd < 0) return 1;
	// Switch to its namespace
	ret = xsetns(fd, 0);
	close(fd);
	return ret;
}

int fork_dont_care() {
	int pid = xfork();
	if (pid) {
		waitpid(pid, NULL, 0);
		return pid;
	} else if ((pid = xfork())) {
		exit(0);
	}
	return 0;
}

void wait_till_exists(const char *target) {
	if (access(target, F_OK) == 0)
		return;
	int fd = inotify_init();
	char *dir = dirname(target);
	char crap[PATH_MAX];
	inotify_add_watch(fd, dir, IN_CREATE);
	while (1) {
		struct inotify_event event;
		read(fd, &event, sizeof(event));
		read(fd, crap, event.len);
		if (access(target, F_OK) == 0)
			break;
	}
	close(fd);
}

void gen_rand_str(char *buf, int len) {
	const char base[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_.";
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
		*lineptr = malloc(*n);
		if (!*lineptr) {
			errno = ENOMEM;
			return -1;
		}
	}

	nchars_avail = *n;
	read_pos = *lineptr;

	while (1) {
		int save_errno;
		register int c = getc(stream);

		save_errno = errno;

		if (nchars_avail < 2) {
			if (*n > MIN_CHUNK)
				*n *= 2;
			else
				*n += MIN_CHUNK;

			nchars_avail = *n + *lineptr - read_pos;
			*lineptr = realloc(*lineptr, *n);
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
