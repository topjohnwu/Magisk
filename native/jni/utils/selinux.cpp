#include <dlfcn.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <syscall.h>
#include <sys/xattr.h>

#include "magisk.h"
#include "utils.h"
#include "selinux.h"

#define UNLABEL_CON "u:object_r:unlabeled:s0"
#define SYSTEM_CON  "u:object_r:system_file:s0"
#define ADB_CON     "u:object_r:adb_data_file:s0"
#define MAGISK_CON  "u:object_r:" SEPOL_FILE_DOMAIN ":s0"

// Stub implementation

static int stub(const char *) { return 0; }

static int stub(const char *, const char *) { return 0; }

static int stub(const char *, char **ctx) {
	*ctx = strdup("");
	return 0;
}

// Builtin implementation

static void __freecon(char *s) {
	free(s);
}

static int __setcon(const char *ctx) {
	int fd = open("/proc/self/attr/current", O_WRONLY | O_CLOEXEC);
	if (fd < 0)
		return fd;
	size_t len = strlen(ctx) + 1;
	int rc = write(fd, ctx, len);
	close(fd);
	return rc != len;
}

static int __getfilecon(const char *path, char **ctx) {
	char buf[1024];
	int rc = syscall(__NR_getxattr, path, XATTR_NAME_SELINUX, buf, sizeof(buf) - 1);
	if (rc > 0)
		*ctx = strdup(buf);
	return rc;
}

static int __lgetfilecon(const char *path, char **ctx) {
	char buf[1024];
	int rc = syscall(__NR_lgetxattr, path, XATTR_NAME_SELINUX, buf, sizeof(buf) - 1);
	if (rc > 0)
		*ctx = strdup(buf);
	return rc;
}

static int __setfilecon(const char *path, const char *ctx) {
	return syscall(__NR_setxattr, path, XATTR_NAME_SELINUX, ctx, strlen(ctx) + 1, 0);
}

static int __lsetfilecon(const char *path, const char *ctx) {
	return syscall(__NR_lsetxattr, path, XATTR_NAME_SELINUX, ctx, strlen(ctx) + 1, 0);
}

// Function pointers

void (*freecon)(char *) = __freecon;
int (*setcon)(const char *) = stub;
int (*getfilecon)(const char *, char **) = stub;
int (*lgetfilecon)(const char *, char **) = stub;
int (*setfilecon)(const char *, const char *) = stub;
int (*lsetfilecon)(const char *, const char *) = stub;

void selinux_builtin_impl() {
	setcon = __setcon;
	getfilecon = __getfilecon;
	lgetfilecon = __lgetfilecon;
	setfilecon = __setfilecon;
	setfilecon = __lsetfilecon;
}

void dload_selinux() {
	void *handle = dlopen("libselinux.so", RTLD_LAZY);
	if (handle == nullptr)
		return;
	*(void **) &freecon = dlsym(handle, "freecon");
	*(void **) &setcon = dlsym(handle, "setcon");
	*(void **) &getfilecon = dlsym(handle, "getfilecon");
	*(void **) &lgetfilecon = dlsym(handle, "lgetfilecon");
	*(void **) &setfilecon = dlsym(handle, "setfilecon");
	*(void **) &lsetfilecon = dlsym(handle, "lsetfilecon");
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
	if (write(fd, ADB_CON, sizeof(ADB_CON)) >= 0)
		lsetfilecon(SECURE_DIR, ADB_CON);
	close(fd);
	fd = xopen(MOUNTPOINT, O_RDONLY | O_CLOEXEC);
	restore_syscon(fd);
	close(fd);
	fd = xopen(DATABIN, O_RDONLY | O_CLOEXEC);
	restore_magiskcon(fd);
	close(fd);
}
