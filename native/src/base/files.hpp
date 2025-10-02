#pragma once

#include <sys/stat.h>
#include <linux/fs.h>
#include <functional>
#include <string_view>
#include <string>

#include "base-rs.hpp"

struct mmap_data : public byte_data {
    static_assert((sizeof(void *) == 8 && BLKGETSIZE64 == 0x80081272) ||
                  (sizeof(void *) == 4 && BLKGETSIZE64 == 0x80041272));
    ALLOW_MOVE_ONLY(mmap_data)

    mmap_data() = default;
    explicit mmap_data(const char *name, bool rw = false);
    mmap_data(int dirfd, const char *name, bool rw = false);
    mmap_data(int fd, size_t sz, bool rw = false);
    ~mmap_data();
};

extern "C" {

int mkdirs(const char *path, mode_t mode);
ssize_t canonical_path(const char * __restrict__ path, char * __restrict__ buf, size_t bufsiz);
bool rm_rf(const char *path);
bool frm_rf(int dirfd);
bool cp_afc(const char *src, const char *dest);
bool mv_path(const char *src, const char *dest);
bool link_path(const char *src, const char *dest);
bool clone_attr(const char *src, const char *dest);
bool fclone_attr(int src, int dest);

} // extern "C"

std::string full_read(int fd);
std::string full_read(const char *filename);
void write_zero(int fd, size_t size);
std::string resolve_preinit_dir(const char *base_dir);

// Functor = function<bool(Utf8CStr, Utf8CStr)>
template <typename Functor>
void parse_prop_file(const char *file, Functor &&fn) {
    parse_prop_file_rs(file, [&](rust::Str key, rust::Str val) -> bool {
        // We perform the null termination here in C++ because it's very difficult to do it
        // right in Rust due to pointer provenance. Trying to dereference a pointer without
        // the correct provenance in Rust, even in unsafe code, is undefined behavior.
        // However on the C++ side, there are fewer restrictions on pointers, so the const_cast here
        // will not trigger UB in the compiler.
        *(const_cast<char *>(key.data()) + key.size()) = '\0';
        *(const_cast<char *>(val.data()) + val.size()) = '\0';
        return fn(Utf8CStr(key.data(), key.size() + 1), Utf8CStr(val.data(), val.size() + 1));
    });
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
