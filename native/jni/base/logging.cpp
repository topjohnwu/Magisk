#include <cstdio>
#include <cstdlib>

#include <android/log.h>

#include <flags.h>
#include <base.hpp>

// Just need to include it somewhere
#include <base-rs.cpp>

using namespace std;

void nop_log(const char *, va_list) {}

log_callback log_cb = {
    .d = nop_log,
    .i = nop_log,
    .w = nop_log,
    .e = nop_log,
};
static bool EXIT_ON_ERROR = false;

static int fmt_and_log_with_rs(LogLevel level, const char *fmt, va_list ap) {
    char buf[4096];
    int ret = vsnprintf(buf, sizeof(buf), fmt, ap);
    log_with_rs(level, buf);
    return ret;
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
        level = LogLevel::Error;
        break;
    default:
        return 0;
    }

    char fmt_buf[4096];
    auto len = strlcpy(fmt_buf, tag, sizeof(fmt_buf));
    // Prevent format specifications in the tag
    std::replace(fmt_buf, fmt_buf + len, '%', '_');
    snprintf(fmt_buf + len, sizeof(fmt_buf) - len, ": %s", fmt);
    va_list argv;
    va_start(argv, fmt);
    int ret = fmt_and_log_with_rs(level, fmt_buf, argv);
    va_end(argv);
    return ret;
}

#define rlog(prio) [](auto fmt, auto ap) { fmt_and_log_with_rs(LogLevel::prio, fmt, ap); }
void forward_logging_to_rs() {
    log_cb.d = rlog(Debug);
    log_cb.i = rlog(Info);
    log_cb.w = rlog(Warn);
    log_cb.e = rlog(Error);
}

void cmdline_logging() {
    rust::cmdline_logging();
    forward_logging_to_rs();
    exit_on_error(true);
}

void exit_on_error(bool b) {
    rust::exit_on_error(b);
    EXIT_ON_ERROR = b;
}

#define LOG_BODY(prio) { \
    va_list argv;        \
    va_start(argv, fmt); \
    log_cb.prio(fmt, argv); \
    va_end(argv);        \
}

// LTO will optimize out the NOP function
#if MAGISK_DEBUG
void LOGD(const char *fmt, ...) { LOG_BODY(d) }
#else
void LOGD(const char *fmt, ...) {}
#endif
void LOGI(const char *fmt, ...) { LOG_BODY(i) }
void LOGW(const char *fmt, ...) { LOG_BODY(w) }
void LOGE(const char *fmt, ...) { LOG_BODY(e); if (EXIT_ON_ERROR) exit(EXIT_FAILURE); }

// Export raw symbol to fortify compat
extern "C" void __vloge(const char* fmt, va_list ap) {
    log_cb.e(fmt, ap);
}
