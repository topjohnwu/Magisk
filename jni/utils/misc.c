/* misc.c - Store all functions that are unable to be catagorized clearly
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/types.h>

#include "utils.h"

int check_data() {
	FILE *fp = xfopen("/proc/mounts", "r");
	while (fgets(magiskbuf, BUF_SIZE, fp)) {
		if (strstr(magiskbuf, " /data ")) {
			if (strstr(magiskbuf, "tmpfs"))
				return 0;
			else
				return 1;
		}
	}
	return 0;
}

/* All the string should be freed manually!! */
void file_to_vector(struct vector *v, FILE *fp) {
	char *line = NULL;
	size_t len = 0;
	ssize_t read;

	while ((read = getline(&line, &len, fp)) != -1) {
		// Remove end newline
		if (line[read - 1] == '\n')
			line[read - 1] = '\0';
		vec_push_back(v, line);
		line = NULL;
	}
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
ssize_t fdreadline(int fd, char *buf, size_t size) {
	ssize_t read = 0;
	buf[0] = '\0';
	while (xread(fd, buf + read, 1)) {
		if (buf[read] == '\n')
			buf[read] = '\0';
		if (buf[read++] == '\0')
			break;
		if (read == size) {
			buf[read - 1] = '\0';
			break;
		}
	}
	return read;
}

/* Call func for each process */
void ps(void (*func)(int)) {
	DIR *dir;
	struct dirent *entry;

	dir = xopendir("/proc");

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
	snprintf(buf, sizeof(buf), "/proc/%d/cmdline", pid);
	int fd = xopen(buf, O_RDONLY);
	if (fdreadline(fd, buf, sizeof(buf)) == 0) {
		snprintf(buf, sizeof(buf), "/proc/%d/comm", pid);
		close(fd);
		fd = xopen(buf, O_RDONLY);
		fdreadline(fd, buf, sizeof(buf));
	}
	if (strstr(buf, ps_filter_pattern)) {
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
