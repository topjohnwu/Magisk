/* file.c - Contains all files related utilities
 */

#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <libgen.h>
#include <sys/sendfile.h>
#include <sys/mman.h>
#include <linux/fs.h>

#ifdef SELINUX
#include <selinux/selinux.h>
#endif

#include "magisk.h"
#include "utils.h"

char **excl_list = NULL;

static int is_excl(const char *name) {
	if (excl_list)
		for (int i = 0; excl_list[i]; ++i)
			if (strcmp(name, excl_list[i]) == 0)
				return 1;
	return 0;
}

static int fd_getpath(int fd, char *path, size_t size) {
	snprintf(path, size, "/proc/self/fd/%d", fd);
    return xreadlink(path, path, size) == -1;
}

static int fd_getpathat(int dirfd, const char *name, char *path, size_t size) {
    if (fd_getpath(dirfd, path, size))
        return 1;
    snprintf(path, size, "%s/%s", path, name);
    return 0;
}

int mkdirs(const char *pathname, mode_t mode) {
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

void in_order_walk(int dirfd, void (*callback)(int, struct dirent*)) {
	struct dirent *entry;
	int newfd;
	DIR *dir = fdopendir(dirfd);
	if (dir == NULL) return;

	while ((entry = xreaddir(dir))) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			continue;
		if (is_excl(entry->d_name))
			continue;
		if (entry->d_type == DT_DIR) {
			newfd = xopenat(dirfd, entry->d_name, O_RDONLY | O_CLOEXEC);
			in_order_walk(newfd, callback);
			close(newfd);
		}
		callback(dirfd, entry);
	}
}

static void rm_cb(int dirfd, struct dirent *entry) {
	switch (entry->d_type) {
	case DT_DIR:
		unlinkat(dirfd, entry->d_name, AT_REMOVEDIR);
		break;
	default:
		unlinkat(dirfd, entry->d_name, 0);
		break;
	}
}

void rm_rf(const char *path) {
	int fd = open(path, O_RDONLY | O_NOFOLLOW | O_CLOEXEC);
	if (fd >= 0) {
		frm_rf(fd);
		close(fd);
	}
	remove(path);
}

void frm_rf(int dirfd) {
	in_order_walk(dirfd, rm_cb);
}

/* This will only on the same file system */
void mv_f(const char *source, const char *destination) {
	struct stat st;
	xlstat(source, &st);
	int src, dest;
	struct file_attr a;

	if (S_ISDIR(st.st_mode)) {
		xmkdirs(destination, st.st_mode & 0777);
		src = xopen(source, O_RDONLY | O_CLOEXEC);
		dest = xopen(destination, O_RDONLY | O_CLOEXEC);
		fclone_attr(src, dest);
		mv_dir(src, dest);
		close(src);
		close(dest);
	} else{
		getattr(source, &a);
		xrename(source, destination);
		setattr(destination, &a);
	}
	rmdir(source);
}

/* This will only on the same file system */
void mv_dir(int src, int dest) {
	struct dirent *entry;
	DIR *dir;
	int newsrc, newdest;
	struct file_attr a;

	dir = xfdopendir(src);
	while ((entry = xreaddir(dir))) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			continue;
		if (is_excl(entry->d_name))
			continue;
		getattrat(src, entry->d_name, &a);
		switch (entry->d_type) {
		case DT_DIR:
			xmkdirat(dest, entry->d_name, a.st.st_mode & 0777);
			newsrc = xopenat(src, entry->d_name, O_RDONLY | O_CLOEXEC);
			newdest = xopenat(dest, entry->d_name, O_RDONLY | O_CLOEXEC);
			fsetattr(newdest, &a);
			mv_dir(newsrc, newdest);
			close(newsrc);
			close(newdest);
			unlinkat(src, entry->d_name, AT_REMOVEDIR);
			break;
		case DT_LNK:
		case DT_REG:
			renameat(src, entry->d_name, dest, entry->d_name);
			setattrat(dest, entry->d_name, &a);
			break;
		}
	}
}

void cp_afc(const char *source, const char *destination) {
	int src, dest;
	struct file_attr a;
	getattr(source, &a);

	if (S_ISDIR(a.st.st_mode)) {
		xmkdirs(destination, a.st.st_mode & 0777);
		src = xopen(source, O_RDONLY | O_CLOEXEC);
		dest = xopen(destination, O_RDONLY | O_CLOEXEC);
		clone_dir(src, dest);
		close(src);
		close(dest);
	} else{
		unlink(destination);
		if (S_ISREG(a.st.st_mode)) {
			src = xopen(source, O_RDONLY);
			dest = xopen(destination, O_WRONLY | O_CREAT | O_TRUNC);
			xsendfile(dest, src, NULL, a.st.st_size);
			close(src);
			close(dest);
		} else if (S_ISLNK(a.st.st_mode)) {
			char buf[PATH_MAX];
			xreadlink(source, buf, sizeof(buf));
			xsymlink(buf, destination);
		}
	}
	setattr(destination, &a);
}

void clone_dir(int src, int dest) {
	struct dirent *entry;
	DIR *dir;
	int srcfd, destfd, newsrc, newdest;
	char buf[PATH_MAX];
	struct file_attr a;

	dir = xfdopendir(src);
	while ((entry = xreaddir(dir))) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			continue;
		if (is_excl(entry->d_name))
			continue;
		getattrat(src, entry->d_name, &a);
		switch (entry->d_type) {
		case DT_DIR:
			xmkdirat(dest, entry->d_name, a.st.st_mode & 0777);
			setattrat(dest, entry->d_name, &a);
			newsrc = xopenat(src, entry->d_name, O_RDONLY | O_CLOEXEC);
			newdest = xopenat(dest, entry->d_name, O_RDONLY | O_CLOEXEC);
			clone_dir(newsrc, newdest);
			close(newsrc);
			close(newdest);
			break;
		case DT_REG:
			destfd = xopenat(dest, entry->d_name, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC);
			srcfd = xopenat(src, entry->d_name, O_RDONLY | O_CLOEXEC);
			xsendfile(destfd, srcfd, 0, a.st.st_size);
			fsetattr(destfd, &a);
			close(destfd);
			close(srcfd);
			break;
		case DT_LNK:
			xreadlinkat(src, entry->d_name, buf, sizeof(buf));
			xsymlinkat(buf, dest, entry->d_name);
			setattrat(dest, entry->d_name, &a);
			break;
		}
	}
}

void link_dir(int src, int dest) {
	struct dirent *entry;
	DIR *dir;
	int newsrc, newdest;
	struct file_attr a;

	dir = xfdopendir(src);
	while ((entry = xreaddir(dir))) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			continue;
		if (is_excl(entry->d_name))
			continue;
		if (entry->d_type == DT_DIR) {
			getattrat(src, entry->d_name, &a);
			xmkdirat(dest, entry->d_name, a.st.st_mode & 0777);
			setattrat(dest, entry->d_name, &a);
			newsrc = xopenat(src, entry->d_name, O_RDONLY | O_CLOEXEC);
			newdest = xopenat(dest, entry->d_name, O_RDONLY | O_CLOEXEC);
			link_dir(newsrc, newdest);
			close(newsrc);
			close(newdest);
		} else {
			xlinkat(src, entry->d_name, dest, entry->d_name, 0);
		}
	}
}

int getattr(const char *path, struct file_attr *a) {
	if (xlstat(path, &a->st) == -1)
		return -1;
#ifdef SELINUX
	char *con = "";
	if (lgetfilecon(path, &con) == -1)
		return -1;
	strcpy(a->con, con);
	freecon(con);
#else
	a->con[0] = '\0';
#endif
	return 0;
}

int getattrat(int dirfd, const char *pathname, struct file_attr *a) {
	char path[PATH_MAX];
	fd_getpathat(dirfd, pathname, path, sizeof(path));
	return getattr(path, a);
}

int fgetattr(int fd, struct file_attr *a) {
#ifdef SELINUX
	char path[PATH_MAX];
	fd_getpath(fd, path, sizeof(path));
	return getattr(path, a);
#else
	if (fstat(fd, &a->st) == -1)
		return -1;
	a->con[0] = '\0';
	return 0;
#endif
}

int setattr(const char *path, struct file_attr *a) {
	if (chmod(path, a->st.st_mode & 0777) < 0)
		return -1;
	if (chown(path, a->st.st_uid, a->st.st_gid) < 0)
		return -1;
#ifdef SELINUX
	if (strlen(a->con) && lsetfilecon(path, a->con) < 0)
		return -1;
#endif
	return 0;
}

int setattrat(int dirfd, const char *pathname, struct file_attr *a) {
	char path[PATH_MAX];
	fd_getpathat(dirfd, pathname, path, sizeof(path));
	return setattr(path, a);
}

int fsetattr(int fd, struct file_attr *a) {
#ifdef SELINUX
	char path[PATH_MAX];
	fd_getpath(fd, path, sizeof(path));
	return setattr(path, a);
#else
	if (fchmod(fd, a->st.st_mode & 0777) < 0)
		return -1;
	if (fchown(fd, a->st.st_uid, a->st.st_gid) < 0)
		return -1;
	return 0;
#endif
}

void clone_attr(const char *source, const char *target) {
	struct file_attr a;
	getattr(source, &a);
	setattr(target, &a);
}

void fclone_attr(const int sourcefd, const int targetfd) {
	struct file_attr a;
	fgetattr(sourcefd, &a);
	fsetattr(targetfd, &a);
}

#ifdef SELINUX

#include "magiskpolicy.h"

#define UNLABEL_CON "u:object_r:unlabeled:s0"
#define SYSTEM_CON  "u:object_r:system_file:s0"
#define ADB_CON     "u:object_r:adb_data_file:s0"
#define MAGISK_CON  "u:object_r:"SEPOL_FILE_DOMAIN":s0"

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

#endif   // SELINUX

static void _mmap(int rw, const char *filename, void **buf, size_t *size) {
	struct stat st;
	int fd = xopen(filename, (rw ? O_RDWR : O_RDONLY) | O_CLOEXEC);
	fstat(fd, &st);
	if (S_ISBLK(st.st_mode))
		ioctl(fd, BLKGETSIZE64, size);
	else
		*size = st.st_size;
	*buf = *size > 0 ? xmmap(NULL, *size, PROT_READ | (rw ? PROT_WRITE : 0), MAP_SHARED, fd, 0) : NULL;
	close(fd);
}

void mmap_ro(const char *filename, void **buf, size_t *size) {
	_mmap(0, filename, buf, size);
}

void mmap_rw(const char *filename, void **buf, size_t *size) {
	_mmap(1, filename, buf, size);
}

void fd_full_read(int fd, void **buf, size_t *size) {
	*size = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);
	*buf = xmalloc(*size + 1);
	xxread(fd, *buf, *size);
	((char *) *buf)[*size] = '\0';
}

void full_read(const char *filename, void **buf, size_t *size) {
	int fd = xopen(filename, O_RDONLY | O_CLOEXEC);
	if (fd < 0) {
		*buf = NULL;
		*size = 0;
		return;
	}
	fd_full_read(fd, buf, size);
	close(fd);
}

void full_read_at(int dirfd, const char *filename, void **buf, size_t *size) {
	int fd = xopenat(dirfd, filename, O_RDONLY | O_CLOEXEC);
	if (fd < 0) {
		*buf = NULL;
		*size = 0;
		return;
	}
	fd_full_read(fd, buf, size);
	close(fd);
}

void stream_full_read(int fd, void **buf, size_t *size) {
	size_t cap = 1 << 20;
	uint8_t tmp[1 << 20];
	*buf = xmalloc(cap);
	ssize_t read;
	*size = 0;
	while (1) {
		read = xread(fd, tmp, sizeof(tmp));
		if (read <= 0)
			break;
		if (*size + read > cap) {
			cap *= 2;
			*buf = realloc(*buf, cap);
		}
		memcpy(*buf + *size, tmp, read);
		*size += read;
	}
}

void write_zero(int fd, size_t size) {
	size_t pos = lseek(fd, 0, SEEK_CUR);
	ftruncate(fd, pos + size);
	lseek(fd, pos + size, SEEK_SET);
}
