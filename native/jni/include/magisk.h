/* magisk.h - Top header
 */

#ifndef _MAGISK_H_
#define _MAGISK_H_

#include "logging.h"

#define MAGISK_VER_STR  xstr(MAGISK_VERSION) ":MAGISK"
#define SOCKET_NAME     "d30138f2310a9fb9c54a3e0c21f58591"

#ifndef ARG_MAX
#define ARG_MAX 4096
#endif

#define LOGFILE         "/cache/magisk.log"
#define UNBLOCKFILE     "/dev/.magisk.unblock"
#define DISABLEFILE     "/cache/.disable_magisk"
#define UNINSTALLER     "/cache/magisk_uninstaller.sh"
#define MAGISKTMP       "/sbin/.core"
#define MIRRDIR         MAGISKTMP "/mirror"
#define BBPATH          MAGISKTMP "/busybox"
#define MOUNTPOINT      MAGISKTMP "/img"
#define COREDIR         MOUNTPOINT "/.core"
#define HOSTSFILE       COREDIR "/hosts"
#define HIDELIST        COREDIR "/hidelist"
#define SECURE_DIR      "/data/adb/"
#define MAINIMG         SECURE_DIR "magisk.img"
#define DATABIN         SECURE_DIR "magisk"
#define MAGISKDB        SECURE_DIR "magisk.db"
#define SIMPLEMOUNT     SECURE_DIR "magisk_simple"
#define DEBUG_LOG       SECURE_DIR "magisk_debug.log"
#define MANAGERAPK      DATABIN "/magisk.apk"
#define MAGISKRC        "/init.magisk.rc"


// selinuxfs paths
#define SELINUX_PATH        "/sys/fs/selinux/"
#define SELINUX_ENFORCE     SELINUX_PATH "enforce"
#define SELINUX_POLICY      SELINUX_PATH "policy"
#define SELINUX_LOAD        SELINUX_PATH "load"

// split policy paths
#define PLAT_POLICY_DIR     "/system/etc/selinux/"
#define NONPLAT_POLICY_DIR  "/vendor/etc/selinux/"
#define SPLIT_PLAT_CIL      PLAT_POLICY_DIR "plat_sepolicy.cil"
#define SPLIT_PLAT_MAPPING  PLAT_POLICY_DIR "mapping/%s.cil"
#define SPLIT_PRECOMPILE    NONPLAT_POLICY_DIR "precompiled_sepolicy"
#define SPLIT_NONPLAT_VER   NONPLAT_POLICY_DIR "plat_sepolicy_vers.txt"

#define MAGISKHIDE_PROP     "persist.magisk.hide"

extern char *argv0;     /* For changing process name */

#define applet          ((char *[]) { "su", "resetprop", "magiskhide", NULL })
#define init_applet     ((char *[]) { "magiskpolicy", "supolicy", NULL })

extern int (*applet_main[]) (int, char *[]), (*init_applet_main[]) (int, char *[]);

int create_links(const char *bin, const char *path);

// Multi-call entrypoints
int magiskhide_main(int argc, char *argv[]);
int magiskpolicy_main(int argc, char *argv[]);
int su_client_main(int argc, char *argv[]);
int resetprop_main(int argc, char *argv[]);

#endif
