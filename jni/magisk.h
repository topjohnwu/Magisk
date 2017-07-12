/* magisk.h - Top header
 */

#ifndef _MAGISK_H_
#define _MAGISK_H_

#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <android/log.h>

#define MAGISK_VER_STR  xstr(MAGISK_VERSION) ":MAGISK"

#define str(a) #a
#define xstr(a) str(a)

#define REQUESTOR_DAEMON_PATH     "\0MAGISK"
#define REQUESTOR_DAEMON_PATH_LEN 7

#define LOG_TAG    "Magisk"

#ifndef ARG_MAX
#define ARG_MAX 4096
#endif

#define LOGFILE         "/cache/magisk.log"
#define LASTLOG         "/cache/last_magisk.log"
#define DEBUG_LOG       "/data/magisk_debug.log"
#define UNBLOCKFILE     "/dev/.magisk.unblock"
#define DISABLEFILE     "/cache/.disable_magisk"
#define UNINSTALLER     "/cache/magisk_uninstaller.sh"
#define MOUNTPOINT      "/magisk"
#define COREDIR         MOUNTPOINT "/.core"
#define HOSTSFILE       COREDIR "/hosts"
#define HIDELIST        COREDIR "/hidelist"
#define MAINIMG         "/data/magisk.img"
#define DATABIN         "/data/magisk"
#define MANAGERAPK      DATABIN "/magisk.apk"
#define MAGISKTMP       "/dev/magisk"
#define MIRRDIR         MAGISKTMP "/mirror"
#define DUMMDIR         MAGISKTMP "/dummy"
#define CACHEMOUNT      "/cache/magisk_mount"

#define SELINUX_PATH        "/sys/fs/selinux/"
#define SELINUX_ENFORCE     SELINUX_PATH "enforce"
#define SELINUX_POLICY      SELINUX_PATH "policy"
#define SELINUX_LOAD        SELINUX_PATH "load"

#define MAGISKHIDE_PROP     "persist.magisk.hide"

// Global handler for PLOGE
extern __thread void (*err_handler)(void);

// Common error handlers
static inline void exit_proc() { exit(1); }
static inline void exit_thread() { pthread_exit(NULL); }
static inline void do_nothing() {}

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

#ifdef STATIC
#define LOGD(...) stub(__VA_ARGS__)
#define LOGI(...) stub(__VA_ARGS__)
#define LOGW(...) stub(__VA_ARGS__)
#define LOGE(...) stub(__VA_ARGS__)
#define PLOGE(...) stub(__VA_ARGS__)
#endif

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
