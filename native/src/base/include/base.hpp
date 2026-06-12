#pragma once

#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <functional>

#include <rust/cxx.h>

void LOGD(const char *fmt, ...) __printflike(1, 2);
void LOGI(const char *fmt, ...) __printflike(1, 2);
void LOGW(const char *fmt, ...) __printflike(1, 2);
void LOGE(const char *fmt, ...) __printflike(1, 2);
#define PLOGE(fmt, args...) LOGE(fmt " failed with %d: %s\n", ##args, errno, std::strerror(errno))

extern "C" {

// xwraps

FILE *xfopen(const char *pathname, const char *mode);
FILE *xfdopen(int fd, const char *mode);
int xopen(const char *pathname, int flags, mode_t mode = 0);
int xopenat(int dirfd, const char *pathname, int flags, mode_t mode = 0);
ssize_t xwrite(int fd, const void *buf, size_t count);
ssize_t xread(int fd, void *buf, size_t count);
ssize_t xxread(int fd, void *buf, size_t count);
int xsetns(int fd, int nstype);
int xunshare(int flags);
DIR *xopendir(const char *name);
DIR *xfdopendir(int fd);
dirent *xreaddir(DIR *dirp);
pid_t xsetsid();
int xfstat(int fd, struct stat *buf);
int xdup2(int oldfd, int newfd);
ssize_t xreadlinkat(
        int dirfd, const char * __restrict__ pathname, char * __restrict__ buf, size_t bufsiz);
int xsymlink(const char *target, const char *linkpath);
int xmount(const char *source, const char *target,
           const char *filesystemtype, unsigned long mountflags,
           const void *data);
int xumount2(const char *target, int flags);
int xrename(const char *oldpath, const char *newpath);
int xmkdir(const char *pathname, mode_t mode);
int xmkdirs(const char *pathname, mode_t mode);
ssize_t xsendfile(int out_fd, int in_fd, off_t *offset, size_t count);
pid_t xfork();
ssize_t xrealpath(const char * __restrict__ path, char * __restrict__ buf, size_t bufsiz);
int xmknod(const char * pathname, mode_t mode, dev_t dev);

// Utils

int mkdirs(const char *path, mode_t mode);
ssize_t canonical_path(const char * __restrict__ path, char * __restrict__ buf, size_t bufsiz);
bool rm_rf(const char *path);
bool cp_afc(const char *src, const char *dest);
bool mv_path(const char *src, const char *dest);
bool link_path(const char *src, const char *dest);
bool clone_attr(const char *src, const char *dest);
bool fclone_attr(int src, int dest);

} // extern "C"

#define DISALLOW_COPY_AND_MOVE(clazz) \
clazz(const clazz&) = delete;        \
clazz(clazz &&) = delete;

#define ALLOW_MOVE_ONLY(clazz) \
clazz(const clazz&) = delete;  \
clazz(clazz &&o) : clazz() { swap(o); }  \
clazz& operator=(clazz &&o) { swap(o); return *this; }

struct Utf8CStr;

class mutex_guard {
    DISALLOW_COPY_AND_MOVE(mutex_guard)
public:
    explicit mutex_guard(pthread_mutex_t &m): mutex(&m) {
        pthread_mutex_lock(mutex);
    }
    void unlock() {
        pthread_mutex_unlock(mutex);
        mutex = nullptr;
    }
    ~mutex_guard() {
        if (mutex) pthread_mutex_unlock(mutex);
    }
private:
    pthread_mutex_t *mutex;
};

template <class Func>
class run_finally {
    DISALLOW_COPY_AND_MOVE(run_finally)
public:
    explicit run_finally(Func &&fn) : fn(std::move(fn)) {}
    ~run_finally() { fn(); }
private:
    Func fn;
};

template<class T>
static void default_new(T *&p) { p = new T(); }

template<class T>
static void default_new(std::unique_ptr<T> &p) { p.reset(new T()); }

struct StringCmp {
    using is_transparent = void;
    bool operator()(std::string_view a, std::string_view b) const { return a < b; }
};

using ByteSlice = rust::Slice<const uint8_t>;
using MutByteSlice = rust::Slice<uint8_t>;

// Interchangeable as `&[u8]` in Rust
struct byte_view {
    byte_view() : ptr(nullptr), sz(0) {}
    byte_view(const void *buf, size_t sz) : ptr((uint8_t *) buf), sz(sz) {}

    // byte_view, or any of its subclasses, can be copied as byte_view
    byte_view(const byte_view &o) : ptr(o.ptr), sz(o.sz) {}

    // Transparent conversion to Rust slice
    byte_view(const ByteSlice o) : byte_view(o.data(), o.size()) {}
    operator ByteSlice() const { return {ptr, sz}; }

    // String as bytes, including null terminator
    byte_view(const char *s) : byte_view(s, strlen(s) + 1) {}

    const uint8_t *data() const { return ptr; }
    size_t size() const { return sz; }

protected:
    uint8_t *ptr;
    size_t sz;
};

// Interchangeable as `&mut [u8]` in Rust
struct byte_data : public byte_view {
    byte_data() = default;
    byte_data(void *buf, size_t sz) : byte_view(buf, sz) {}

    // byte_data, or any of its subclasses, can be copied as byte_data
    byte_data(const byte_data &o) : byte_data(o.ptr, o.sz) {}

    // Transparent conversion to Rust slice
    byte_data(const MutByteSlice o) : byte_data(o.data(), o.size()) {}
    operator MutByteSlice() const { return {ptr, sz}; }

    using byte_view::data;
    uint8_t *data() const { return ptr; }

    rust::Vec<size_t> patch(byte_view from, byte_view to) const;
};

struct mmap_data : public byte_data {
    ALLOW_MOVE_ONLY(mmap_data)

    mmap_data() = default;
    explicit mmap_data(const char *name, bool rw = false);
    mmap_data(int dirfd, const char *name, bool rw = false);
    mmap_data(int fd, size_t sz, bool rw = false);
    ~mmap_data();
private:
    void swap(mmap_data &o);
};


struct owned_fd {
    ALLOW_MOVE_ONLY(owned_fd)

    owned_fd() : fd(-1) {}
    owned_fd(int fd) : fd(fd) {}
    ~owned_fd() { close(fd); fd = -1; }

    operator int() { return fd; }
    int release() { int f = fd; fd = -1; return f; }
    void swap(owned_fd &owned) { std::swap(fd, owned.fd); }

private:
    int fd;
};

rust::Vec<size_t> mut_u8_patch(MutByteSlice buf, ByteSlice from, ByteSlice to);

uint32_t parse_uint32_hex(std::string_view s);
int parse_int(std::string_view s);

using thread_entry = void *(*)(void *);
extern "C" int new_daemon_thread(thread_entry entry, void *arg = nullptr);

static inline std::string rtrim(std::string &&s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch) && ch != '\0';
    }).base(), s.end());
    return std::move(s);
}

int fork_dont_care();
int fork_no_orphan();
void init_argv0(int argc, char **argv);
void set_nice_name(Utf8CStr name);
int switch_mnt_ns(int pid);
std::string &replace_all(std::string &str, std::string_view from, std::string_view to);
std::vector<std::string> split(std::string_view s, std::string_view delims);

// Similar to vsnprintf, but the return value is the written number of bytes
__printflike(3, 0) int vssprintf(char *dest, size_t size, const char *fmt, va_list ap);
// Similar to snprintf, but the return value is the written number of bytes
__printflike(3, 4) int ssprintf(char *dest, size_t size, const char *fmt, ...);
// This is not actually the strscpy from the Linux kernel.
// Silently truncates, and returns the number of bytes written.
extern "C" size_t strscpy(char *dest, const char *src, size_t size);

// Ban usage of unsafe cstring functions
#define vsnprintf  __use_vssprintf_instead__
#define snprintf   __use_ssprintf_instead__
#define strlcpy    __use_strscpy_instead__

struct exec_t {
    bool err = false;
    int fd = -2;
    void (*pre_exec)() = nullptr;
    int (*fork)() = xfork;
    const char **argv = nullptr;
};

int exec_command(exec_t &exec);
template <class ...Args>
int exec_command(exec_t &exec, Args &&...args) {
    const char *argv[] = {args..., nullptr};
    exec.argv = argv;
    return exec_command(exec);
}
int exec_command_sync(exec_t &exec);
template <class ...Args>
int exec_command_sync(exec_t &exec, Args &&...args) {
    const char *argv[] = {args..., nullptr};
    exec.argv = argv;
    return exec_command_sync(exec);
}
template <class ...Args>
int exec_command_sync(Args &&...args) {
    exec_t exec;
    return exec_command_sync(exec, args...);
}
template <class ...Args>
void exec_command_async(Args &&...args) {
    const char *argv[] = {args..., nullptr};
    exec_t exec {
        .fork = fork_dont_care,
        .argv = argv,
    };
    exec_command(exec);
}

template <typename T>
constexpr auto operator+(T e) noexcept ->
    std::enable_if_t<std::is_enum<T>::value, std::underlying_type_t<T>> {
    return static_cast<std::underlying_type_t<T>>(e);
}

std::string full_read(int fd);
std::string full_read(const char *filename);
void write_zero(int fd, size_t size);
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

// Bindings to &Utf8CStr in Rust
struct Utf8CStr {
    const char *data() const;
    size_t length() const;
    Utf8CStr(const char *s, size_t len);

    Utf8CStr() : Utf8CStr("", 1) {};
    Utf8CStr(const Utf8CStr &o) = default;
    Utf8CStr(const char *s) : Utf8CStr(s, strlen(s) + 1) {};
    Utf8CStr(std::string s) : Utf8CStr(s.data(), s.length() + 1) {};
    const char *c_str() const { return this->data(); }
    size_t size() const { return this->length(); }
    bool empty() const { return this->length() == 0 ; }
    std::string_view sv() const { return {data(), length()}; }
    operator std::string_view() const { return sv(); }
    bool operator==(std::string_view rhs) const { return sv() == rhs; }

private:
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-private-field"
    std::array<std::uintptr_t, 2> repr;
#pragma clang diagnostic pop
};

// Bindings for std::function to be callable from Rust
using CxxFnBoolStrStr = std::function<bool(rust::Str, rust::Str)>;
struct FnBoolStrStr : public CxxFnBoolStrStr {
    using CxxFnBoolStrStr::function;
    bool call(rust::Str a, rust::Str b) const {
        return operator()(a, b);
    }
};
using CxxFnBoolStr = std::function<bool(Utf8CStr)>;
struct FnBoolStr : public CxxFnBoolStr {
    using CxxFnBoolStr::function;
    bool call(Utf8CStr s) const {
        return operator()(s);
    }
};

#include "../base-rs.hpp"

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
