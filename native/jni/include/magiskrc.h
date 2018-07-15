#include "magisk.h"
#include "magiskpolicy.h"

const char magiskrc[] =

// Triggers

"on post-fs\n"
"   start logd\n"
"\n"

"on post-fs-data\n"
"   load_persist_props\n"
"   rm "UNBLOCKFILE"\n"
"   start magisk_startup\n"
"   wait "UNBLOCKFILE" 10\n"
"   rm "UNBLOCKFILE"\n"
"\n"

// Services

"service magisk_daemon /sbin/magisk --daemon\n"
"   user root\n"
"   seclabel u:r:"SEPOL_PROC_DOMAIN":s0\n"
"   oneshot\n"
"\n"

"service magisk_startup /sbin/magisk --startup\n"
"   user root\n"
"   seclabel u:r:"SEPOL_PROC_DOMAIN":s0\n"
"   oneshot\n"
"\n"

"service magisk_service /sbin/magisk --service\n"
"   class late_start\n"
"   user root\n"
"   seclabel u:r:"SEPOL_PROC_DOMAIN":s0\n"
"   oneshot\n"
;
