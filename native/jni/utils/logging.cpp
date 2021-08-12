#include <cstdio>
#include <cstdlib>

#include <flags.hpp>

#include "logging.hpp"

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

#define LOG_BODY(prio) { \
    va_list argv;        \
    va_start(argv, fmt); \
    log_cb.prio(fmt, argv); \
    va_end(argv);        \
}

// LTO will optimize out the NOP function
#ifdef MAGISK_DEBUG
void LOGD(const char *fmt, ...) { LOG_BODY(d) }
#else
void LOGD(const char *fmt, ...) {}
#endif
void LOGI(const char *fmt, ...) { LOG_BODY(i) }
void LOGW(const char *fmt, ...) { LOG_BODY(w) }
void LOGE(const char *fmt, ...) { LOG_BODY(e); log_cb.ex(1); }
