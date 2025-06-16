#include <cstdio>

#include <android/log.h>

#include <flags.h>
#include <base.hpp>

using namespace std;

#ifndef __call_bypassing_fortify
#define __call_bypassing_fortify(fn) (&fn)
#endif

#undef vsnprintf
static int fmt_and_log_with_rs(LogLevel level, const char *fmt, va_list ap) {
    constexpr int sz = 4096;
    char buf[sz];
    buf[0] = '\0';
    // Fortify logs when a fatal error occurs. Do not run through fortify again
    int len = std::min(__call_bypassing_fortify(vsnprintf)(buf, sz, fmt, ap), sz - 1);
    log_with_rs(level, rust::Utf8CStr(buf, len + 1));
    return len;
}

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
        level = LogLevel::ErrorCxx;
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
void LOGE(const char *fmt, ...) { LOG_BODY(ErrorCxx) }

// Export raw symbol to fortify compat
extern "C" void __vloge(const char* fmt, va_list ap) {
    fmt_and_log_with_rs(LogLevel::ErrorCxx, fmt, ap);
}
