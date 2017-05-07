/* misc.c - Store all functions that are unable to be catagorized clearly
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <pwd.h>
#include <signal.h>
#include <errno.h>
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
int isNum(const char *s) {
	int len = strlen(s);
	for (int i = 0; i < len; ++i)
		if (s[i] < '0' || s[i] > '9')
			return 0;
	return 1;
}

/* Read a whole line from file descriptor */
ssize_t fdgets(char *buf, const size_t size, int fd) {
	ssize_t read = 0;
	buf[0] = '\0';
	while (xread(fd, buf + read, 1) && read < size - 1) {
		if (buf[read] == '\0' || buf[read++] == '\n') {
			buf[read] = '\0';
			break;
		}
	}
	buf[size - 1] = '\0';
	return read;
}

/* Call func for each process */
void ps(void (*func)(int)) {
	DIR *dir;
	struct dirent *entry;

	if (!(dir = xopendir("/proc")))
		return;

	while ((entry = xreaddir(dir))) {
		if (entry->d_type == DT_DIR) {
			if (isNum(entry->d_name))
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
		if (entry->d_type == DT_BLK && strstr(entry->d_name, "mmc") != NULL) {
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

int run_command(int *fd, const char *path, char *const argv[]) {
	int sv[2];

	if (fd) {
		if (socketpair(AF_LOCAL, SOCK_STREAM, 0, sv) == -1)
			return -1;
		// We use sv[0], give them sv[1] for communication
		if (fcntl(sv[1], F_SETFD, FD_CLOEXEC))
			PLOGE("fcntl FD_CLOEXEC");
		*fd = sv[1];
	}

	int pid = fork();
	if (pid != 0) {
		if (fd) close(sv[0]);
		return pid;
	}

	if (fd) {
		close(sv[1]);
		xdup2(sv[0], STDIN_FILENO);
		xdup2(sv[0], STDOUT_FILENO);
		xdup2(sv[0], STDERR_FILENO);
	}

	execv(path, argv);
	PLOGE("execv");
	return -1;
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
	LOGD("bind_mount: %s -> %s\n", from, to);
	LOGI("bind_mount: %s\n", to);
	return ret;
}

int open_new(const char *filename) {
	return xopen(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
}

// file/link -> file/link only!!
int cp_afc(const char *source, const char *target) {
	struct stat buf;
	xlstat(source, &buf);
	unlink(target);
	char *con;
	if (S_ISREG(buf.st_mode)) {
		int sfd, tfd;
		sfd = xopen(source, O_RDONLY);
		tfd = xopen(target, O_WRONLY | O_CREAT | O_TRUNC);
		xsendfile(tfd, sfd, NULL, buf.st_size);
		fchmod(tfd, buf.st_mode & 0777);
		fchown(tfd, buf.st_uid, buf.st_gid);
		fgetfilecon(sfd, &con);
		fsetfilecon(tfd, con);
		free(con);
		close(sfd);
		close(tfd);
	} else if (S_ISLNK(buf.st_mode)) {
		char buffer[PATH_MAX];
		xreadlink(source, buffer, sizeof(buffer));
		xsymlink(buffer, target);
		lgetfilecon(source, &con);
		lsetfilecon(target, con);
		free(con);
	} else {
		return 1;
	}
	return 0;
}

int clone_dir(const char *source, const char *target) {
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
		switch (entry->d_type) {
		case DT_DIR:
			clone_dir(s_path, t_path);
			break;
		case DT_REG:
		case DT_LNK:
			cp_afc(s_path, t_path);
			break;
		}
	}
	free(s_path);
	free(t_path);

	closedir(dir);
	return 0;
}

int rm_rf(const char *target) {
	struct stat buf;
	xlstat(target, &buf);
	char *next;
	if (S_ISDIR(buf.st_mode)) {
		DIR *dir;
		struct dirent *entry;
		if (!(dir = xopendir(target)))
			return 1;
		next = xmalloc(PATH_MAX);
		while ((entry = xreaddir(dir))) {
			if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
				continue;
			snprintf(next, PATH_MAX, "%s/%s", target, entry->d_name);
			switch (entry->d_type) {
			case DT_DIR:
				rm_rf(next);
				break;
			case DT_REG:
			case DT_LNK:
				unlink(next);
				break;
			}
		}
		free(next);
		closedir(dir);
		rmdir(target);
	} else if (S_ISREG(buf.st_mode) || S_ISLNK(buf.st_mode)) {
		unlink(target);
	} else {
		return 1;
	}
	return 0;
}

void clone_attr(const char *source, const char *target) {
	struct stat buf;
	xstat(source, &buf);
	chmod(target, buf.st_mode & 0777);
	chown(target, buf.st_uid, buf.st_gid);
	char *con;
	lgetfilecon(source, &con);
	lsetfilecon(target, con);
	free(con);
}

void get_client_cred(int fd, struct ucred *cred) {
	socklen_t ucred_length = sizeof(*cred);
	if(getsockopt(fd, SOL_SOCKET, SO_PEERCRED, cred, &ucred_length))
		PLOGE("getsockopt");
}
