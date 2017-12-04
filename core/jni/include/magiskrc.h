const char magiskrc[] =

// Triggers

"on post-fs\n"
"   start logd\n"
"   start magisk_pfs\n"
"   wait /dev/.magisk.unblock 10\n"
"\n"

"on post-fs-data\n"
"   load_persist_props\n"
"   rm /dev/.magisk.unblock\n"
"   start magisk_pfsd\n"
"   wait /dev/.magisk.unblock 10\n"
"   rm /dev/.magisk.unblock\n"
"\n"

"on property:magisk.daemon=1\n"
"   start magisk_daemon\n"
"\n"

// Services

"service magisk_daemon /sbin/magisk --daemon\n"
"   user root\n"
"   seclabel u:r:su:s0\n"
"   oneshot\n"
"\n"

"service magisk_pfs /sbin/magisk --post-fs\n"
"   user root\n"
"   seclabel u:r:su:s0\n"
"   oneshot\n"
"\n"

"service magisk_pfsd /sbin/magisk --post-fs-data\n"
"   user root\n"
"   seclabel u:r:su:s0\n"
"   oneshot\n"
"\n"

"service magisk_service /sbin/magisk  --service\n"
"   class late_start\n"
"   user root\n"
"   seclabel u:r:su:s0\n"
"   oneshot\n"
;
