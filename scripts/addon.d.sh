#!/sbin/sh
##########################################################################################
#
# Magisk Survival Script for ROMs with addon.d support
# by topjohnwu
#
# Inspired by 99-flashafterupdate.sh of osm0sis @ xda-developers
#
##########################################################################################

. /tmp/backuptool.functions
[ -z $backuptool_ab ] && backuptool_ab=false

initialize() {
  # This path should work in any cases
  TMPDIR=/dev/tmp

  mount /data 2>/dev/null

  MAGISKBIN=/data/adb/magisk
  if [ ! -d $MAGISKBIN ]; then
    echo "! Cannot find Magisk binaries!"
    exit 1
  fi

  # Load utility functions
  . $MAGISKBIN/util_functions.sh

  APK=/data/adb/magisk.apk
  [ -f $APK ] || APK=/data/magisk/magisk.apk
  [ -f $APK ] || APK=/data/app/com.topjohnwu.magisk*/*.apk
}

show_logo() {
  ui_print "************************"
  ui_print "* Magisk v$MAGISK_VER addon.d"
  ui_print "************************"
}

detection() {
  find_boot_image
  find_dtbo_image
  [ -z $BOOTIMAGE ] && abort "! Unable to detect target image"
  ui_print "- Target image: $BOOTIMAGE"
  [ -z $DTBOIMAGE ] || ui_print "- DTBO image: $DTBOIMAGE"
  get_flags
}

installation() {
  remove_system_su

  [ -f $APK ] && eval $BOOTSIGNER -verify < $BOOTIMAGE && BOOTSIGNED=true
  $BOOTSIGNED && ui_print "- Boot image is signed with AVB 1.0"

  SOURCEDMODE=true
  cd $MAGISKBIN

  # Source the boot patcher
  . ./boot_patch.sh "$BOOTIMAGE"

  flash_boot_image new-boot.img "$BOOTIMAGE"
  rm -f new-boot.img

  if [ -f stock_boot* ]; then
    rm -f /data/stock_boot* 2>/dev/null
    $DATA && mv stock_boot* /data
  fi

  $KEEPVERITY || patch_dtbo_image

  if [ -f stock_dtbo* ]; then
    rm -f /data/stock_dtbo* 2>/dev/null
    $DATA && mv stock_dtbo* /data
  fi

  cd /
}

finalize() {
  ui_print "- Done"
  exit 0
}

main_v1() {
  # Wait for post addon.d processes to finish
  sleep 5
  recovery_actions
  show_logo
  mount_partitions
  detection
  installation
  recovery_cleanup
  finalize
}

main_v2() {
  boot_actions
  show_logo
  mount_partitions
  # Swap the slot
  if [ ! -z $SLOT ]; then [ $SLOT = _a ] && SLOT=_b || SLOT=_a; fi
  detection
  installation
  finalize
}

case "$1" in
  backup)
    # Stub
  ;;
  restore)
    # Stub
  ;;
  pre-backup)
    # Stub
  ;;
  post-backup)
    # Stub
  ;;
  pre-restore)
    # Stub
  ;;
  post-restore)
    initialize
    if $backuptool_ab; then
      # addon.d-v2
      main_v2
    else
      OUTFD=
      get_outfd
      # Run in background, hack for addon.d-v1
      (main_v1) &
    fi
  ;;
esac
