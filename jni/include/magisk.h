/* magisk.h - Top header
 */

#ifndef _MAGISK_H_
#define _MAGISK_H_

#include "logging.h"

#define MAGISK_VER_STR  xstr(MAGISK_VERSION) ":MAGISK"
#define REQUESTOR_DAEMON_PATH     "\0MAGISK"

#ifndef ARG_MAX
#define ARG_MAX 4096
#endif

#define LOGFILE         "/cache/magisk.log"
#define LASTLOG         "/cache/last_magisk.log"
#define DEBUG_LOG       "/data/magisk_debug.log"
#define UNBLOCKFILE     "/dev/.magisk.unblock"
#define PATCHDONE       "/dev/.magisk.patch.done"
#define DISABLEFILE     "/cache/.disable_magisk"
#define UNINSTALLER     "/cache/magisk_uninstaller.sh"
#define CACHEMOUNT      "/cache/magisk_mount"
#define MAGISKTMP       "/dev/magisk"
#define MIRRDIR         MAGISKTMP "/mirror"
#define BBPATH          MAGISKTMP "/bin"
#define MOUNTPOINT      MAGISKTMP "/img"
#define FAKEPOINT       "/magisk"
#define COREDIR         MOUNTPOINT "/.core"
#define HOSTSFILE       COREDIR "/hosts"
#define HIDELIST        COREDIR "/hidelist"
#define MAINIMG         "/data/magisk.img"
#define DATABIN         "/data/magisk"
#define MANAGERAPK      DATABIN "/magisk.apk"


#define SELINUX_PATH        "/sys/fs/selinux/"
#define SELINUX_ENFORCE     SELINUX_PATH "enforce"
#define SELINUX_POLICY      SELINUX_PATH "policy"
#define SELINUX_LOAD        SELINUX_PATH "load"

#define MAGISKHIDE_PROP     "persist.magisk.hide"

extern char *argv0;     /* For changing process name */

extern char *applet[];
extern int (*applet_main[]) (int, char *[]);

int create_links(const char *bin, const char *path);

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
