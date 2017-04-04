#ifndef _MAGISK_H_
#define _MAGISK_H_

#include <errno.h>
#include <string.h>
#include <android/log.h>

#define LOG_TAG    "Magisk"

#ifdef DEBUG
#define LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#else
#define LOGD(...)  stub(__VA_ARGS__)
#endif
#define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define PLOGE(fmt, args...) LOGE(fmt " failed with %d: %s", ##args, errno, strerror(errno))

void stub(const char *fmt, ...);

// Global buffer
#define BUF_SIZE 4096
extern char magiskbuf[BUF_SIZE];

#endif
