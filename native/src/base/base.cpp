module;
#include <sys/wait.h>
#include <sys/prctl.h>
#include <sys/mman.h>
#include <android/log.h>
#include <linux/fs.h>
#include <syscall.h>
#include <flags.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <pthread.h>
#include <rust/cxx.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

export module magisk.base;
export import std;

export template<__SIZE_TYPE__ N>
struct StringLiteral {
    char value[N]{};
    constexpr StringLiteral() = default;
    consteval StringLiteral(const char (&text)[N]) {
        for (__SIZE_TYPE__ i = 0; i < N; ++i) value[i] = text[i];
    }
};

template<StringLiteral... Parts>
consteval auto concat_literals() {
    StringLiteral<((sizeof(Parts.value) - 1) + ...) + 1> result;
    __SIZE_TYPE__ offset = 0;
    auto append = [&](auto part) {
        for (__SIZE_TYPE__ i = 0; i + 1 < sizeof(part.value); ++i)
            result.value[offset++] = part.value[i];
    };
    (append(Parts), ...);
    return result;
}

export template<StringLiteral... Parts>
inline constexpr auto concat = concat_literals<Parts...>();

export inline constexpr char JAVA_PACKAGE_NAME[] = "com.topjohnwu.magisk";
export inline constexpr char SECURE_DIR[] = "/data/adb";
export inline constexpr auto &MODULEROOT = concat<SECURE_DIR, "/modules">.value;
export inline constexpr auto &DATABIN = concat<SECURE_DIR, "/magisk">.value;
export inline constexpr auto &MAGISKDB = concat<SECURE_DIR, "/magisk.db">.value;
// tmpfs paths
export inline constexpr char INTLROOT[] = ".magisk";
export inline constexpr auto &MIRRDIR = concat<INTLROOT, "/mirror">.value;
export inline constexpr auto &PREINITMIRR = concat<INTLROOT, "/preinit">.value;
export inline constexpr auto &DEVICEDIR = concat<INTLROOT, "/device">.value;
export inline constexpr auto &PREINITDEV = concat<DEVICEDIR, "/preinit">.value;
export inline constexpr auto &WORKERDIR = concat<INTLROOT, "/worker">.value;
export inline constexpr auto &BBPATH = concat<INTLROOT, "/busybox">.value;
export inline constexpr auto &ROOTOVL = concat<INTLROOT, "/rootdir">.value;
export inline constexpr auto &SHELLPTS = concat<INTLROOT, "/pts">.value;
export inline constexpr auto &MAIN_CONFIG = concat<INTLROOT, "/config">.value;
export inline constexpr auto &MAIN_SOCKET = concat<DEVICEDIR, "/socket">.value;
export inline constexpr auto POST_FS_DATA_WAIT_TIME = 40;
export inline constexpr auto POST_FS_DATA_SCRIPT_MAX_TIME = 35;
// Unconstrained domain the daemon and root processes run in
export inline constexpr char SEPOL_PROC_DOMAIN[] = "magisk";
export inline constexpr auto &MAGISK_PROC_CON = concat<"u:r:", SEPOL_PROC_DOMAIN, ":s0">.value;
// Unconstrained file type that anyone can access
export inline constexpr char SEPOL_FILE_TYPE[] = "magisk_file";
export inline constexpr auto &MAGISK_FILE_CON = concat<"u:object_r:", SEPOL_FILE_TYPE, ":s0">.value;
export inline constexpr char PLAT_POLICY_DIR[] = "/system/etc/selinux/";
export inline constexpr char VEND_POLICY_DIR[] = "/vendor/etc/selinux/";
export inline constexpr char PROD_POLICY_DIR[] = "/product/etc/selinux/";
export inline constexpr char ODM_POLICY_DIR[] = "/odm/etc/selinux/";
export inline constexpr char SYSEXT_POLICY_DIR[] = "/system_ext/etc/selinux/";
export inline constexpr auto &SPLIT_PLAT_CIL = concat<PLAT_POLICY_DIR, "plat_sepolicy.cil">.value;
export inline constexpr char SELINUX_MNT[] = "/sys/fs/selinux";
export inline constexpr auto &SELINUX_VERSION = concat<SELINUX_MNT, "/policyvers">.value;
export inline constexpr char DEFAULT_DT_DIR[] = "/proc/device-tree/firmware/android";
export inline constexpr char REDIR_PATH[] = "/data/magiskinit";
export inline constexpr char PRELOAD_LIB[] = "/dev/preload.so";
export inline constexpr char PRELOAD_POLICY[] = "/dev/sepolicy";
export inline constexpr char PRELOAD_ACK[] = "/dev/ack";

export inline constexpr const char *applet_names[] = { "su", "resetprop", nullptr };

#define PLOGE(fmt, args...) LOGE(fmt " failed with %d: %s\n", ##args, errno, ::strerror(errno))

export extern "C++" {
#include "base-rs.hpp"

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

// Bindings to &Utf8CStr in Rust
extern "C" void cxx$utf8str$new(Utf8CStr *self, const void *s, size_t len);
extern "C" const char *cxx$utf8str$ptr(const Utf8CStr *self);
extern "C" size_t cxx$utf8str$len(const Utf8CStr *self);

struct Utf8CStr {
    const char *data() const {
        return cxx$utf8str$ptr(this);
    }
    size_t length() const {
        return cxx$utf8str$len(this);
    }
    Utf8CStr(const char *s, size_t len) : repr{} {
        cxx$utf8str$new(this, s, len);
    }

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
inline void default_new(T *&p) { p = new T(); }

template<class T>
inline void default_new(std::unique_ptr<T> &p) { p.reset(new T()); }

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

    rust::Vec<size_t> patch(byte_view from, byte_view to) const {
        rust::Vec<size_t> v;
        if (ptr == nullptr)
            return v;
        auto p = ptr;
        auto eof = ptr + sz;
        while (p < eof) {
            p = static_cast<uint8_t *>(memmem(p, eof - p, from.data(), from.size()));
            if (p == nullptr)
                return v;
            memset(p, 0, from.size());
            memcpy(p, to.data(), to.size());
            v.push_back(p - ptr);
            p += from.size();
        }
        return v;
    }
};

struct mmap_data : public byte_data {
    ALLOW_MOVE_ONLY(mmap_data)

    mmap_data() = default;
    explicit mmap_data(const char *name, bool rw = false) {
        auto slice = rust::map_file(name, rw);
        if (!slice.empty()) {
            this->ptr = slice.data();
            this->sz = slice.size();
        }
    }
    mmap_data(int dirfd, const char *name, bool rw = false) {
        auto slice = rust::map_file_at(dirfd, name, rw);
        if (!slice.empty()) {
            this->ptr = slice.data();
            this->sz = slice.size();
        }
    }
    mmap_data(int fd, size_t sz, bool rw = false) {
        auto slice = rust::map_fd(fd, sz, rw);
        if (!slice.empty()) {
            this->ptr = slice.data();
            this->sz = slice.size();
        }
    }
    ~mmap_data() {
        if (ptr) munmap(ptr, sz);
    }
private:
    void swap(mmap_data &o) {
        std::swap(ptr, o.ptr);
        std::swap(sz, o.sz);
    }
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

using thread_entry = void *(*)(void *);

inline std::string rtrim(std::string &&s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch) && ch != '\0';
    }).base(), s.end());
    return std::move(s);
}

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

template <typename T>
constexpr auto operator+(T e) noexcept ->
    std::enable_if_t<std::is_enum<T>::value, std::underlying_type_t<T>> {
    return static_cast<std::underlying_type_t<T>>(e);
}

using sFILE = std::unique_ptr<FILE, decltype(&fclose)>;
using sDIR = std::unique_ptr<DIR, decltype(&closedir)>;

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
}

using namespace std;

#ifndef __call_bypassing_fortify
#define __call_bypassing_fortify(fn) (&fn)
#endif

#ifdef __LP64__
static_assert(BLKGETSIZE64 == 0x80081272);
#else
static_assert(BLKGETSIZE64 == 0x80041272);
#endif

#undef vsnprintf
#undef strlcpy
// Similar to vsnprintf, returning the number of bytes written.
export extern "C++" __printflike(3, 0) int vssprintf(char *dest, size_t size, const char *fmt, va_list ap) {
    if (size > 0) {
        *dest = 0;
        return std::min(vsnprintf(dest, size, fmt, ap), (int) size - 1);
    }
    return -1;
}

export extern "C++" __printflike(3, 4) int ssprintf(char *dest, size_t size, const char *fmt, ...) {
    va_list va;
    va_start(va, fmt);
    int r = vssprintf(dest, size, fmt, va);
    va_end(va);
    return r;
}

// Silently truncate to the buffer size and return the number of bytes written.
export extern "C" size_t strscpy(char *dest, const char *src, size_t size) {
    return std::min(strlcpy(dest, src, size), size - 1);
}

int fmt_and_log_with_rs(LogLevel level, const char *fmt, va_list ap) {
    constexpr int sz = 4096;
    char buf[sz];
    buf[0] = '\0';
    // Fortify logs when a fatal error occurs. Do not run through fortify again
    int len = std::min(__call_bypassing_fortify(vsnprintf)(buf, sz, fmt, ap), sz - 1);
    log_with_rs(level, Utf8CStr(buf, len + 1));
    return len;
}

#define LOG_BODY(level)   \
    va_list argv;         \
    va_start(argv, fmt);  \
    fmt_and_log_with_rs(LogLevel::level, fmt, argv); \
    va_end(argv);         \

// LTO will optimize out the NOP function
#if MAGISK_DEBUG
export extern "C++" __printflike(1, 2) void LOGD(const char *fmt, ...) { LOG_BODY(Debug) }
#else
export extern "C++" __printflike(1, 2) void LOGD(const char *fmt, ...) {}
#endif
export extern "C++" __printflike(1, 2) void LOGI(const char *fmt, ...) { LOG_BODY(Info) }
export extern "C++" __printflike(1, 2) void LOGW(const char *fmt, ...) { LOG_BODY(Warn) }
export extern "C++" __printflike(1, 2) void LOGE(const char *fmt, ...) { LOG_BODY(Error) }

export extern "C++" rust::Vec<size_t> mut_u8_patch(MutByteSlice buf, ByteSlice from, ByteSlice to) {
    byte_data data(buf);
    return data.patch(from, to);
}

export extern "C++" int fork_dont_care() {
    if (int pid = xfork()) {
        waitpid(pid, nullptr, 0);
        return pid;
    } else if (xfork()) {
        exit(0);
    }
    return 0;
}

export extern "C++" int fork_no_orphan() {
    int pid = xfork();
    if (pid)
        return pid;
    prctl(PR_SET_PDEATHSIG, SIGKILL);
    if (getppid() == 1)
        exit(1);
    return 0;
}

export extern "C++" int exec_command(exec_t &exec) {
    auto pipefd = array<int, 2>{-1, -1};
    int outfd = -1;

    if (exec.fd == -1) {
        if (xpipe2(pipefd, O_CLOEXEC) == -1)
            return -1;
        outfd = pipefd[1];
    } else if (exec.fd >= 0) {
        outfd = exec.fd;
    }

    int pid = exec.fork();
    if (pid < 0) {
        close(pipefd[0]);
        close(pipefd[1]);
        return -1;
    } else if (pid) {
        if (exec.fd == -1) {
            exec.fd = pipefd[0];
            close(pipefd[1]);
        }
        return pid;
    }

    // Unblock all signals
    sigset_t set;
    sigfillset(&set);
    pthread_sigmask(SIG_UNBLOCK, &set, nullptr);

    if (outfd >= 0) {
        xdup2(outfd, STDOUT_FILENO);
        if (exec.err)
            xdup2(outfd, STDERR_FILENO);
        close(outfd);
    }

    // Call the pre-exec callback
    if (exec.pre_exec)
        exec.pre_exec();

    execve(exec.argv[0], (char **) exec.argv, environ);
    PLOGE("execve %s", exec.argv[0]);
    exit(-1);
}

export extern "C++" int exec_command_sync(exec_t &exec) {
    int pid = exec_command(exec);
    if (pid < 0)
        return -1;
    int status;
    waitpid(pid, &status, 0);
    return WEXITSTATUS(status);
}

export extern "C" int new_daemon_thread(thread_entry entry, void *arg = nullptr) {
    pthread_t thread;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    errno = pthread_create(&thread, &attr, entry, arg);
    if (errno) {
        PLOGE("pthread_create");
    }
    return errno;
}

char *argv0;
size_t name_len;
export extern "C++" void init_argv0(int argc, char **argv) {
    argv0 = argv[0];
    name_len = (argv[argc - 1] - argv[0]) + strlen(argv[argc - 1]) + 1;
}

export extern "C++" void set_nice_name(Utf8CStr name) {
    memset(argv0, 0, name_len);
    strscpy(argv0, name.c_str(), name_len);
    prctl(PR_SET_NAME, name.c_str());
}

template<typename T, int base>
T parse_num(string_view s) {
    T val = 0;
    for (char c : s) {
        if (isdigit(c)) {
            c -= '0';
        } else if (base > 10 && isalpha(c)) {
            c -= isupper(c) ? 'A' - 10 : 'a' - 10;
        } else {
            return -1;
        }
        if (c >= base) {
            return -1;
        }
        val *= base;
        val += c;
    }
    return val;
}

/*
 * Bionic's atoi runs through strtol().
 * Use our own implementation for faster conversion.
 */
export extern "C++" int parse_int(string_view s) {
    return parse_num<int, 10>(s);
}

export extern "C++" uint32_t parse_uint32_hex(string_view s) {
    return parse_num<uint32_t, 16>(s);
}

export extern "C++" int switch_mnt_ns(int pid) {
    int ret = -1;
    int fd = syscall(__NR_pidfd_open, pid, 0);
    if (fd > 0) {
        ret = setns(fd, CLONE_NEWNS);
        close(fd);
    }
    if (ret < 0) {
        char mnt[32];
        ssprintf(mnt, sizeof(mnt), "/proc/%d/ns/mnt", pid);
        fd = open(mnt, O_RDONLY);
        if (fd < 0) return 1; // Maybe process died..

        // Switch to its namespace
        ret = xsetns(fd, 0);
        close(fd);
    }
    return ret;
}

export extern "C++" string &replace_all(string &str, string_view from, string_view to) {
    size_t pos = 0;
    while((pos = str.find(from, pos)) != string::npos) {
        str.replace(pos, from.length(), to);
        pos += to.length();
    }
    return str;
}

template <typename T>
auto split_impl(string_view s, string_view delims) {
    vector<T> result;
    size_t base = 0;
    size_t found;
    while (true) {
        found = s.find_first_of(delims, base);
        result.emplace_back(s.substr(base, found - base));
        if (found == string::npos)
            break;
        base = found + 1;
    }
    return result;
}

export extern "C++" vector<string> split(string_view s, string_view delims) {
    return split_impl<string>(s, delims);
}

#undef vsnprintf

#undef strlcpy

#undef vsnprintf

// Used to override external C library logging
extern "C" int magisk_log_print(int prio, const char *tag, const char *fmt, ...) {
    LogLevel level;
    switch (prio) {
    case ANDROID_LOG_DEBUG:
        level = LogLevel::Debug;
        break;
    case ANDROID_LOG_INFO:
        level = LogLevel::Info;
        break;
    case ANDROID_LOG_WARN:
        level = LogLevel::Warn;
        break;
    case ANDROID_LOG_ERROR:
        level = LogLevel::Error;
        break;
    default:
        return 0;
    }

    char fmt_buf[4096];
    auto len = strscpy(fmt_buf, tag, sizeof(fmt_buf) - 1);
    // Prevent format specifications in the tag
    std::replace(fmt_buf, fmt_buf + len, '%', '_');
    len = ssprintf(fmt_buf + len, sizeof(fmt_buf) - len - 1, ": %s", fmt) + len;
    // Ensure the fmt string always ends with newline
    if (fmt_buf[len - 1] != '\n') {
        fmt_buf[len] = '\n';
        fmt_buf[len + 1] = '\0';
    }
    va_list argv;
    va_start(argv, fmt);
    int ret = fmt_and_log_with_rs(level, fmt_buf, argv);
    va_end(argv);
    return ret;
}

// Export raw symbol to fortify compat
extern "C" void __vloge(const char* fmt, va_list ap) {
    fmt_and_log_with_rs(LogLevel::Error, fmt, ap);
}

export extern "C++" string full_read(int fd) {
    string str;
    char buf[4096];
    for (ssize_t len; (len = xread(fd, buf, sizeof(buf))) > 0;)
        str.insert(str.end(), buf, buf + len);
    return str;
}

export extern "C++" string full_read(const char *filename) {
    string str;
    if (int fd = xopen(filename, O_RDONLY | O_CLOEXEC); fd >= 0) {
        str = full_read(fd);
        close(fd);
    }
    return str;
}

export extern "C++" void write_zero(int fd, size_t size) {
    char buf[4096] = {0};
    size_t len;
    while (size > 0) {
        len = sizeof(buf) > size ? size : sizeof(buf);
        write(fd, buf, len);
        size -= len;
    }
}

export extern "C++" sDIR make_dir(DIR *dp) {
    return sDIR(dp, [](DIR *dp){ return dp ? closedir(dp) : 1; });
}

export extern "C++" sFILE make_file(FILE *fp) {
    return sFILE(fp, [](FILE *fp){ return fp ? fclose(fp) : 1; });
}

export extern "C++" string resolve_preinit_dir(const char *base_dir) {
    string dir = base_dir;
    if (access((dir + "/unencrypted").data(), F_OK) == 0) {
        dir += "/unencrypted/magisk";
    } else if (access((dir + "/adb").data(), F_OK) == 0) {
        dir += "/adb";
    } else if (access((dir + "/watchdog").data(), F_OK) == 0) {
        dir += "/watchdog/magisk";
    } else {
        dir += "/magisk";
    }
    return dir;
}

export extern "C++" template <class ...Args>
int exec_command(exec_t &exec, Args &&...args) {
    const char *argv[] = {args..., nullptr};
    exec.argv = argv;
    return exec_command(exec);
}

export extern "C++" template <class ...Args>
int exec_command_sync(exec_t &exec, Args &&...args) {
    const char *argv[] = {args..., nullptr};
    exec.argv = argv;
    return exec_command_sync(exec);
}

export extern "C++" template <class ...Args>
int exec_command_sync(Args &&...args) {
    exec_t exec;
    return exec_command_sync(exec, args...);
}

export extern "C++" template <class ...Args>
void exec_command_async(Args &&...args) {
    const char *argv[] = {args..., nullptr};
    exec_t exec {
        .fork = fork_dont_care,
        .argv = argv,
    };
    exec_command(exec);
}

export extern "C++" inline sDIR open_dir(const char *path) {
    return make_dir(opendir(path));
}

export extern "C++" inline sDIR xopen_dir(const char *path) {
    return make_dir(xopendir(path));
}

export extern "C++" inline sDIR xopen_dir(int dirfd) {
    return make_dir(xfdopendir(dirfd));
}

export extern "C++" inline sFILE open_file(const char *path, const char *mode) {
    return make_file(fopen(path, mode));
}

export extern "C++" inline sFILE xopen_file(const char *path, const char *mode) {
    return make_file(xfopen(path, mode));
}

export extern "C++" inline sFILE xopen_file(int fd, const char *mode) {
    return make_file(xfdopen(fd, mode));
}
