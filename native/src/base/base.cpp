#include <sys/mman.h>

#include <base.hpp>
#include <flags.h>

using namespace std;

rust::Vec<size_t> byte_data::patch(byte_view from, byte_view to) const {
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

rust::Vec<size_t> mut_u8_patch(MutByteSlice buf, ByteSlice from, ByteSlice to) {
    byte_data data(buf);
    return data.patch(from, to);
}

template<typename T, int base>
static T parse_num(string_view s) {
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
int parse_int(string_view s) {
    return parse_num<int, 10>(s);
}

uint32_t parse_uint32_hex(string_view s) {
    return parse_num<uint32_t, 16>(s);
}

string &replace_all(string &str, string_view from, string_view to) {
    size_t pos = 0;
    while((pos = str.find(from, pos)) != string::npos) {
        str.replace(pos, from.length(), to);
        pos += to.length();
    }
    return str;
}

template <typename T>
static auto split_impl(string_view s, string_view delims) {
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

vector<string> split(string_view s, string_view delims) {
    return split_impl<string>(s, delims);
}

#undef vsnprintf
int vssprintf(char *dest, size_t size, const char *fmt, va_list ap) {
    if (size > 0) {
        *dest = 0;
        return std::min(vsnprintf(dest, size, fmt, ap), (int) size - 1);
    }
    return -1;
}

int ssprintf(char *dest, size_t size, const char *fmt, ...) {
    va_list va;
    va_start(va, fmt);
    int r = vssprintf(dest, size, fmt, va);
    va_end(va);
    return r;
}

#undef strlcpy
size_t strscpy(char *dest, const char *src, size_t size) {
    return std::min(strlcpy(dest, src, size), size - 1);
}

#undef vsnprintf
static int fmt_and_log_with_rs(LogLevel level, const char *fmt, va_list ap) {
    constexpr int sz = 4096;
    char buf[sz];
    buf[0] = '\0';
    // Fortify logs when a fatal error occurs. Do not run through fortify again
    int len = std::min((&vsnprintf)(buf, sz, fmt, ap), sz - 1);
    log_with_rs(level, Utf8CStr(buf, len + 1));
    return len;
}

#if __has_include(<android/log.h>)
#include <android/log.h>
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
#endif // __has_include(<android/log.h>)

#define LOG_BODY(level)   \
    va_list argv;         \
    va_start(argv, fmt);  \
    fmt_and_log_with_rs(LogLevel::level, fmt, argv); \
    va_end(argv);         \

// LTO will optimize out the NOP function
#if MAGISK_DEBUG
void LOGD(const char *fmt, ...) { LOG_BODY(Debug) }
#else
void LOGD(const char *fmt, ...) {}
#endif
void LOGI(const char *fmt, ...) { LOG_BODY(Info) }
void LOGW(const char *fmt, ...) { LOG_BODY(Warn) }
void LOGE(const char *fmt, ...) { LOG_BODY(Error) }

string full_read(int fd) {
    string str;
    char buf[4096];
    for (ssize_t len; (len = xread(fd, buf, sizeof(buf))) > 0;)
        str.insert(str.end(), buf, buf + len);
    return str;
}

string full_read(const char *filename) {
    string str;
    if (int fd = xopen(filename, O_RDONLY | O_CLOEXEC); fd >= 0) {
        str = full_read(fd);
        close(fd);
    }
    return str;
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

sDIR make_dir(DIR *dp) {
    return sDIR(dp, [](DIR *dp){ return dp ? closedir(dp) : 1; });
}

sFILE make_file(FILE *fp) {
    return sFILE(fp, [](FILE *fp){ return fp ? fclose(fp) : 1; });
}

mmap_data::mmap_data(const char *name, bool rw) {
    auto slice = rust::map_file(name, rw);
    if (!slice.empty()) {
        this->ptr = slice.data();
        this->sz = slice.size();
    }
}

mmap_data::mmap_data(int dirfd, const char *name, bool rw) {
    auto slice = rust::map_file_at(dirfd, name, rw);
    if (!slice.empty()) {
        this->ptr = slice.data();
        this->sz = slice.size();
    }
}

mmap_data::mmap_data(int fd, size_t sz, bool rw) {
    auto slice = rust::map_fd(fd, sz, rw);
    if (!slice.empty()) {
        this->ptr = slice.data();
        this->sz = slice.size();
    }
}

mmap_data::~mmap_data() {
    if (ptr) munmap(ptr, sz);
}

void mmap_data::swap(mmap_data &o) {
    std::swap(ptr, o.ptr);
    std::swap(sz, o.sz);
}

string resolve_preinit_dir(const char *base_dir) {
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

// FFI for Utf8CStr

extern "C" void cxx$utf8str$new(Utf8CStr *self, const void *s, size_t len);
extern "C" const char *cxx$utf8str$ptr(const Utf8CStr *self);
extern "C" size_t cxx$utf8str$len(const Utf8CStr *self);

Utf8CStr::Utf8CStr(const char *s, size_t len) : repr{} {
    cxx$utf8str$new(this, s, len);
}

const char *Utf8CStr::data() const {
    return cxx$utf8str$ptr(this);
}

size_t Utf8CStr::length() const {
    return cxx$utf8str$len(this);
}
