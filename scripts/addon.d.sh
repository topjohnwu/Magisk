##########################################################################################
#
# Magisk Survival Script for ROMs with addon.d support
# by topjohnwu
#
# Inspired by 99-flashafterupdate.sh of osm0sis @ xda-developers
#
##########################################################################################

V1_FUNCS=/tmp/backuptool.functions
V2_FUNCS=/postinstall/system/bin/backuptool_ab.functions

if [ -f $V1_FUNCS ]; then
  . $V1_FUNCS
  backuptool_ab=false
elif [ -f $V2_FUNCS ]; then
  . $V2_FUNCS
else
  return 1
fi

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
}

show_logo() {
  ui_print "************************"
  ui_print "* Magisk v$MAGISK_VER addon.d"
  ui_print "************************"
}

installation() {
  find_manager_apk
  get_flags
  find_boot_image
  find_dtbo_image
  [ -z $BOOTIMAGE ] && abort "! Unable to detect target image"
  ui_print "- Target image: $BOOTIMAGE"
  [ -z $DTBOIMAGE ] || ui_print "- DTBO image: $DTBOIMAGE"

  remove_system_su

  [ -f $APK ] && eval $BOOTSIGNER -verify < $BOOTIMAGE && BOOTSIGNED=true
  $BOOTSIGNED && ui_print "- Boot image is signed with AVB 1.0"

  SOURCEDMODE=true
  cd $MAGISKBIN

  # Source the boot patcher
  . ./boot_patch.sh "$BOOTIMAGE"

  ui_print "- Flashing new boot image"
  flash_image new-boot.img "$BOOTIMAGE" || abort "! Insufficient partition size"
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
  installation
  recovery_cleanup
  finalize
}

main_v2() {
  show_logo
  mount_partitions
  # Swap the slot
  if [ ! -z $SLOT ]; then [ $SLOT = _a ] && SLOT=_b || SLOT=_a; fi
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
    if $backuptool_ab; then
      exec su -c "sh $0 addond-v2"
    else
      initialize
      OUTFD=
      setup_flashable
      # Run in background, hack for addon.d-v1
      (main_v1) &
    fi
  ;;
  addond-v2)
    initialize
    # Override ui_print
    ui_print() { log -t Magisk -- "$1"; }
    # addon.d-v2
    main_v2
  ;;
esac
