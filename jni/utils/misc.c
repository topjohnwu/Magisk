/* misc.c - Store all functions that are unable to be catagorized clearly
 */

#define _GNU_SOURCE
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <pwd.h>
#include <signal.h>
#include <errno.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <selinux/selinux.h>

#include "magisk.h"
#include "utils.h"

int quit_signals[] = { SIGALRM, SIGABRT, SIGHUP, SIGPIPE, SIGQUIT, SIGTERM, SIGINT, 0 };

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
	int ret = 0;
	char buffer[4096];
	FILE *fp = xfopen("/proc/mounts", "r");
	while (fgets(buffer, sizeof(buffer), fp)) {
		if (strstr(buffer, " /data ")) {
			if (strstr(buffer, "tmpfs") == NULL)
				ret = 1;
			break;
		}
	}
	fclose(fp);
	return ret;
}

/* All the string should be freed manually!! */
int file_to_vector(const char* filename, struct vector *v) {
	char *line = NULL;
	size_t len = 0;
	ssize_t read;

	FILE *fp = fopen(filename, "r");
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
	while (read(fd, buf + len, 1) >= 0 && len < size - 1) {
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
	if ((fd = open(buf, O_RDONLY)) == -1)
		return;
	if (fdgets(buf, sizeof(buf), fd) == 0) {
		snprintf(buf, sizeof(buf), "/proc/%d/comm", pid);
		close(fd);
		if ((fd = open(buf, O_RDONLY)) == -1)
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

int create_links(const char *bin, const char *path) {
	char self[PATH_MAX], linkpath[PATH_MAX];
	if (bin == NULL) {
		xreadlink("/proc/self/exe", self, sizeof(self));
		bin = self;
	}
	int ret = 0;
	for (int i = 0; applet[i]; ++i) {
		snprintf(linkpath, sizeof(linkpath), "%s/%s", path, applet[i]);
		unlink(linkpath);
		ret |= symlink(bin, linkpath);
	}
	return ret;
}

#define DEV_BLOCK "/dev/block"

void unlock_blocks() {
	char path[PATH_MAX];
	DIR *dir;
	struct dirent *entry;
	int fd, OFF = 0;

	if (!(dir = xopendir(DEV_BLOCK)))
		return;

	while((entry = readdir(dir))) {
		if (entry->d_type == DT_BLK &&
			strstr(entry->d_name, "ram") == NULL &&
			strstr(entry->d_name, "loop") == NULL &&
			strstr(entry->d_name, "dm-0") == NULL) {
			snprintf(path, sizeof(path), "%s/%s", DEV_BLOCK, entry->d_name);
			if ((fd = xopen(path, O_RDONLY)) < 0)
				continue;

			if (ioctl(fd, BLKROSET, &OFF) == -1)
				PLOGE("unlock %s", path);
			close(fd);
		}
	}

	closedir(dir);
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
static int v_exec_command(int err, int *fd, void (*cb)(void), const char *argv0, va_list argv) {
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

	int pid = fork();
	if (pid != 0) {
		if (fd && *fd < 0) {
			// Give the read end and close write end
			*fd = pipefd[0];
			close(pipefd[1]);
		}
		return pid;
	}

	// Don't affect the daemon if anything wrong happens
	err_handler = do_nothing;

	if (cb) cb();

	if (fd) {
		xdup2(writeEnd, STDOUT_FILENO);
		if (err) xdup2(writeEnd, STDERR_FILENO);
	}

	// Collect va_list into vector
	struct vector v;
	vec_init(&v);
	vec_push_back(&v, (void *) argv0);
	for (void *arg = va_arg(argv, void*); arg; arg = va_arg(argv, void*))
		vec_push_back(&v, arg);
	vec_push_back(&v, NULL);

	execvp(argv0, (char **) vec_entry(&v));
	PLOGE("execvp");
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

int exec_command(int err, int *fd, void (*cb)(void), const char *argv0, ...) {
	va_list argv;
	va_start(argv, argv0);
	int pid = v_exec_command(err, fd, cb, argv0, argv);
	va_end(argv);
	return pid;
}

int mkdir_p(const char *pathname, mode_t mode) {
	char *path = strdup(pathname), *p;
	errno = 0;
	for (p = path + 1; *p; ++p) {
		if (*p == '/') {
			*p = '\0';
			if (mkdir(path, mode) == -1) {
				if (errno != EEXIST)
					return -1;
			}
			*p = '/';
		}
	}
	if (mkdir(path, mode) == -1) {
		if (errno != EEXIST)
			return -1;
	}
	free(path);
	return 0;
}

int bind_mount(const char *from, const char *to) {
	int ret = xmount(from, to, NULL, MS_BIND, NULL);
#ifdef MAGISK_DEBUG
	LOGD("bind_mount: %s -> %s\n", from, to);
#else
	LOGI("bind_mount: %s\n", to);
#endif
	return ret;
}

int open_new(const char *filename) {
	return xopen(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
}

int cp_afc(const char *source, const char *target) {
	struct stat buf;
	xlstat(source, &buf);

	if (S_ISDIR(buf.st_mode)) {
		DIR *dir;
		struct dirent *entry;
		char *s_path, *t_path;

		if (!(dir = xopendir(source)))
			return 1;

		s_path = xmalloc(PATH_MAX);
		t_path = xmalloc(PATH_MAX);

		mkdir_p(target, 0755);
		clone_attr(source, target);

		while ((entry = xreaddir(dir))) {
			if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
				continue;
			snprintf(s_path, PATH_MAX, "%s/%s", source, entry->d_name);
			snprintf(t_path, PATH_MAX, "%s/%s", target, entry->d_name);
			cp_afc(s_path, t_path);
		}
		free(s_path);
		free(t_path);

		closedir(dir);
	} else{
		unlink(target);
		if (S_ISREG(buf.st_mode)) {
			int sfd, tfd;
			sfd = xopen(source, O_RDONLY);
			tfd = xopen(target, O_WRONLY | O_CREAT | O_TRUNC);
			xsendfile(tfd, sfd, NULL, buf.st_size);
			fclone_attr(sfd, tfd);
			close(sfd);
			close(tfd);
		} else if (S_ISLNK(buf.st_mode)) {
			char buffer[PATH_MAX];
			xreadlink(source, buffer, sizeof(buffer));
			xsymlink(buffer, target);
			clone_attr(source, target);
		} else {
			return 1;
		}
	}
	return 0;
}

void clone_attr(const char *source, const char *target) {
	struct stat buf;
	lstat(target, &buf);
	chmod(target, buf.st_mode & 0777);
	chown(target, buf.st_uid, buf.st_gid);
	char *con;
	lgetfilecon(source, &con);
	lsetfilecon(target, con);
	free(con);
}

void fclone_attr(const int sourcefd, const int targetfd) {
	struct stat buf;
	fstat(sourcefd, &buf);
	fchmod(targetfd, buf.st_mode & 0777);
	fchown(targetfd, buf.st_uid, buf.st_gid);
	char *con;
	fgetfilecon(sourcefd, &con);
	fsetfilecon(targetfd, con);
	free(con);
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
	ret = setns(fd, 0);
	close(fd);
	return ret;
}
