/* logging.h - Error handling and logging
 */

#ifndef _LOGGING_H_
#define _LOGGING_H_

#include <string.h>
#include <errno.h>
#include <stdlib.h>

#ifdef IS_DAEMON

#include <pthread.h>
#include <android/log.h>

#define LOG_TAG    "Magisk"

// Global handler for PLOGE
extern __thread void (*err_handler)(void);

// Common error handlers
static inline void exit_proc() { exit(1); }
static inline void exit_thread() { pthread_exit(NULL); }
static inline void do_nothing() {}

#ifdef MAGISK_DEBUG
#define LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#else
#define LOGD(...)  {}
#endif
#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGW(...)  __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

#define PLOGE(fmt, args...) { LOGE(fmt " failed with %d: %s", ##args, errno, strerror(errno)); err_handler(); }

#else // IS_DAEMON

#include <stdio.h>

#define LOGE(...) { fprintf(stderr, __VA_ARGS__); exit(1); }
#define PLOGE(fmt, args...) { fprintf(stderr, fmt " failed with %d: %s\n\n", ##args, errno, strerror(errno)); exit(1); }

#endif // IS_DAEMON

#endif // _LOGGING_H_
