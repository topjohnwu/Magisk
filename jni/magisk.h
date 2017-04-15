/* magisk.h - Top header
 */

#ifndef _MAGISK_H_
#define _MAGISK_H_

#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <android/log.h>


#define VERSION_CODE 130
#define VERSION      13.0
#define VERSION_STR  xstr(VERSION) ":MAGISK"

#define str(a) #a
#define xstr(a) str(a)

#define REQUESTOR_DAEMON_PATH     "\0MAGISK"
#define REQUESTOR_DAEMON_PATH_LEN 7

#define LOG_TAG    "Magisk"

// Global handler for PLOGE
extern __thread void (*err_handler)(void);

// Two common error handlers
static inline void exit_proc() { exit(1); }
static inline void exit_thread() { pthread_exit(NULL); }

// Dummy function to depress debug message
static inline void stub(const char *fmt, ...) {}

#ifdef DEBUG
#define LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#else
#define LOGD(...)  stub(__VA_ARGS__)
#endif
#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGW(...)  __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

#define PLOGE(fmt, args...) { LOGE(fmt " failed with %d: %s", ##args, errno, strerror(errno)); err_handler(); }

extern char *argv0;     /* For changing process name */

extern char *applet[];
extern int (*applet_main[]) (int, char *[]);

// Multi-call entrypoints
int magiskhide_main(int argc, char *argv[]);
int magiskpolicy_main(int argc, char *argv[]);
int su_client_main(int argc, char *argv[]);

#ifdef __cplusplus
extern "C" {
#endif
int resetprop_main(int argc, char *argv[]);
#ifdef __cplusplus
}
#endif

#endif
