/* logging.h - Error handling and logging
 */

#ifndef _LOGGING_H_
#define _LOGGING_H_

#include <string.h>
#include <errno.h>
#include <stdlib.h>

#define str(a) #a
#define xstr(a) str(a)

/**************
 * No logging *
 **************/

#define LOGD(...)
#define LOGI(...)
#define LOGW(...)
#define LOGE(...)
#define PLOGE(...)

/******************
 * Daemon logging *
 ******************/

#ifdef IS_DAEMON

#undef LOGI
#undef LOGW
#undef LOGE
#undef PLOGE

#include <pthread.h>
#include <android/log.h>

#define LOG_TAG    "Magisk"

#ifdef MAGISK_DEBUG
#undef LOGD
#define LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#endif
#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGW(...)  __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define PLOGE(fmt, args...) LOGE(fmt " failed with %d: %s", ##args, errno, strerror(errno))

#endif

/********************
 * Tools Log & Exit *
 ********************/

#ifdef XWRAP_EXIT

#undef LOGE
#undef PLOGE

#include <stdio.h>

#define LOGE(...) { fprintf(stderr, __VA_ARGS__); exit(1); }
#define PLOGE(fmt, args...) { fprintf(stderr, fmt " failed with %d: %s\n\n", ##args, errno, strerror(errno)); exit(1); }

#endif


#endif // _LOGGING_H_
