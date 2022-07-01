#include <cstdio>
#include <cstdlib>

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

static void fmt_and_log_with_rs(LogLevel level, const char *fmt, va_list ap) {
    char buf[4096];
    int len = vsnprintf(buf, sizeof(buf), fmt, ap);
    if (len > 0 && buf[len - 1] == '\n') {
        // It's unfortunate that all logging on the C++ side always manually include
        // a newline at the end due to how it was originally implemented.
        // The logging infrastructure on the rust side does NOT expect a newline
        // at the end, so we will have to strip it out before sending it over.
        buf[len - 1] = '\0';
    }
    log_with_rs(level, buf);
}

#define rlog(prio) [](auto fmt, auto ap) { fmt_and_log_with_rs(LogLevel::prio, fmt, ap); }
static void forward_logging_to_rs() {
    log_cb.d = rlog(Debug);
    log_cb.i = rlog(Info);
    log_cb.w = rlog(Warn);
    log_cb.e = rlog(Error);
}

void cmdline_logging() {
    rs::logging::cmdline_logging();
    forward_logging_to_rs();
}

void exit_on_error(bool b) {
    rs::logging::exit_on_error(b);
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
