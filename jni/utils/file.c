/* file.c - Contains all files related utilities
 */

#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/sendfile.h>

#ifndef NO_SELINUX
#include <selinux/selinux.h>
#endif

#include "utils.h"

char **excl_list = (char *[]) { NULL };

static int is_excl(const char *name) {
	for (int i = 0; excl_list[i]; ++i) {
		if (strcmp(name, excl_list[i]) == 0)
			return 1;
	}
	return 0;
}

int fd_getpath(int fd, char *path, size_t size) {
	snprintf(path, size, "/proc/self/fd/%d", fd);
	if (xreadlink(path, path, size) == -1)
		return -1;
	return 0;
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

void rm_rf(const char *path) {
	int fd = xopen(path, O_RDONLY | O_CLOEXEC);
	frm_rf(fd);
	close(fd);
	rmdir(path);
}

void frm_rf(int dirfd) {
	struct dirent *entry;
	int newfd;
	DIR *dir = xfdopendir(dirfd);

	while ((entry = xreaddir(dir))) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			continue;
		if (is_excl(entry->d_name))
			continue;
		switch (entry->d_type) {
		case DT_DIR:
			newfd = xopenat(dirfd, entry->d_name, O_RDONLY | O_CLOEXEC);
			frm_rf(newfd);
			close(newfd);
			unlinkat(dirfd, entry->d_name, AT_REMOVEDIR);
			break;
		default:
			unlinkat(dirfd, entry->d_name, 0);
			break;
		}
	}
}

/* This will only on the same file system */
void mv_f(const char *source, const char *destination) {
	struct stat st;
	xlstat(source, &st);
	int src, dest;
	struct file_attr a;

	if (S_ISDIR(st.st_mode)) {
		xmkdir_p(destination, st.st_mode & 0777);
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
		xmkdir_p(destination, a.st.st_mode & 0777);
		src = xopen(source, O_RDONLY | O_CLOEXEC);
		dest = xopen(destination, O_RDONLY | O_CLOEXEC);
		fsetattr(dest, &a);
		clone_dir(src, dest);
		close(src);
		close(dest);
	} else{
		unlink(destination);
		if (S_ISREG(a.st.st_mode)) {
			src = xopen(source, O_RDONLY);
			dest = xopen(destination, O_WRONLY | O_CREAT | O_TRUNC);
			xsendfile(dest, src, NULL, a.st.st_size);
			fsetattr(src, &a);
			close(src);
			close(dest);
		} else if (S_ISLNK(a.st.st_mode)) {
			char buf[PATH_MAX];
			xreadlink(source, buf, sizeof(buf));
			xsymlink(buf, destination);
			setattr(destination, &a);
		}
	}
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
			symlinkat(buf, dest, entry->d_name);
			setattrat(dest, entry->d_name, &a);
			break;
		}
	}
}

int getattr(const char *path, struct file_attr *a) {
	if (xlstat(path, &a->st) == -1)
		return -1;
	char *con = "";
#ifndef NO_SELINUX
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
	int fd = xopenat(dirfd, pathname, O_PATH | O_NOFOLLOW | O_CLOEXEC);
	if (fd < 0)
		return -1;
	int ret = fgetattr(fd, a);
	close(fd);
	return ret;
}

int fgetattr(int fd, struct file_attr *a) {
#ifndef NO_SELINUX
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
#ifndef NO_SELINUX
	if (strlen(a->con) && lsetfilecon(path, a->con) < 0)
		return -1;
#endif
	return 0;
}

int setattrat(int dirfd, const char *pathname, struct file_attr *a) {
	int fd = xopenat(dirfd, pathname, O_PATH | O_NOFOLLOW | O_CLOEXEC);
	if (fd < 0)
		return -1;
	int ret = fsetattr(fd, a);
	close(fd);
	return ret;
}

int fsetattr(int fd, struct file_attr *a) {
#ifndef NO_SELINUX
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

#ifndef NO_SELINUX

#define UNLABEL_CON "u:object_r:unlabeled:s0"
#define SYSTEM_CON  "u:object_r:system_file:s0"

void restorecon(int dirfd, int force) {
	struct dirent *entry;
	DIR *dir;
	int fd;
	char path[PATH_MAX], *con;

	fd_getpath(dirfd, path, sizeof(path));
	lgetfilecon(path, &con);
	if (force || strlen(con) == 0 || strcmp(con, UNLABEL_CON) == 0)
		lsetfilecon(path, SYSTEM_CON);
	freecon(con);

	dir = xfdopendir(dirfd);
	while ((entry = xreaddir(dir))) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			continue;
		if (entry->d_type == DT_DIR) {
			fd = xopenat(dirfd, entry->d_name, O_RDONLY | O_CLOEXEC);
			restorecon(fd, force);
		} else {
			fd = xopenat(dirfd, entry->d_name, O_PATH | O_NOFOLLOW | O_CLOEXEC);
			fd_getpath(fd, path, sizeof(path));
			lgetfilecon(path, &con);
			if (force || strlen(con) == 0 || strcmp(con, UNLABEL_CON) == 0)
				lsetfilecon(path, SYSTEM_CON);
			freecon(con);
		}
		close(fd);
	}
}

#endif   // NO_SELINUX
