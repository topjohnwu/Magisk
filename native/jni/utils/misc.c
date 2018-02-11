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

// Internal usage
static void (*ps_filter_cb)(int);
static const char *ps_filter_pattern;
static void proc_name_filter(int pid) {
	char buf[64];
	int fd;
	snprintf(buf, sizeof(buf), "/proc/%d/cmdline", pid);
	if (access(buf, R_OK) == -1 || (fd = xopen(buf, O_RDONLY)) == -1)
		return;
	if (fdgets(buf, sizeof(buf), fd) == 0) {
		snprintf(buf, sizeof(buf), "/proc/%d/comm", pid);
		close(fd);
		if (access(buf, R_OK) == -1 || (fd = xopen(buf, O_RDONLY)) == -1)
			return;
		fdgets(buf, sizeof(buf), fd);
	}
	if (strcmp(buf, ps_filter_pattern) == 0) {
		ps_filter_cb(pid);
	}
	close(fd);
}

/* Call func with process name filtered with pattern */
void ps_filter_proc_name(const char *pattern, void (*func)(int)) {
	ps_filter_cb = func;
	ps_filter_pattern = ((pattern == NULL) ? "" : pattern);
	ps(proc_name_filter);
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
  *cb         -> A callback function which runs after fork
*/
static int v_exec_command(int err, int *fd, void (*setupenv)(struct vector*), const char *argv0, va_list argv) {
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

	// Collect va_list into vector
	struct vector args;
	vec_init(&args);
	vec_push_back(&args, strdup(argv0));
	for (void *arg = va_arg(argv, void*); arg; arg = va_arg(argv, void*))
		vec_push_back(&args, strdup(arg));
	vec_push_back(&args, NULL);

	// Setup environment
	char *const *envp;
	struct vector env;
	vec_init(&env);
	if (setupenv) {
		setupenv(&env);
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
		vec_deep_destroy(&args);
		vec_deep_destroy(&env);
		return pid;
	}

	if (fd) {
		xdup2(writeEnd, STDOUT_FILENO);
		if (err) xdup2(writeEnd, STDERR_FILENO);
	}

	execvpe(argv0, (char **) vec_entry(&args), envp);
	PLOGE("execvpe");
	return -1;
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

int exec_command(int err, int *fd, void (*setupenv)(struct vector*), const char *argv0, ...) {
	va_list argv;
	va_start(argv, argv0);
	int pid = v_exec_command(err, fd, setupenv, argv0, argv);
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
