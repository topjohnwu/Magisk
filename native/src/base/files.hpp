#pragma once

#include <sys/mman.h>
#include <sys/stat.h>
#include <mntent.h>
#include <functional>
#include <string_view>
#include <string>
#include <vector>

#include "xwrap.hpp"

template <typename T>
static inline T align_to(T v, int a) {
    static_assert(std::is_integral<T>::value);
    return (v + a - 1) / a * a;
}

template <typename T>
static inline T align_padding(T v, int a) {
    return align_to(v, a) - v;
}

struct file_attr {
    struct stat st;
    char con[128];
};

struct byte_data {
    using str_pairs = std::initializer_list<std::pair<std::string_view, std::string_view>>;

    uint8_t *buf = nullptr;
    size_t sz = 0;

    int patch(str_pairs list) { return patch(true, list); }
    int patch(bool log, str_pairs list);
    bool contains(std::string_view pattern, bool log = true) const;
    bool contains_raw(const char* pattern) const;
protected:
    void swap(byte_data &o);
};

struct mount_info {
    unsigned int id;
    unsigned int parent;
    dev_t device;
    std::string root;
    std::string target;
    std::string vfs_option;
    struct {
        unsigned int shared;
        unsigned int master;
        unsigned int propagate_from;
    } optional;
    std::string type;
    std::string source;
    std::string fs_option;
};

struct mmap_data : public byte_data {
    mmap_data() = default;
    mmap_data(const mmap_data&) = delete;
    mmap_data(mmap_data &&o) { swap(o); }
    mmap_data(const char *name, bool rw = false);
    ~mmap_data() { if (buf) munmap(buf, sz); }
    mmap_data& operator=(mmap_data &&other) { swap(other); return *this; }
};

extern "C" {

int mkdirs(const char *path, mode_t mode);
ssize_t canonical_path(const char * __restrict__ path, char * __restrict__ buf, size_t bufsiz);

} // extern "C"

using rust::fd_path;
int fd_pathat(int dirfd, const char *name, char *path, size_t size);
void rm_rf(const char *path);
void mv_path(const char *src, const char *dest);
void mv_dir(int src, int dest);
void cp_afc(const char *src, const char *dest);
void link_path(const char *src, const char *dest);
void link_dir(int src, int dest);
static inline ssize_t realpath(
        const char * __restrict__ path, char * __restrict__ buf, size_t bufsiz) {
    return canonical_path(path, buf, bufsiz);
}
int getattr(const char *path, file_attr *a);
int getattrat(int dirfd, const char *name, file_attr *a);
int fgetattr(int fd, file_attr *a);
int setattr(const char *path, file_attr *a);
int setattrat(int dirfd, const char *name, file_attr *a);
int fsetattr(int fd, file_attr *a);
void fclone_attr(int src, int dest);
void clone_attr(const char *src, const char *dest);
void full_read(int fd, std::string &str);
void full_read(const char *filename, std::string &str);
std::string full_read(int fd);
std::string full_read(const char *filename);
void write_zero(int fd, size_t size);
void file_readline(bool trim, FILE *fp, const std::function<bool(std::string_view)> &fn);
void file_readline(bool trim, const char *file, const std::function<bool(std::string_view)> &fn);
void file_readline(const char *file, const std::function<bool(std::string_view)> &fn);
void parse_prop_file(FILE *fp, const std::function<bool(std::string_view, std::string_view)> &fn);
void parse_prop_file(const char *file,
        const std::function<bool(std::string_view, std::string_view)> &fn);
void frm_rf(int dirfd);
void clone_dir(int src, int dest);
std::vector<mount_info> parse_mount_info(const char *pid);
std::string find_apk_path(const char *pkg);
std::string resolve_preinit_dir(const char *base_dir);

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
