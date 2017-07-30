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

main() {
  # Magisk binaries
  MAGISKBIN=/data/magisk
  # This script always run in recovery
  BOOTMODE=false

  if [ ! -d $MAGISKBIN ]; then
    echo "! Cannot find Magisk binaries!"
    exit 1
  fi

  # Wait for post addon.d processes to finish
  sleep 5

  mount -o ro /system 2>/dev/null
  mount -o ro /vendor 2>/dev/null
  mount /data 2>/dev/null

  # Load utility functions
  . $MAGISKBIN/util_functions.sh

  [ -f /system/build.prop ] || abort "! /system could not be mounted!"

  ui_print "************************"
  ui_print "* Magisk v$MAGISK_VER addon.d"
  ui_print "************************"

  api_level_arch_detect

  # Check if system root is installed and remove
  remove_system_su

  recovery_actions

  find_boot_image
  [ -z $BOOTIMAGE ] && abort "! Unable to detect boot image"
  ui_print "- Found Boot Image: $BOOTIMAGE"

  SOURCEDMODE=true
  cd $MAGISKBIN

  # Source the boot patcher
  . $MAGISKBIN/boot_patch.sh "$BOOTIMAGE"

  [ -f stock_boot* ] && rm -f /data/stock_boot* 2>/dev/null

  ui_print "- Flashing new boot image"
  if [ -L "$BOOTIMAGE" ]; then
    dd if=new-boot.img of="$BOOTIMAGE" bs=4096
  else
    cat new-boot.img /dev/zero | dd of="$BOOTIMAGE" bs=4096 >/dev/null 2>&1
  fi
  rm -f new-boot.img

  cd /

  recovery_cleanup

  ui_print "- Done"
  exit 0
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
    # Get the FD for ui_print
    OUTFD=`ps | grep -v grep | grep -oE "update(.*)" | cut -d" " -f3`
    # Run the main function in a parallel subshell
    (main) &
  ;;
esac
