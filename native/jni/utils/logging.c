#include <stdio.h>
#include <stdlib.h>
#include <android/log.h>

#include "logging.h"
#include "flags.h"

int nop_log(const char *fmt, va_list ap) {
	return 0;
}

void nop_ex(int i) {}

struct log_callback log_cb = {
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

#define LOG_TAG "Magisk"

static int log_d(const char *fmt, va_list ap) {
	return __android_log_vprint(ANDROID_LOG_DEBUG, LOG_TAG, fmt, ap);
}

static int log_i(const char *fmt, va_list ap) {
	return __android_log_vprint(ANDROID_LOG_INFO, LOG_TAG, fmt, ap);
}

static int log_w(const char *fmt, va_list ap) {
	return __android_log_vprint(ANDROID_LOG_WARN, LOG_TAG, fmt, ap);
}

static int log_e(const char *fmt, va_list ap) {
	return __android_log_vprint(ANDROID_LOG_ERROR, LOG_TAG, fmt, ap);
}

void android_logging() {
#ifdef MAGISK_DEBUG
	log_cb.d = log_d;
#else
	log_cb.d = nop_log;
#endif
	log_cb.i = log_i;
	log_cb.w = log_w;
	log_cb.e = log_e;
	log_cb.ex = nop_ex;
}

static int vprinte(const char *fmt, va_list ap) {
	return vfprintf(stderr, fmt, ap);
}

void cmdline_logging() {
	log_cb.d = vprinte;
	log_cb.i = vprintf;
	log_cb.w = vprinte;
	log_cb.e = vprinte;
	log_cb.ex = exit;
}

int log_handler(log_type t, const char *fmt, ...) {
	va_list argv;
	int ret = 0;
	va_start(argv, fmt);
	switch (t) {
		case L_DEBUG:
			ret = log_cb.d(fmt, argv);
			break;
		case L_INFO:
			ret = log_cb.i(fmt, argv);
			break;
		case L_WARN:
			ret = log_cb.w(fmt, argv);
			break;
		case L_ERR:
			ret = log_cb.e(fmt, argv);
			log_cb.ex(1);
			break;
	}
	va_end(argv);
	return ret;
}
