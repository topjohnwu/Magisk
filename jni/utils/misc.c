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
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <sys/wait.h>

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
	char buffer[4096];
	FILE *fp = xfopen("/proc/mounts", "r");
	while (fgets(buffer, sizeof(buffer), fp)) {
		if (strstr(buffer, " /data ")) {
			if (strstr(buffer, "tmpfs"))
				return 0;
			else
				return 1;
		}
	}
	return 0;
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
	if (strstr(buf, ps_filter_pattern)) {
		// printf("%d: %s\n", pid, buf);
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

void unblock_boot_process() {
	int fd = open("/dev/.magisk.unblock", O_RDONLY | O_CREAT);
	close(fd);
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
	} else {
		int null = open("/dev/null", O_RDWR);
		xdup2(null, STDIN_FILENO);
		xdup2(null, STDOUT_FILENO);
		xdup2(null, STDERR_FILENO);
	}

	execv(path, argv);
	PLOGE("execv");
	return -1;
}

#define MAGISK_CORE "/magisk/.core/"

void exec_common_script(const char* stage) {
	DIR *dir;
	struct dirent *entry;
	char buf[PATH_MAX];
	snprintf(buf, sizeof(buf), MAGISK_CORE "%s.d", stage);

	if (!(dir = opendir(buf)))
		return;

	while ((entry = xreaddir(dir))) {
		if (entry->d_type == DT_REG) {
			snprintf(buf, sizeof(buf), MAGISK_CORE "%s.d/%s", stage, entry->d_name);
			if (access(buf, X_OK) == -1)
				continue;
			LOGI("%s.d: exec [%s]\n", stage, entry->d_name);
			char *const command[] = { "sh", buf, NULL };
			int pid = run_command(NULL, "/system/bin/sh", command);
			if (pid != -1)
				waitpid(pid, NULL, 0);
		}
	}

	closedir(dir);
}
