mount_partitions() {
  [ "`getprop ro.build.ab_update`" = "true" ] && SLOT=`getprop ro.boot.slot_suffix` || SLOT=
  [ "`getprop ro.build.system_root_image`" = "true" ] && SYSTEM_ROOT=true || SYSTEM_ROOT=false
}

get_flags() {
  $SYSTEM_ROOT && KEEPVERITY=true || KEEPVERITY=false
  [ "`getprop ro.crypto.state`" = "encrypted" ] && KEEPFORCEENCRYPT=true || KEEPFORCEENCRYPT=false
}
