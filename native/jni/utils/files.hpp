#pragma once

#include <sys/mman.h>
#include <sys/stat.h>
#include <mntent.h>
#include <functional>
#include <string_view>
#include <string>
#include <vector>

#include "xwrap.hpp"

#define do_align(p, a)  (((p) + (a) - 1) / (a) * (a))
#define align_off(p, a) (do_align(p, a) - (p))

struct file_attr {
    struct stat st;
    char con[128];
};

struct raw_file {
    std::string path;
    file_attr attr;
    uint8_t *buf;
    size_t sz;

    raw_file() : attr({}), buf(nullptr), sz(0) {}
    raw_file(const raw_file&) = delete;
    raw_file(raw_file &&o);
    ~raw_file();
};

ssize_t fd_path(int fd, char *path, size_t size);
int fd_pathat(int dirfd, const char *name, char *path, size_t size);
int mkdirs(std::string path, mode_t mode);
void rm_rf(const char *path);
void mv_path(const char *src, const char *dest);
void mv_dir(int src, int dest);
void cp_afc(const char *src, const char *dest);
void link_path(const char *src, const char *dest);
void link_dir(int src, int dest);
int getattr(const char *path, file_attr *a);
int getattrat(int dirfd, const char *name, file_attr *a);
int fgetattr(int fd, file_attr *a);
int setattr(const char *path, file_attr *a);
int setattrat(int dirfd, const char *name, file_attr *a);
int fsetattr(int fd, file_attr *a);
void fclone_attr(int src, int dest);
void clone_attr(const char *src, const char *dest);
void fd_full_read(int fd, void **buf, size_t *size);
void full_read(const char *filename, void **buf, size_t *size);
std::string fd_full_read(int fd);
std::string full_read(const char *filename);
void write_zero(int fd, size_t size);
void file_readline(bool trim, const char *file, const std::function<bool(std::string_view)> &fn);
static inline void file_readline(const char *file,
        const std::function<bool(std::string_view)> &fn) {
    file_readline(false, file, fn);
}
void parse_prop_file(const char *file,
        const std::function<bool(std::string_view, std::string_view)> &fn);
void *__mmap(const char *filename, size_t *size, bool rw);
void frm_rf(int dirfd);
void clone_dir(int src, int dest);
void parse_mnt(const char *file, const std::function<bool(mntent*)> &fn);
void backup_folder(const char *dir, std::vector<raw_file> &files);
void restore_folder(const char *dir, std::vector<raw_file> &files);

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

using sFILE = std::unique_ptr<FILE, decltype(&fclose)>;
using sDIR = std::unique_ptr<DIR, decltype(&closedir)>;
sDIR make_dir(DIR *dp);
sFILE make_file(FILE *fp);

static inline sDIR open_dir(const char *path) {
    return make_dir(opendir(path));
}

static inline sDIR xopen_dir(const char *path) {
    return make_dir(xopendir(path));
}

static inline sDIR xopen_dir(int dirfd) {
    return make_dir(xfdopendir(dirfd));
}

static inline sFILE open_file(const char *path, const char *mode) {
    return make_file(fopen(path, mode));
}

static inline sFILE xopen_file(const char *path, const char *mode) {
    return make_file(xfopen(path, mode));
}

static inline sFILE xopen_file(int fd, const char *mode) {
    return make_file(xfdopen(fd, mode));
}
