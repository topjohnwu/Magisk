#include <cstdio>
#include <cstdlib>

#include "logging.hpp"
#include <flags.hpp>

using namespace std;

int nop_log(const char *, va_list) { return 0; }

void nop_ex(int) {}

log_callback log_cb = {
    .d = nop_log,
    .i = nop_log,
    .w = nop_log,
    .e = nop_log,
    .ex = nop_ex
};

void no_logging() {
    log_cb.d = nop_log;
    log_cb.i = nop_log;
    log_cb.w = nop_log;
    log_cb.e = nop_log;
    log_cb.ex = nop_ex;
}

static int vprintfe(const char *fmt, va_list ap) {
    return vfprintf(stderr, fmt, ap);
}

void cmdline_logging() {
    log_cb.d = vprintfe;
    log_cb.i = vprintf;
    log_cb.w = vprintfe;
    log_cb.e = vprintfe;
    log_cb.ex = exit;
}

template <int type>
void log_handler(const char *fmt, ...) {
    va_list argv;
    va_start(argv, fmt);
    if constexpr (type == L_DEBUG) {
        log_cb.d(fmt, argv);
    } else if constexpr (type == L_INFO) {
        log_cb.i(fmt, argv);
    } else if constexpr (type == L_WARN) {
        log_cb.w(fmt, argv);
    } else if constexpr (type == L_ERR) {
        log_cb.e(fmt, argv);
        log_cb.ex(1);
    }
    va_end(argv);
}

template void log_handler<L_INFO>(const char *fmt, ...);
template void log_handler<L_WARN>(const char *fmt, ...);
template void log_handler<L_ERR>(const char *fmt, ...);

#ifdef MAGISK_DEBUG
template void log_handler<L_DEBUG>(const char *fmt, ...);
#else
// Strip debug logging for release builds
template <> void log_handler<L_DEBUG>(const char *fmt, ...) {}
#endif
