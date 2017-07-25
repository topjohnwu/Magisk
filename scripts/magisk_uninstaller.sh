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

# Call ui_print_wrap if exists, or else simply use echo
# Useful when wrapped in flashable zip
ui_print_wrap() {
  type ui_print >/dev/null 2>&1 && ui_print "$1" || echo "$1"
}

# Call abort if exists, or else show error message and exit
# Essential when wrapped in flashable zip
abort_wrap() {
  type abort >/dev/null 2>&1
  if [ $? -ne 0 ]; then
    ui_print_wrap "$1"
    exit 1
  else
    abort "$1"
  fi
}

if [ ! -d $MAGISKBIN -o ! -f $MAGISKBIN/magiskboot -o ! -f $MAGISKBIN/util_functions.sh ]; then
  ui_print_wrap "! Cannot find $MAGISKBIN"
  exit 1
fi

[ -z $BOOTMODE ] && BOOTMODE=false

MAGISKBIN=/data/magisk
CHROMEDIR=$MAGISKBIN/chromeos

# Default permissions
umask 022

# Load utility functions
. $MAGISKBIN/util_functions.sh

isABDevice=false
SYSTEM=/system
ABdevice_check

# Find the boot image
find_boot_image
[ -z $BOOTIMAGE ] && abort "! Unable to detect boot image"

ui_print_wrap "- Found Boot Image: $BOOTIMAGE"

cd $MAGISKBIN

ui_print_wrap "- Unpacking boot image"
./magiskboot --unpack "$BOOTIMAGE"
CHROMEOS=false
case $? in
  1 )
    abort_wrap "! Unable to unpack boot image"
    ;;
  2 )
    CHROMEOS=true
    ;;
  3 )
    ui_print_wrap "! Sony ELF32 format detected"
    abort_wrap "! Please use BootBridge from @AdrianDC to flash Magisk"
    ;;
  4 )
    ui_print_wrap "! Sony ELF64 format detected"
    abort_wrap "! Stock kernel cannot be patched, please use a custom kernel"
esac

# Update our previous backup to new format if exists
if [ -f /data/stock_boot.img ]; then
  SHA1=`./magiskboot --sha1 /data/stock_boot.img | tail -n 1`
  STOCKDUMP=/data/stock_boot_${SHA1}.img
  mv /data/stock_boot.img $STOCKDUMP
  ./magiskboot --compress $STOCKDUMP
fi

# Detect boot image state
./magiskboot --cpio-test ramdisk.cpio
case $? in
  0 )  # Stock boot
    ui_print_wrap "- Stock boot image detected!"
    ui_print_wrap "! Magisk is not installed!"
    exit
    ;;
  1 )  # Magisk patched
    ui_print_wrap "- Magisk patched image detected!"
    # Find SHA1 of stock boot image
    if [ -z $SHA1 ]; then
      ./magiskboot --cpio-extract ramdisk.cpio init.magisk.rc init.magisk.rc.old
      SHA1=`grep_prop "# STOCKSHA1" init.magisk.rc.old`
      rm -f init.magisk.rc.old
    fi
    [ ! -z $SHA1 ] && STOCKDUMP=/data/stock_boot_${SHA1}.img
    if [ -f ${STOCKDUMP}.gz ]; then
      ui_print_wrap "- Boot image backup found!"
      ./magiskboot --decompress ${STOCKDUMP}.gz stock_boot.img
    else
      ui_print_wrap "! Boot image backup unavailable"
      ui_print_wrap "- Restoring ramdisk with backup"
      ./magiskboot --cpio-restore ramdisk.cpio
      ./magiskboot --repack $BOOTIMAGE stock_boot.img
    fi
    ;;
  2 ) # Other patched
    ui_print_wrap "! Boot image patched by other programs!"
    abort_wrap "! Cannot uninstall with this uninstaller"
    ;;
esac

# Sign chromeos boot
if $CHROMEOS; then
  echo > empty

  ./chromeos/futility vbutil_kernel --pack stock_boot.img.signed \
  --keyblock ./chromeos/kernel.keyblock --signprivate ./chromeos/kernel_data_key.vbprivk \
  --version 1 --vmlinuz stock_boot.img --config empty --arch arm --bootloader empty --flags 0x1

  rm -f empty stock_boot.img
  mv stock_boot.img.signed stock_boot.img
fi

ui_print_wrap "- Flashing stock/reverted image"
if [ -L "$BOOTIMAGE" ]; then
  dd if=stock_boot.img of="$BOOTIMAGE" bs=4096
else
  cat stock_boot.img /dev/zero | dd of="$BOOTIMAGE" bs=4096 >/dev/null 2>&1
fi
rm -f stock_boot.img

ui_print_wrap "- Removing Magisk files"
rm -rf  /cache/magisk.log /cache/last_magisk.log /cache/magiskhide.log /cache/.disable_magisk \
        /cache/magisk /cache/magisk_merge /cache/magisk_mount  /cache/unblock /cache/magisk_uninstaller.sh \
        /data/Magisk.apk /data/magisk.apk /data/magisk.img /data/magisk_merge.img /data/magisk_debug.log \
        /data/busybox /data/magisk /data/custom_ramdisk_patch.sh /data/property/*magisk* \
        /data/app/com.topjohnwu.magisk* /data/user/*/com.topjohnwu.magisk 2>/dev/null

$BOOTMODE && reboot
