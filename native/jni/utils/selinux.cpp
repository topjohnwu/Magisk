#include <dlfcn.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <syscall.h>
#include <sys/xattr.h>

#include <magisk.h>
#include <utils.h>
#include <selinux.h>

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

static int stub(int, const char *) { return 0; }

static int stub(int, char **ctx) {
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
	if (rc < 0) {
		errno = -rc;
		return -1;
	}
	*ctx = strdup(buf);
	return rc;
}

static int __lgetfilecon(const char *path, char **ctx) {
	char buf[1024];
	int rc = syscall(__NR_lgetxattr, path, XATTR_NAME_SELINUX, buf, sizeof(buf) - 1);
	if (rc < 0) {
		errno = -rc;
		return -1;
	}
	*ctx = strdup(buf);
	return rc;
}

static int __fgetfilecon(int fd, char **ctx) {
	char buf[1024];
	int rc = syscall(__NR_fgetxattr, fd, XATTR_NAME_SELINUX, buf, sizeof(buf) - 1);
	if (rc < 0) {
		errno = -rc;
		return -1;
	}
	*ctx = strdup(buf);
	return rc;
}

static int __setfilecon(const char *path, const char *ctx) {
	int rc = syscall(__NR_setxattr, path, XATTR_NAME_SELINUX, ctx, strlen(ctx) + 1, 0);
	if (rc) {
		errno = -rc;
		return -1;
	}
	return 0;
}

static int __lsetfilecon(const char *path, const char *ctx) {
	int rc = syscall(__NR_lsetxattr, path, XATTR_NAME_SELINUX, ctx, strlen(ctx) + 1, 0);
	if (rc) {
		errno = -rc;
		return -1;
	}
	return 0;
}

static int __fsetfilecon(int fd, const char *ctx) {
	int rc = syscall(__NR_fsetxattr, fd, XATTR_NAME_SELINUX, ctx, strlen(ctx) + 1, 0);
	if (rc) {
		errno = -rc;
		return -1;
	}
	return 0;
}

// Function pointers

void (*freecon)(char *) = __freecon;
int (*setcon)(const char *) = stub;
int (*getfilecon)(const char *, char **) = stub;
int (*lgetfilecon)(const char *, char **) = stub;
int (*fgetfilecon)(int, char **) = stub;
int (*setfilecon)(const char *, const char *) = stub;
int (*lsetfilecon)(const char *, const char *) = stub;
int (*fsetfilecon)(int, const char *) = stub;

void getfilecon_at(int dirfd, const char *name, char **con) {
	char path[4096];
	fd_pathat(dirfd, name, path, sizeof(path));
	if (lgetfilecon(path, con))
		*con = strdup("");
}

void setfilecon_at(int dirfd, const char *name, const char *con) {
	char path[4096];
	fd_pathat(dirfd, name, path, sizeof(path));
	lsetfilecon(path, con);
}

void selinux_builtin_impl() {
	setcon = __setcon;
	getfilecon = __getfilecon;
	lgetfilecon = __lgetfilecon;
	fgetfilecon = __fgetfilecon;
	setfilecon = __setfilecon;
	lsetfilecon = __lsetfilecon;
	fsetfilecon = __fsetfilecon;
}

void dload_selinux() {
	if (access("/system/lib/libselinux.so", F_OK))
		return;
	/* We only check whether libselinux.so exists but don't dlopen.
	 * For some reason calling symbols returned from dlsym
	 * will result to SEGV_ACCERR on some devices.
	 * Always use builtin implementations for SELinux stuffs. */
	selinux_builtin_impl();
}

static void restore_syscon(int dirfd) {
	struct dirent *entry;
	DIR *dir;
	char *con;

	fgetfilecon(dirfd, &con);
	if (strlen(con) == 0 || strcmp(con, UNLABEL_CON) == 0)
		fsetfilecon(dirfd, SYSTEM_CON);
	freecon(con);

	dir = xfdopendir(dirfd);
	while ((entry = xreaddir(dir))) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			continue;
		int fd = openat(dirfd, entry->d_name, O_RDONLY | O_CLOEXEC);
		if (entry->d_type == DT_DIR) {
			restore_syscon(fd);
		} else if (entry->d_type == DT_REG) {
			fgetfilecon(fd, &con);
			if (con[0] == '\0' || strcmp(con, UNLABEL_CON) == 0)
				fsetfilecon(fd, SYSTEM_CON);
			freecon(con);
		} else if (entry->d_type == DT_LNK) {
			getfilecon_at(dirfd, entry->d_name, &con);
			if (con[0] == '\0' || strcmp(con, UNLABEL_CON) == 0)
				setfilecon_at(dirfd, entry->d_name, con);
			freecon(con);
		}
		close(fd);
	}
}

static void restore_magiskcon(int dirfd) {
	struct dirent *entry;
	DIR *dir;

	fsetfilecon(dirfd, MAGISK_CON);
	fchown(dirfd, 0, 0);

	dir = xfdopendir(dirfd);
	while ((entry = xreaddir(dir))) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			continue;
		int fd = xopenat(dirfd, entry->d_name, O_RDONLY | O_CLOEXEC);
		if (entry->d_type == DT_DIR) {
			restore_magiskcon(fd);
		} else if (entry->d_type) {
			fsetfilecon(fd, MAGISK_CON);
			fchown(fd, 0, 0);
		}
		close(fd);
	}
}

void restorecon() {
	int fd;
	fd = xopen(SELINUX_CONTEXT, O_WRONLY | O_CLOEXEC);
	if (write(fd, ADB_CON, sizeof(ADB_CON)) >= 0)
		lsetfilecon(SECURE_DIR, ADB_CON);
	close(fd);
	lsetfilecon(MODULEROOT, SYSTEM_CON);
	fd = xopen(MODULEROOT, O_RDONLY | O_CLOEXEC);
	restore_syscon(fd);
	close(fd);
	fd = xopen(DATABIN, O_RDONLY | O_CLOEXEC);
	restore_magiskcon(fd);
	close(fd);
}
