module;
#include <ctype.h>
#include <dirent.h>
#include <fcntl.h>
#include <limits.h>
#include <poll.h>
#include <pthread.h>
#include <pwd.h>
#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

export module base:utils;

// Keep the original C declarations and their linkage when exporting names.
export {
using ::DIR, ::dirent, ::opendir, ::readdir, ::rewinddir, ::closedir, ::dirfd;
using ::FILE, ::fopen, ::fclose, ::fileno, ::funopen, ::setbuf;
using ::printf, ::fprintf, ::vfprintf, ::sscanf, ::fscanf;
using ::stdin, ::stdout, ::stderr, ::va_list;
using ::malloc, ::calloc, ::realloc, ::free, ::abort, ::exit;
using ::atoi, ::strtol, ::getenv, ::setenv, ::putenv, ::clearenv, ::realpath, ::system;
using ::strlen, ::strcmp, ::strcasecmp, ::strstr, ::strdup, ::strerror, ::memcmp, ::memmem;
using ::size_t, ::ssize_t, ::off_t, ::pid_t, ::uid_t, ::gid_t, ::mode_t, ::dev_t, ::ino_t;
using ::stat, ::fstat, ::fstatat, ::mkdir, ::mkfifo;
using ::access, ::faccessat, ::close, ::unlink, ::unlinkat, ::rmdir, ::remove;
using ::link, ::symlink, ::chdir, ::lseek, ::ftruncate, ::truncate, ::sync;
using ::fork, ::wait, ::waitpid, ::execve, ::execvp, ::_exit;
using ::getpid, ::getppid, ::gettid, ::getuid, ::getpagesize, ::sleep, ::usleep;
using ::syscall;
using ::setresuid, ::setresgid, ::setgroups, ::setsid, ::isatty, ::grantpt, ::unlockpt;
using ::clone, ::setns, ::unshare;
using ::passwd, ::getpwnam, ::getpwuid;
using ::mmap, ::munmap;
using ::sigset_t, ::sigaction, ::signal, ::kill;
using ::sigaddset, ::sigemptyset, ::sigfillset, ::sigprocmask;
using ::pthread_t, ::pthread_attr_t, ::pthread_mutex_t;
using ::pthread_create, ::pthread_exit, ::pthread_sigmask;
using ::pthread_attr_init, ::pthread_attr_destroy, ::pthread_attr_setdetachstate;
using ::pthread_mutex_lock, ::pthread_mutex_unlock;
using ::time_t, ::timespec, ::time, ::clock_gettime, ::clock_nanosleep;
}

// Bionic's static inline overloads cannot be exported with using declarations.
// Carry their caller-side size and diagnostic attributes across the module
// boundary, and let Bionic select the runtime checks for the target API level.
export namespace sys {

[[gnu::always_inline, clang::no_stack_protector]]
inline void *memcpy(void *const dst __pass_object_size0, const void *src, size_t size)
        __diagnose_as_builtin(__builtin_memcpy, 1, 2, 3) {
    return ::memcpy(dst, src, size);
}

[[gnu::always_inline, clang::no_stack_protector]]
inline void *memset(void *const dst __pass_object_size0, int value, size_t size)
        __diagnose_as_builtin(__builtin_memset, 1, 2, 3)
        __clang_warning_if(!size, "'memset' will set 0 bytes; maybe the arguments got flipped?") {
    return ::memset(dst, value, size);
}

[[gnu::always_inline, clang::no_stack_protector]]
inline char *strcpy(char *const dst __pass_object_size, const char *src)
        __diagnose_as_builtin(__builtin_strcpy, 1, 2) {
    return ::strcpy(dst, src);
}

[[gnu::always_inline, clang::no_stack_protector]]
inline size_t strlcpy(char *const dst __pass_object_size, const char *src, size_t size)
        __clang_error_if(__bos_unevaluated_lt(__bos(dst), size),
                         "'strlcpy' called with size bigger than buffer") {
    return ::strlcpy(dst, src, size);
}

[[gnu::always_inline, clang::no_stack_protector]]
inline ssize_t write(int fd, const void *const buf __pass_object_size0, size_t count)
        __clang_error_if(count > SSIZE_MAX, "in call to 'write', 'count' must be <= SSIZE_MAX")
        __clang_error_if(__bos_unevaluated_lt(__bosn(buf, 0), count),
                         "in call to 'write', 'count' bytes overflows the given object") {
    return ::write(fd, buf, count);
}

[[gnu::always_inline, clang::no_stack_protector]]
inline mode_t umask(mode_t mode)
        __clang_error_if(mode & ~0777, "'umask' called with invalid mode") {
    return ::umask(mode);
}

[[gnu::always_inline, clang::no_stack_protector]]
inline int open(const char *const path __pass_object_size, int flags)
        __clang_error_if((flags & O_CREAT) || (flags & O_TMPFILE) == O_TMPFILE,
                         "'open' called with O_CREAT or O_TMPFILE, but missing mode") {
    return ::open(path, flags);
}

[[gnu::always_inline, clang::no_stack_protector]]
inline int open(const char *const path __pass_object_size, int flags, mode_t mode)
        __clang_warning_if(!((flags & O_CREAT) || (flags & O_TMPFILE) == O_TMPFILE) && mode,
                           "'open' has superfluous mode bits; missing O_CREAT or O_TMPFILE?") {
    return ::open(path, flags, mode);
}

[[gnu::always_inline, clang::no_stack_protector]]
inline int openat(int dirfd, const char *const path __pass_object_size, int flags)
        __clang_error_if((flags & O_CREAT) || (flags & O_TMPFILE) == O_TMPFILE,
                         "'openat' called with O_CREAT or O_TMPFILE, but missing mode") {
    return ::openat(dirfd, path, flags);
}

[[gnu::always_inline, clang::no_stack_protector]]
inline int openat(int dirfd, const char *const path __pass_object_size, int flags, mode_t mode)
        __clang_warning_if(!((flags & O_CREAT) || (flags & O_TMPFILE) == O_TMPFILE) && mode,
                           "'openat' has superfluous mode bits; missing O_CREAT or O_TMPFILE?") {
    return ::openat(dirfd, path, flags, mode);
}

[[gnu::always_inline, clang::no_stack_protector]]
inline int poll(pollfd *const fds __pass_object_size, nfds_t count, int timeout)
        __clang_error_if(__bos_unevaluated_lt(__bos(fds), sizeof(*fds) * count),
                         "in call to 'poll', fd_count is larger than the given buffer") {
    return ::poll(fds, count, timeout);
}

[[gnu::always_inline, clang::no_stack_protector]]
inline char *fgets(char *const dst __pass_object_size, int size, FILE *stream)
        __clang_error_if(size < 0, "in call to 'fgets', size should not be negative")
        __clang_error_if(__bos_unevaluated_lt(__bos(dst), size),
                         "in call to 'fgets', size is larger than the destination buffer") {
    return ::fgets(dst, size, stream);
}

[[gnu::always_inline, clang::no_stack_protector]]
inline size_t fwrite(const void *const buf __pass_object_size0,
                     size_t size, size_t count, FILE *stream)
        __clang_error_if(size && (size_t)-1 / size < count,
                         "in call to 'fwrite', size * count overflows")
        __clang_error_if(__bos_unevaluated_lt(__bosn(buf, 0), size * count),
                         "in call to 'fwrite', size * count is too large for the given buffer") {
    return ::fwrite(buf, size, count, stream);
}

[[gnu::always_inline, clang::no_stack_protector]]
inline __printflike(3, 0) int vsnprintf(char *const dst __pass_object_size,
                                      size_t size, const char *format, va_list args)
        __diagnose_as_builtin(__builtin_vsnprintf, 1, 2, 3, 4) {
    return ::vsnprintf(dst, size, format, args);
}

// Like Bionic's sprintf, a va_list-producing function cannot be always_inline.
int sprintf(char *dst, const char *format)
        __enable_if(__bos_unevaluated_lt(__bos(dst), __builtin_strlen(format)),
                    "format string will always overflow destination buffer")
        __clang_error_if(1, "format string will always overflow destination buffer");

inline __printflike(2, 3) int sprintf(char *const dst __pass_object_size,
                                    const char *format, ...) {
    va_list args;
    va_start(args, format);
    int ret = ::vsprintf(dst, format, args);
    va_end(args);
    return ret;
}

[[gnu::always_inline]] inline int isalnum(int c) { return ::isalnum(c); }
[[gnu::always_inline]] inline int isalpha(int c) { return ::isalpha(c); }
[[gnu::always_inline]] inline int isdigit(int c) { return ::isdigit(c); }
[[gnu::always_inline]] inline int isspace(int c) { return ::isspace(c); }
[[gnu::always_inline]] inline int isupper(int c) { return ::isupper(c); }

}
