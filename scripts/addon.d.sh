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
  MAGISKBIN=/data/adb/magisk

  mount /data 2>/dev/null

  if [ ! -d $MAGISKBIN ]; then
    echo "! Cannot find Magisk binaries!"
    exit 1
  fi

  # Wait for post addon.d processes to finish
  sleep 5

  # Load utility functions
  . $MAGISKBIN/util_functions.sh

  ui_print "************************"
  ui_print "* Magisk v$MAGISK_VER addon.d"
  ui_print "************************"

  mount_partitions

  api_level_arch_detect

  recovery_actions

  remove_system_su

  [ -z $BOOTIMAGE ] && abort "! Unable to detect boot image"
  ui_print "- Found boot image: $BOOTIMAGE"

  SOURCEDMODE=true
  cd $MAGISKBIN

  # Source the boot patcher
  . $MAGISKBIN/boot_patch.sh "$BOOTIMAGE"

  flash_boot_image new-boot.img "$BOOTIMAGE"
  rm -f new-boot.img

  if [ -f stock_boot* ]; then
    rm -f /data/stock_boot* 2>/dev/null
    mv stock_boot* /data
  fi

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
