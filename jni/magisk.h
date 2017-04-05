/* magisk.h - Top header
 *
 * Contain global functions like logging,
 * and entrypoint for main
 */

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
#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGW(...)  __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

#define PLOGE(fmt, args...) LOGE(fmt " failed with %d: %s", ##args, errno, strerror(errno)); exit(1)
#define PLOGEV(fmt, err, args...) LOGE(fmt " failed with %d: %s", ##args, err, strerror(err)); exit(1)

void stub(const char *fmt, ...);

// Global buffer (only for main thread!!)
#define BUF_SIZE 4096
extern char magiskbuf[BUF_SIZE];
extern char *argv0;     /* For changing process name */

// Multi-call entrypoints
int magiskhide_main(int argc, char *argv[]);
int magiskpolicy_main(int argc, char *argv[]);
int su_main(int argc, char *argv[]);

#ifdef __cplusplus
extern "C" {
#endif
int resetprop_main(int argc, char *argv[]);
#ifdef __cplusplus
}
#endif

/**************
 * MagiskHide *
 **************/

void launch_magiskhide();
void stop_magiskhide();


#endif
