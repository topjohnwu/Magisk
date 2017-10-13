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

#ifdef MAGISK_DEBUG
#define LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#else
#define LOGD(...)  {}
#endif
#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGW(...)  __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

#define PLOGE(fmt, args...) LOGE(fmt " failed with %d: %s", ##args, errno, strerror(errno))

enum {
	HIDE_EVENT,
	LOG_EVENT,
	DEBUG_EVENT
};
extern int logcat_events[];

void monitor_logs();
void start_debug_full_log();
void stop_debug_full_log();
void start_debug_log();

#else // IS_DAEMON

#include <stdio.h>

#define LOGE(...) { fprintf(stderr, __VA_ARGS__); exit(1); }
#define PLOGE(fmt, args...) { fprintf(stderr, fmt " failed with %d: %s\n\n", ##args, errno, strerror(errno)); exit(1); }

#endif // IS_DAEMON

#endif // _LOGGING_H_
