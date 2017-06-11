/* misc.c - Store all functions that are unable to be catagorized clearly
 */

#define _GNU_SOURCE
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
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
int isNum(const char *s) {
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
  *fd == 0    -> Open pipe and set *fd to the read end
  *fd != 0    -> STDOUT (or STDERR) will be redirected to *fd
*/
int run_command(int err, int *fd, const char *path, char *const argv[]) {
	int pipefd[2], writeEnd = 0;

	if (fd) {
		if (*fd == 0) {
			if (pipe(pipefd) == -1)
				return -1;
			writeEnd = pipefd[1];
			// Give the read end of the pipe
			*fd = pipefd[0];
		}
	}

	int pid = fork();
	if (pid != 0) {
		if (writeEnd) close(writeEnd);
		return pid;
	}

	if (fd) {
		if (writeEnd == 0) writeEnd = *fd;
		xdup2(writeEnd, STDOUT_FILENO);
		if (err) xdup2(writeEnd, STDERR_FILENO);
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
#ifdef DEBUG
	LOGD("bind_mount: %s -> %s\n", from, to);
#else
	LOGI("bind_mount: %s\n", to);
#endif
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
	if (access(target, F_OK) == -1)
		return 0;
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

int create_img(const char *img, int size) {
	unlink(img);
	LOGI("Create %s with size %dM\n", img, size);
	// Create a temp file with the file contexts
	char file_contexts[] = "/magisk(/.*)? u:object_r:system_file:s0\n";
	int fd = xopen("/dev/file_contexts_image", O_WRONLY | O_CREAT | O_TRUNC, 0644);
	xwrite(fd, file_contexts, sizeof(file_contexts));
	close(fd);

	char buffer[PATH_MAX];
	snprintf(buffer, sizeof(buffer),
		"make_ext4fs -l %dM -a /magisk -S /dev/file_contexts_image %s; e2fsck -yf %s;", size, img, img);
	char *const command[] = { "sh", "-c", buffer, NULL };
	int pid, status;
	pid = run_command(0, NULL, "/system/bin/sh", command);
	if (pid == -1)
		return 1;
	waitpid(pid, &status, 0);
	unlink("/dev/file_contexts_image");
	return WEXITSTATUS(status);
}

int get_img_size(const char *img, int *used, int *total) {
	if (access(img, R_OK) == -1)
		return 1;
	char buffer[PATH_MAX];
	snprintf(buffer, sizeof(buffer), "e2fsck -yf %s", img);
	char *const command[] = { "sh", "-c", buffer, NULL };
	int pid, fd = 0, status = 1;
	pid = run_command(1, &fd, "/system/bin/sh", command);
	if (pid == -1)
		return 1;
	while (fdgets(buffer, sizeof(buffer), fd)) {
		LOGD("magisk_img: %s", buffer);
		if (strstr(buffer, img)) {
			char *tok;
			tok = strtok(buffer, ",");
			while(tok != NULL) {
				if (strstr(tok, "blocks")) {
					status = 0;
					break;
				}
				tok = strtok(NULL, ",");
			}
			if (status) continue;
			sscanf(tok, "%d/%d", used, total);
			*used = *used / 256 + 1;
			*total /= 256;
			break;
		}
	}
	close(fd);
	waitpid(pid, &status, 0);
	return WEXITSTATUS(status);
}

int resize_img(const char *img, int size) {
	LOGI("Resize %s to %dM\n", img, size);
	char buffer[PATH_MAX];
	snprintf(buffer, sizeof(buffer), "resize2fs %s %dM; e2fsck -yf %s;", img, size, img);
	char *const command[] = { "sh", "-c", buffer, NULL };
	int pid, status, fd = 0;
	pid = run_command(1, &fd, "/system/bin/sh", command);
	if (pid == -1)
		return 1;
	while (fdgets(buffer, sizeof(buffer), fd))
		LOGD("magisk_img: %s", buffer);
	close(fd);
	waitpid(pid, &status, 0);
	return WEXITSTATUS(status);
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
