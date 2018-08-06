/* magisk.h - Top header
 */

#ifndef _MAGISK_H_
#define _MAGISK_H_

#include "logging.h"

#define MAGISK_VER_STR  xstr(MAGISK_VERSION) ":MAGISK"
#define MAIN_SOCKET  "d30138f2310a9fb9c54a3e0c21f58591"
#define LOG_SOCKET   "5864cd77f2f8c59b3882e2d35dbf51e4"
#define JAVA_PACKAGE_NAME "com.topjohnwu.magisk"

#ifndef ARG_MAX
#define ARG_MAX 4096
#endif

#define LOGFILE         "/cache/magisk.log"
#define UNBLOCKFILE     "/dev/.magisk.unblock"
#define TMPSEPOLICY     "/dev/.tmp_sepolicy"
#define DISABLEFILE     "/cache/.disable_magisk"
#define MAGISKTMP       "/sbin/.core"
#define BLOCKDIR        MAGISKTMP "/block"
#define MIRRDIR         MAGISKTMP "/mirror"
#define BBPATH          MAGISKTMP "/busybox"
#define MOUNTPOINT      MAGISKTMP "/img"
#define COREDIR         MOUNTPOINT "/.core"
#define HOSTSFILE       COREDIR "/hosts"
#define HIDELIST        COREDIR "/hidelist"
#define SECURE_DIR      "/data/adb"
#define MAINIMG         SECURE_DIR "/magisk.img"
#define DATABIN         SECURE_DIR "/magisk"
#define MAGISKDB        SECURE_DIR "/magisk.db"
#define SIMPLEMOUNT     SECURE_DIR "/magisk_simple"
#define BOOTCOUNT       SECURE_DIR "/.boot_count"
#define MANAGERAPK      DATABIN "/magisk.apk"
#define MAGISKRC        "/init.magisk.rc"


// selinuxfs paths
#define SELINUX_PATH        "/sys/fs/selinux"
#define SELINUX_ENFORCE     SELINUX_PATH "/enforce"
#define SELINUX_POLICY      SELINUX_PATH "/policy"
#define SELINUX_LOAD        SELINUX_PATH "/load"
#define SELINUX_CONTEXT     SELINUX_PATH "/context"

#define MAGISKHIDE_PROP     "persist.magisk.hide"

extern char *argv0;     /* For changing process name */

#define applet          ((char *[]) { "su", "resetprop", "magiskhide", "imgtool", NULL })
#define init_applet     ((char *[]) { "magiskpolicy", "supolicy", NULL })

extern int (*applet_main[]) (int, char *[]), (*init_applet_main[]) (int, char *[]);

int create_links(const char *bin, const char *path);

// Multi-call entrypoints
int magiskhide_main(int argc, char *argv[]);
int magiskpolicy_main(int argc, char *argv[]);
int su_client_main(int argc, char *argv[]);
int resetprop_main(int argc, char *argv[]);
int imgtool_main(int argc, char *argv[]);

#endif
