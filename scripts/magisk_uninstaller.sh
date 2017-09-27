#!/system/bin/sh
##########################################################################################
#
# Magisk Uninstaller
# by topjohnwu
#
# This script can be placed in /cache/magisk_uninstaller.sh
# The Magisk main binary will pick up the script, and uninstall itself, following a reboot
# This script can also be used in flashable zip with the uninstaller_loader.sh
#
# This script will try to do restoration with the following:
# 1-1. Find and restore the original stock boot image dump (OTA proof)
# 1-2. If 1-1 fails, restore ramdisk from the internal backup
#      (ramdisk fully restored, not OTA friendly)
# 1-3. If 1-2 fails, it will remove added files in ramdisk, however modified files
#      are remained modified, because we have no backups. By doing so, Magisk will
#      not be started at boot, but this isn't actually 100% cleaned up
# 2. Remove all Magisk related files
#    (The list is LARGE, most likely due to bad decision in early versions
#    the latest versions has much less bloat to cleanup)
#
##########################################################################################

[ -z $BOOTMODE ] && BOOTMODE=false

MAGISKBIN=/data/magisk
CHROMEDIR=$MAGISKBIN/chromeos

if [ ! -f $MAGISKBIN/magiskboot -o ! -f $MAGISKBIN/util_functions.sh ]; then
  echo "! Cannot find $MAGISKBIN"
  exit 1
fi

if $BOOTMODE; then
  # Load utility functions
  . $MAGISKBIN/util_functions.sh
  boot_actions
  mount_partitions
fi

cd $MAGISKBIN

# Find the boot image
find_boot_image
[ -z $BOOTIMAGE ] && abort "! Unable to detect boot image"

ui_print "- Found Boot Image: $BOOTIMAGE"

migrate_boot_backup

ui_print "- Unpacking boot image"
./magiskboot --unpack "$BOOTIMAGE"
CHROMEOS=false
case $? in
  1 )
    abort "! Unable to unpack boot image"
    ;;
  2 )
    CHROMEOS=true
    ;;
  3 )
    ui_print "! Sony ELF32 format detected"
    abort "! Please use BootBridge from @AdrianDC to flash Magisk"
    ;;
  4 )
    ui_print "! Sony ELF64 format detected"
    abort "! Stock kernel cannot be patched, please use a custom kernel"
esac

# Detect boot image state
ui_print "- Checking ramdisk status"
./magiskboot --cpio-test ramdisk.cpio
case $? in
  0 )  # Stock boot
    ui_print "- Stock boot image detected!"
    abort "! Magisk is not installed!"
    ;;
  1 )  # Magisk patched
    ui_print "- Magisk patched image detected!"
    # Find SHA1 of stock boot image
    [ -z $SHA1 ] && SHA1=`./magiskboot --cpio-stocksha1 ramdisk.cpio 2>/dev/null`
    [ ! -z $SHA1 ] && STOCKBOOT=/data/stock_boot_${SHA1}.img.gz
    if [ -f "$STOCKBOOT" ]; then
      ui_print "- Boot image backup found!"
    else
      ui_print "! Boot image backup unavailable"
      ui_print "- Restoring ramdisk with internal backup"
      ./magiskboot --cpio-restore ramdisk.cpio
      ./magiskboot --repack $BOOTIMAGE
      # Sign chromeos boot
      $CHROMEOS && sign_chromeos
      STOCKBOOT=new-boot.img
    fi
    ;;
  2 ) # Other patched
    ui_print "! Boot image patched by other programs!"
    abort "! Cannot uninstall"
    ;;
esac

flash_boot_image $STOCKBOOT "$BOOTIMAGE"

cd /

ui_print "- Removing Magisk files"
rm -rf  /cache/*magisk* /cache/unblock /data/*magisk* /data/cache/*magisk* /data/property/*magisk* \
        /data/Magisk.apk /data/busybox /data/custom_ramdisk_patch.sh  \
        /data/app/com.topjohnwu.magisk* /data/user*/*/com.topjohnwu.magisk 2>/dev/null

$BOOTMODE && reboot
