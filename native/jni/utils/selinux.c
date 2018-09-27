#include <dlfcn.h>
#include <unistd.h>

#include "magisk.h"
#include "utils.h"
#include "logging.h"
#include "selinux.h"

#define UNLABEL_CON "u:object_r:unlabeled:s0"
#define SYSTEM_CON  "u:object_r:system_file:s0"
#define ADB_CON     "u:object_r:adb_data_file:s0"
#define MAGISK_CON  "u:object_r:"SEPOL_FILE_DOMAIN":s0"

// Stub implementations

static char *empty_str = "";

static void v_s(char *s) {}

static int i_s(const char *s) { return 0; }

static int i_ss(const char *s, const char * ss) { return 0; }

static int i_ssp(const char *s, char ** sp) {
	*sp = empty_str;
	return 0;
}

// Function pointers

void (*freecon)(char * con) = v_s;
int (*setcon)(const char * con) = i_s;
int (*getfilecon)(const char *path, char ** con) = i_ssp;
int (*lgetfilecon)(const char *path, char ** con) = i_ssp;
int (*setfilecon)(const char *path, const char * con) = i_ss;
int (*lsetfilecon)(const char *path, const char * con) = i_ss;

void setup_selinux() {
	void *handle = dlopen("libselinux.so", RTLD_LAZY);
	if (handle == NULL)
		return;
	freecon = dlsym(handle, "freecon");
	setcon = dlsym(handle, "setcon");
	getfilecon = dlsym(handle, "getfilecon");
	lgetfilecon = dlsym(handle, "lgetfilecon");
	setfilecon = dlsym(handle, "setfilecon");
	lsetfilecon = dlsym(handle, "lsetfilecon");
}

static void restore_syscon(int dirfd) {
	struct dirent *entry;
	DIR *dir;

	char path[PATH_MAX], *con;

	fd_getpath(dirfd, path, sizeof(path));
	size_t len = strlen(path);
	getfilecon(path, &con);
	if (strlen(con) == 0 || strcmp(con, UNLABEL_CON) == 0)
		lsetfilecon(path, SYSTEM_CON);
	freecon(con);

	dir = xfdopendir(dirfd);
	while ((entry = xreaddir(dir))) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			continue;
		if (entry->d_type == DT_DIR) {
			int fd = xopenat(dirfd, entry->d_name, O_RDONLY | O_CLOEXEC);
			restore_syscon(fd);
			close(fd);
		} else {
			path[len] = '/';
			strcpy(path + len + 1, entry->d_name);
			lgetfilecon(path, &con);
			if (strlen(con) == 0 || strcmp(con, UNLABEL_CON) == 0)
				lsetfilecon(path, SYSTEM_CON);
			freecon(con);
			path[len] = '\0';
		}
	}
}

static void restore_magiskcon(int dirfd) {
	struct dirent *entry;
	DIR *dir;

	char path[PATH_MAX];

	fd_getpath(dirfd, path, sizeof(path));
	size_t len = strlen(path);
	lsetfilecon(path, MAGISK_CON);
	lchown(path, 0, 0);

	dir = xfdopendir(dirfd);
	while ((entry = xreaddir(dir))) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			continue;
		if (entry->d_type == DT_DIR) {
			int fd = xopenat(dirfd, entry->d_name, O_RDONLY | O_CLOEXEC);
			restore_magiskcon(fd);
			close(fd);
		} else {
			path[len] = '/';
			strcpy(path + len + 1, entry->d_name);
			lsetfilecon(path, MAGISK_CON);
			lchown(path, 0, 0);
			path[len] = '\0';
		}
	}
}

void restorecon() {
	int fd;
	fd = xopen(SELINUX_CONTEXT, O_WRONLY | O_CLOEXEC);
	if (write(fd, ADB_CON, sizeof(ADB_CON)) >= 0) {
		lsetfilecon(SECURE_DIR, ADB_CON);
	}
	close(fd);
	fd = xopen(MOUNTPOINT, O_RDONLY | O_CLOEXEC);
	restore_syscon(fd);
	close(fd);
	fd = xopen(DATABIN, O_RDONLY | O_CLOEXEC);
	restore_magiskcon(fd);
	close(fd);
}