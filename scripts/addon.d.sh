#!/sbin/sh
# ADDOND_VERSION=2
########################################################
#
# Magisk Survival Script for ROMs with addon.d support
# by topjohnwu and osm0sis
#
########################################################

trampoline() {
  mount /data 2>/dev/null
  if [ -f $MAGISKBIN/addon.d.sh ]; then
    exec sh $MAGISKBIN/addon.d.sh "$@"
    exit $?
  elif [ "$1" = post-restore ]; then
    BOOTMODE=false
    ps | grep zygote | grep -v grep >/dev/null && BOOTMODE=true
    $BOOTMODE || ps -A 2>/dev/null | grep zygote | grep -v grep >/dev/null && BOOTMODE=true

    if ! $BOOTMODE; then
      # update-binary|updater <RECOVERY_API_VERSION> <OUTFD> <ZIPFILE>
      OUTFD=$(ps | grep -v 'grep' | grep -oE 'update(.*) 3 [0-9]+' | cut -d" " -f3)
      [ -z $OUTFD ] && OUTFD=$(ps -Af | grep -v 'grep' | grep -oE 'update(.*) 3 [0-9]+' | cut -d" " -f3)
      # update_engine_sideload --payload=file://<ZIPFILE> --offset=<OFFSET> --headers=<HEADERS> --status_fd=<OUTFD>
      [ -z $OUTFD ] && OUTFD=$(ps | grep -v 'grep' | grep -oE 'status_fd=[0-9]+' | cut -d= -f2)
      [ -z $OUTFD ] && OUTFD=$(ps -Af | grep -v 'grep' | grep -oE 'status_fd=[0-9]+' | cut -d= -f2)
    fi
    ui_print() {
      if $BOOTMODE; then
        log -t Magisk -- "$1"
      else
        echo -e "ui_print $1\nui_print" >> /proc/self/fd/$OUTFD
      fi
    }

    ui_print "***********************"
    ui_print " Magisk addon.d failed"
    ui_print "***********************"
    ui_print "! Cannot find Magisk binaries - was data wiped or not decrypted?"
    ui_print "! Reflash OTA from decrypted recovery or reflash Magisk"
  fi
  exit 1
}

# Always use the script in /data
MAGISKBIN=/data/adb/magisk
[ "$0" = $MAGISKBIN/addon.d.sh ] || trampoline "$@"

V1_FUNCS=/tmp/backuptool.functions
V2_FUNCS=/postinstall/tmp/backuptool.functions

if [ -f $V1_FUNCS ]; then
  . $V1_FUNCS
  backuptool_ab=false
elif [ -f $V2_FUNCS ]; then
  . $V2_FUNCS
else
  return 1
fi

initialize() {
  # Load utility functions
  . $MAGISKBIN/util_functions.sh

  if $BOOTMODE; then
    # Override ui_print when booted
    ui_print() { log -t Magisk -- "$1"; }
  fi
  OUTFD=
  setup_flashable
}

main() {
  if ! $backuptool_ab; then
    # Restore PREINITDEVICE from previous A-only partition
    if [ -f config.orig ]; then
      PREINITDEVICE=$(grep_prop PREINITDEVICE config.orig)
      rm config.orig
    fi

    # Wait for post addon.d-v1 processes to finish
    sleep 5
  fi

  # Ensure we aren't in /tmp/addon.d anymore (since it's been deleted by addon.d)
  mkdir -p $TMPDIR
  cd $TMPDIR

  if echo $MAGISK_VER | grep -q '\.'; then
    PRETTY_VER=$MAGISK_VER
  else
    PRETTY_VER="$MAGISK_VER($MAGISK_VER_CODE)"
  fi
  print_title "Magisk $PRETTY_VER addon.d"

  mount_partitions
  check_data
  get_flags

  if $backuptool_ab; then
    # Swap the slot for addon.d-v2
    if [ ! -z $SLOT ]; then
      case $SLOT in
        _a) SLOT=_b;;
        _b) SLOT=_a;;
      esac
    fi
  fi

  find_boot_image
  [ -z $BOOTIMAGE ] && abort "! Unable to detect target image"
  ui_print "- Target image: $BOOTIMAGE"

  api_level_arch_detect
  ui_print "- Device platform: $ABI"

  remove_system_su
  install_magisk

  # Cleanups
  cd /
  $BOOTMODE || recovery_cleanup
  rm -rf $TMPDIR

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
    # Back up PREINITDEVICE from existing partition before OTA on A-only devices
    if ! $backuptool_ab; then
      initialize
      RECOVERYMODE=false
      find_boot_image
      $MAGISKBIN/magiskboot unpack "$BOOTIMAGE"
      $MAGISKBIN/magiskboot cpio ramdisk.cpio "extract .backup/.magisk config.orig"
      $MAGISKBIN/magiskboot cleanup
    fi
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
      su=sh
      $BOOTMODE && su=su
      exec $su -c "sh $0 addond-v2"
    else
      # Run in background, hack for addon.d-v1
      (main) &
    fi
  ;;
  addond-v2)
    initialize
    main
  ;;
esac
