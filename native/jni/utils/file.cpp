/* file.cpp - Contains all files related utilities
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

#include <magisk.h>
#include <utils.h>
#include <selinux.h>

using namespace std;

ssize_t fd_path(int fd, char *path, size_t size) {
	snprintf(path, size, "/proc/self/fd/%d", fd);
	return xreadlink(path, path, size);
}

int fd_pathat(int dirfd, const char *name, char *path, size_t size) {
	ssize_t len = fd_path(dirfd, path, size);
	if (len < 0)
		return -1;
	path[len] = '/';
	strlcpy(&path[len + 1], name, size - len - 1);
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

static bool is_excl(initializer_list<const char *> excl, const char *name) {
	for (auto item : excl)
		if (strcmp(item, name) == 0)
			return true;
	return false;
}

static void post_order_walk(int dirfd, initializer_list<const char *> excl,
		int (*fn)(int, struct dirent *)) {
	struct dirent *entry;
	int newfd;
	DIR *dir = fdopendir(dirfd);
	if (dir == nullptr) return;

	while ((entry = xreaddir(dir))) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			continue;
		if (is_excl(excl, entry->d_name))
			continue;
		if (entry->d_type == DT_DIR) {
			newfd = xopenat(dirfd, entry->d_name, O_RDONLY | O_CLOEXEC);
			post_order_walk(newfd, excl, fn);
			close(newfd);
		}
		fn(dirfd, entry);
	}
}

static int remove_at(int dirfd, struct dirent *entry) {
	return unlinkat(dirfd, entry->d_name, entry->d_type == DT_DIR ? AT_REMOVEDIR : 0);
}

void rm_rf(const char *path) {
	struct stat st;
	if (lstat(path, &st) < 0)
		return;
	if (S_ISDIR(st.st_mode)) {
		int fd = open(path, O_RDONLY | O_CLOEXEC);
		frm_rf(fd);
		close(fd);
	}
	remove(path);
}

void frm_rf(int dirfd, initializer_list<const char *> excl) {
	post_order_walk(dirfd, excl, remove_at);
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
			xsendfile(dest, src, nullptr, a.st.st_size);
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

void clone_dir(int src, int dest, bool overwrite) {
	struct dirent *entry;
	DIR *dir;
	int srcfd, destfd, newsrc, newdest;
	char buf[PATH_MAX];
	struct file_attr a;

	dir = xfdopendir(src);
	while ((entry = xreaddir(dir))) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			continue;
		if (struct stat st; !overwrite &&
				fstatat(dest, entry->d_name, &st, AT_SYMLINK_NOFOLLOW) == 0)
			continue;
		getattrat(src, entry->d_name, &a);
		switch (entry->d_type) {
		case DT_DIR:
			xmkdirat(dest, entry->d_name, a.st.st_mode & 0777);
			setattrat(dest, entry->d_name, &a);
			newsrc = xopenat(src, entry->d_name, O_RDONLY | O_CLOEXEC);
			newdest = xopenat(dest, entry->d_name, O_RDONLY | O_CLOEXEC);
			clone_dir(newsrc, newdest, overwrite);
			close(newsrc);
			close(newdest);
			break;
		case DT_REG:
			destfd = xopenat(dest, entry->d_name, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC);
			srcfd = xopenat(src, entry->d_name, O_RDONLY | O_CLOEXEC);
			xsendfile(destfd, srcfd, nullptr, a.st.st_size);
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
	char *con;
	if (lgetfilecon(path, &con) == -1)
		return -1;
	strcpy(a->con, con);
	freecon(con);
	return 0;
}

int getattrat(int dirfd, const char *name, struct file_attr *a) {
	char path[4096];
	fd_pathat(dirfd, name, path, sizeof(path));
	return getattr(path, a);
}

int fgetattr(int fd, struct file_attr *a) {
	if (xfstat(fd, &a->st) < 0)
		return -1;
	char *con;
	if (fgetfilecon(fd, &con) < 0)
		return -1;
	strcpy(a->con, con);
	freecon(con);
	return 0;
}

int setattr(const char *path, struct file_attr *a) {
	if (chmod(path, a->st.st_mode & 0777) < 0)
		return -1;
	if (chown(path, a->st.st_uid, a->st.st_gid) < 0)
		return -1;
	if (a->con[0] && lsetfilecon(path, a->con) < 0)
		return -1;
	return 0;
}

int setattrat(int dirfd, const char *name, struct file_attr *a) {
	char path[4096];
	fd_pathat(dirfd, name, path, sizeof(path));
	return setattr(path, a);
}

int fsetattr(int fd, struct file_attr *a) {
	if (fchmod(fd, a->st.st_mode & 0777) < 0)
		return -1;
	if (fchown(fd, a->st.st_uid, a->st.st_gid) < 0)
		return -1;
	if (a->con[0] && fsetfilecon(fd, a->con) < 0)
		return -1;
	return 0;
}

void clone_attr(const char *source, const char *target) {
	struct file_attr a;
	getattr(source, &a);
	setattr(target, &a);
}

void fclone_attr(int sourcefd, int targetfd) {
	struct file_attr a;
	fgetattr(sourcefd, &a);
	fsetattr(targetfd, &a);
}

void *__mmap(const char *filename, size_t *size, bool rw) {
	struct stat st;
	void *buf;
	int fd = xopen(filename, (rw ? O_RDWR : O_RDONLY) | O_CLOEXEC);
	fstat(fd, &st);
	if (S_ISBLK(st.st_mode))
		ioctl(fd, BLKGETSIZE64, size);
	else
		*size = st.st_size;
	buf = *size > 0 ? xmmap(nullptr, *size, PROT_READ | (rw ? PROT_WRITE : 0), MAP_SHARED, fd, 0) : nullptr;
	close(fd);
	return buf;
}

void mmap_ro(const char *filename, void **buf, size_t *size) {
	*buf = __mmap(filename, size, false);
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
		*buf = nullptr;
		*size = 0;
		return;
	}
	fd_full_read(fd, buf, size);
	close(fd);
}

void write_zero(int fd, size_t size) {
	char buf[4096] = {0};
	size_t len;
	while (size > 0) {
		len = sizeof(buf) > size ? size : sizeof(buf);
		write(fd, buf, len);
		size -= len;
	}
}

void file_readline(const char *file, const function<bool (string_view)> &fn, bool trim) {
	FILE *fp = xfopen(file, "re");
	if (fp == nullptr)
		return;
	size_t len = 1024;
	char *buf = (char *) malloc(len);
	char *start;
	ssize_t read;
	while ((read = getline(&buf, &len, fp)) >= 0) {
		start = buf;
		if (trim) {
			while (read && (buf[read - 1] == '\n' || buf[read - 1] == ' '))
				--read;
			buf[read] = '\0';
			while (*start == ' ')
				++start;
		}
		if (!fn(start))
			break;
	}
	fclose(fp);
	free(buf);
}

void parse_prop_file(const char *file, const function<bool (string_view, string_view)> &fn) {
	file_readline(file, [&](string_view line_view) -> bool {
		char *line = (char *) line_view.data();
		if (line[0] == '#')
			return true;
		char *eql = strchr(line, '=');
		if (eql == nullptr || eql == line)
			return true;
		*eql = '\0';
		return fn(line, eql + 1);
	}, true);
}
