#pragma once

#define do_align(p, a)  (((p) + (a) - 1) / (a) * (a))
#define align_off(p, a) (do_align(p, a) - (p))

struct file_attr {
	struct stat st;
	char con[128];
};

#ifdef __cplusplus
extern "C" {
#endif

ssize_t fd_path(int fd, char *path, size_t size);
int fd_pathat(int dirfd, const char *name, char *path, size_t size);
int mkdirs(const char *pathname, mode_t mode);
void rm_rf(const char *path);
void mv_f(const char *source, const char *destination);
void mv_dir(int src, int dest);
void cp_afc(const char *source, const char *destination);
void link_dir(int src, int dest);
int getattr(const char *path, struct file_attr *a);
int getattrat(int dirfd, const char *name, struct file_attr *a);
int fgetattr(int fd, struct file_attr *a);
int setattr(const char *path, struct file_attr *a);
int setattrat(int dirfd, const char *name, struct file_attr *a);
int fsetattr(int fd, struct file_attr *a);
void fclone_attr(int sourcefd, int targetfd);
void clone_attr(const char *source, const char *target);
void fd_full_read(int fd, void **buf, size_t *size);
void full_read(const char *filename, void **buf, size_t *size);
void write_zero(int fd, size_t size);

#ifdef __cplusplus
}

#include <functional>
#include <string_view>

void file_readline(const char *file, const std::function<bool (std::string_view)> &fn, bool trim = false);
void parse_prop_file(const char *file, const std::function
		<bool(std::string_view, std::string_view)> &fn);
void *__mmap(const char *filename, size_t *size, bool rw);
void frm_rf(int dirfd, std::initializer_list<const char *> excl = std::initializer_list<const char *>());
void clone_dir(int src, int dest, bool overwrite = true);
void parse_mnt(const char *file, const std::function<bool (mntent*)> &fn);

template <typename T>
void full_read(const char *filename, T &buf, size_t &size) {
	static_assert(std::is_pointer<T>::value);
	full_read(filename, reinterpret_cast<void**>(&buf), &size);
}

template <typename T>
void fd_full_read(int fd, T &buf, size_t &size) {
	static_assert(std::is_pointer<T>::value);
	fd_full_read(fd, reinterpret_cast<void**>(&buf), &size);
}

template <typename B>
void mmap_ro(const char *filename, B &buf, size_t &sz) {
	buf = (B) __mmap(filename, &sz, false);
}

template <typename B, typename L>
void mmap_ro(const char *filename, B &buf, L &sz) {
	size_t __sz;
	buf = (B) __mmap(filename, &__sz, false);
	sz = __sz;
}

template <typename B>
void mmap_rw(const char *filename, B &buf, size_t &sz) {
	buf = (B) __mmap(filename, &sz, true);
}

template <typename B, typename L>
void mmap_rw(const char *filename, B &buf, L &sz) {
	size_t __sz;
	buf = (B) __mmap(filename, &__sz, true);
	sz = __sz;
}

#endif