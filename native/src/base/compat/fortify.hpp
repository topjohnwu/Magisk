// Original source: https://android.googlesource.com/platform/bionic/+/master/libc/bionic/fortify.cpp
// License: AOSP, full copyright notice please check original source

#include <sys/stat.h>
#include <fcntl.h>

#undef _FORTIFY_SOURCE

extern void __vloge(const char* fmt, va_list ap);

static inline __noreturn __printflike(1, 2) void __fortify_fatal(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    __vloge(fmt, args);
    va_end(args);
    abort();
}
static inline void __check_count(const char* fn, const char* identifier, size_t value) {
    if (__predict_false(value > SSIZE_MAX)) {
        __fortify_fatal("%s: %s %zu > SSIZE_MAX", fn, identifier, value);
    }
}
static inline void __check_buffer_access(const char* fn, const char* action,
                                         size_t claim, size_t actual) {
    if (__predict_false(claim > actual)) {
        __fortify_fatal("%s: prevented %zu-byte %s %zu-byte buffer", fn, claim, action, actual);
    }
}

[[gnu::weak]]
void* __memcpy_chk(void* dst, const void* src, size_t count, size_t dst_len) {
    __check_count("memcpy", "count", count);
    __check_buffer_access("memcpy", "write into", count, dst_len);
    return __call_bypassing_fortify(memcpy)(dst, src, count);
}

[[gnu::weak]]
char* __strcpy_chk(char* dst, const char* src, size_t dst_len) {
    // TODO: optimize so we don't scan src twice.
    size_t src_len = __builtin_strlen(src) + 1;
    __check_buffer_access("strcpy", "write into", src_len, dst_len);
    return __builtin_strcpy(dst, src);
}

[[gnu::weak]]
size_t __strlcpy_chk(char* dst, const char* src,
                     size_t supplied_size, size_t dst_len_from_compiler) {
    __check_buffer_access("strlcpy", "write into", supplied_size, dst_len_from_compiler);
    return __call_bypassing_fortify(strlcpy)(dst, src, supplied_size);
}

[[gnu::weak]]
char* __strchr_chk(const char* p, int ch, size_t s_len) {
    for (;; ++p, s_len--) {
        if (__predict_false(s_len == 0)) {
            __fortify_fatal("strchr: prevented read past end of buffer");
        }
        if (*p == static_cast<char>(ch)) {
            return const_cast<char*>(p);
        }
        if (*p == '\0') {
            return nullptr;
        }
    }
}

[[gnu::weak]]
char* __strcat_chk(char* dst, const char* src, size_t dst_buf_size) {
    char* save = dst;
    size_t dst_len = __strlen_chk(dst, dst_buf_size);
    dst += dst_len;
    dst_buf_size -= dst_len;
    while ((*dst++ = *src++) != '\0') {
        dst_buf_size--;
        if (__predict_false(dst_buf_size == 0)) {
            __fortify_fatal("strcat: prevented write past end of %zu-byte buffer", dst_buf_size);
        }
    }
    return save;
}

[[gnu::weak]]
size_t __strlen_chk(const char* s, size_t s_len) {
    // TODO: "prevented" here would be a lie because this strlen can run off the end.
    // strlen is too important to be expensive, so we wanted to be able to call the optimized
    // implementation, but I think we need to implement optimized assembler __strlen_chk routines.
    size_t ret = __builtin_strlen(s);
    if (__predict_false(ret >= s_len)) {
        __fortify_fatal("strlen: detected read past end of buffer");
    }
    return ret;
}

[[gnu::weak]]
int __vsprintf_chk(char* dst, int /*flags*/,
                   size_t dst_len_from_compiler, const char* format, va_list va) {
    // The compiler uses SIZE_MAX to mean "no idea", but our vsnprintf rejects sizes that large.
    int result = __call_bypassing_fortify(vsnprintf)(dst,
                           dst_len_from_compiler == SIZE_MAX ? SSIZE_MAX : dst_len_from_compiler,
                           format, va);
    // Try to catch failures after the fact...
    __check_buffer_access("vsprintf", "write into", result + 1, dst_len_from_compiler);
    return result;
}

[[gnu::weak]]
mode_t __umask_chk(mode_t mode) {
    if (__predict_false((mode & 0777) != mode)) {
        __fortify_fatal("umask: called with invalid mask %o", mode);
    }
    return __umask_real(mode);
}

[[gnu::weak]]
ssize_t __read_chk(int fd, void* buf, size_t count, size_t buf_size) {
    __check_count("read", "count", count);
    __check_buffer_access("read", "write into", count, buf_size);
    return __call_bypassing_fortify(read)(fd, buf, count);
}
static inline bool needs_mode(int flags) {
    return ((flags & O_CREAT) == O_CREAT) || ((flags & O_TMPFILE) == O_TMPFILE);
}
static inline int force_O_LARGEFILE(int flags) {
    return flags | O_LARGEFILE;
}

[[gnu::weak]]
int __open_2(const char* pathname, int flags) {
    if (needs_mode(flags)) __fortify_fatal("open: called with O_CREAT/O_TMPFILE but no mode");
    return __openat_real(AT_FDCWD, pathname, force_O_LARGEFILE(flags), 0);
}

[[gnu::weak]]
int __openat_2(int fd, const char* pathname, int flags) {
    if (needs_mode(flags)) __fortify_fatal("open: called with O_CREAT/O_TMPFILE but no mode");
    return __openat_real(fd, pathname, force_O_LARGEFILE(flags), 0);
}

[[gnu::weak]]
int __vsnprintf_chk(char* dst, size_t supplied_size, int /*flags*/,
                               size_t dst_len_from_compiler, const char* format, va_list va) {
    __check_buffer_access("vsnprintf", "write into", supplied_size, dst_len_from_compiler);
    return __call_bypassing_fortify(vsnprintf)(dst, supplied_size, format, va);
}
