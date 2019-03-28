#pragma once

#include <logging.h>

#define MAIN_SOCKET  "d30138f2310a9fb9c54a3e0c21f58591"
#define LOG_SOCKET   "5864cd77f2f8c59b3882e2d35dbf51e4"
#define JAVA_PACKAGE_NAME "com.topjohnwu.magisk"
#define LOGFILE         "/cache/magisk.log"
#define UNBLOCKFILE     "/dev/.magisk_unblock"
#define EARLYINIT       "/dev/.magisk_early_init"
#define EARLYINITDONE   "/dev/.magisk_early_init_done"
#define DISABLEFILE     "/cache/.disable_magisk"
#define MAGISKTMP       "/sbin/.magisk"
#define MIRRDIR         MAGISKTMP "/mirror"
#define BBPATH          MAGISKTMP "/busybox"
#define MODULEMNT       MAGISKTMP "/modules"
#define SECURE_DIR      "/data/adb"
#define MODULEROOT      SECURE_DIR "/modules"
#define MODULEUPGRADE   SECURE_DIR "/modules_update"
#define DATABIN         SECURE_DIR "/magisk"
#define MAGISKDB        SECURE_DIR "/magisk.db"
#define SIMPLEMOUNT     SECURE_DIR "/magisk_simple"
#define BOOTCOUNT       SECURE_DIR "/.boot_count"
#define MANAGERAPK      DATABIN "/magisk.apk"

// Legacy crap
#define LEGACYCORE      MODULEROOT "/.core"

extern int SDK_INT;

constexpr const char *applet_names[] = { "magisk", "su", "resetprop", "magiskhide", nullptr };

// Multi-call entrypoints
int magisk_main(int argc, char *argv[]);
int magiskhide_main(int argc, char *argv[]);
int magiskpolicy_main(int argc, char *argv[]);
int su_client_main(int argc, char *argv[]);
int resetprop_main(int argc, char *argv[]);
