#include "magisk.h"
#include "magiskpolicy.h"

static const char magiskrc[] =

"on early-init\n"
"    write " EARLYINIT " 1\n"
"    wait " EARLYINITDONE "\n"
"    rm " EARLYINITDONE "\n"
"\n"

"on post-fs-data\n"
"    start logd\n"
"    load_persist_props\n"
"    rm " UNBLOCKFILE "\n"
"    start %s\n"
"    wait " UNBLOCKFILE " 10\n"
"    rm " UNBLOCKFILE "\n"
"\n"

"service %s /sbin/magisk --post-fs-data\n"
"    user root\n"
"    seclabel u:r:" SEPOL_PROC_DOMAIN ":s0\n"
"    oneshot\n"
"\n"

"service %s /sbin/magisk --service\n"
"    class late_start\n"
"    user root\n"
"    seclabel u:r:" SEPOL_PROC_DOMAIN ":s0\n"
"    oneshot\n"
"\n"

#if 0
"on property:sys.boot_completed=1\n"
"    start magisk_bc\n"
"\n"

"service magisk_bc /sbin/magisk --boot-complete\n"
"    user root\n"
"    seclabel u:r:"SEPOL_PROC_DOMAIN":s0\n"
"    oneshot\n"
;
#endif
;
