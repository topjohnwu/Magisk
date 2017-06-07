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
# This script will try to do restoration in the following order:
# 1. Find and restore the original stock boot image dump (OTA proof)
# 2. Restore ramdisk from the internal backup (ramdisk fully restored, not OTA friendly)
# 3. Remove added files in ramdisk, modified files are remained intact. By doing so, Magisk
#    will not be started at boot, but not removed clean enough
# 
# Finally, this uninstaller will remove all Magisk related files
# (The list is LARGE, most likely due to bad decision in early versions
# the latest versions has much less bloat to cleanup)
#
##########################################################################################


[ -z $BOOTMODE ] && BOOTMODE=false

MAGISKBIN=/data/magisk
CHROMEDIR=$MAGISKBIN/chromeos

[ -d /system/lib64 ] && SYSTEMLIB=/system/lib64 || SYSTEMLIB=/system/lib

# Default permissions
umask 022

# Call ui_print_wrap if exists, or else simply use echo
# Useful when wrapped in flashable zip
ui_print_wrap() {
  type ui_print >/dev/null 2>&1 && ui_print "$1" || echo "$1"
}

grep_prop() {
  REGEX="s/^$1=//p"
  shift
  FILES=$@
  if [ -z "$FILES" ]; then
    FILES='/system/build.prop'
  fi
  cat $FILES 2>/dev/null | sed -n "$REGEX" | head -n 1
}

find_boot_image() {
  if [ -z "$BOOTIMAGE" ]; then
    for PARTITION in kern-a KERN-A android_boot ANDROID_BOOT kernel KERNEL boot BOOT lnx LNX; do
      BOOTIMAGE=`readlink /dev/block/by-name/$PARTITION || readlink /dev/block/platform/*/by-name/$PARTITION || readlink /dev/block/platform/*/*/by-name/$PARTITION`
      if [ ! -z "$BOOTIMAGE" ]; then break; fi
    done
  fi
  if [ -z "$BOOTIMAGE" ]; then
    FSTAB="/etc/recovery.fstab"
    [ ! -f "$FSTAB" ] && FSTAB="/etc/recovery.fstab.bak"
    [ -f "$FSTAB" ] && BOOTIMAGE=`grep -E '\b/boot\b' "$FSTAB" | grep -oE '/dev/[a-zA-Z0-9_./-]*'`
  fi
}

# Environments
# Set permissions
chmod -R 755 $CHROMEDIR/futility $MAGISKBIN 2>/dev/null

# Find the boot image
find_boot_image
if [ -z "$BOOTIMAGE" ]; then
  ui_print_wrap "! Unable to detect boot image"
  exit 1
fi

ui_print_wrap "- Found Boot Image: $BOOTIMAGE"

cd $MAGISKBIN

ui_print_wrap "- Unpacking boot image"
LD_LIBRARY_PATH=$SYSTEMLIB ./magiskboot --unpack $BOOTIMAGE
if [ $? -ne 0 ]; then
  ui_print_wrap "! Unable to unpack boot image"
  exit 1
fi

# Update our previous backups to new format if exists
if [ -f /data/stock_boot.img ]; then
  SHA1=`LD_LIBRARY_PATH=$SYSTEMLIB ./magiskboot --sha1 /data/stock_boot.img | tail -n 1`
  STOCKDUMP=/data/stock_boot_${SHA1}.img
  mv /data/stock_boot.img $STOCKDUMP
  LD_LIBRARY_PATH=$SYSTEMLIB ./magiskboot --compress $STOCKDUMP
fi

# Detect boot image state
LD_LIBRARY_PATH=$SYSTEMLIB ./magiskboot --cpio-test ramdisk.cpio
case $? in
  0 )
    ui_print_wrap "- Stock boot image detected!"
    ui_print_wrap "! Magisk is not installed!"
    exit
    ;;
  1 )
    ui_print_wrap "- Magisk patched image detected!"
    # Find SHA1 of stock boot image
    if [ -z $SHA1 ]; then
      LD_LIBRARY_PATH=$SYSTEMLIB ./magiskboot --cpio-extract ramdisk.cpio init.magisk.rc init.magisk.rc.old
      SHA1=`grep_prop "# STOCKSHA1" init.magisk.rc.old`
      [ ! -z $SHA1 ] && STOCKDUMP=/data/stock_boot_${SHA1}.img
      rm -f init.magisk.rc.old
    fi
    if [ -f ${STOCKDUMP}.gz ]; then
      ui_print_wrap "- Boot image backup found!"
      LD_LIBRARY_PATH=$SYSTEMLIB ./magiskboot --decompress ${STOCKDUMP}.gz stock_boot.img
    else
      ui_print_wrap "! Boot image backup unavailable"
      ui_print_wrap "- Restoring ramdisk with backup"
      LD_LIBRARY_PATH=$SYSTEMLIB ./magiskboot --cpio-restore ramdisk.cpio
      LD_LIBRARY_PATH=$SYSTEMLIB ./magiskboot --repack $BOOTIMAGE stock_boot.img
    fi
    ;;
  2 ) # Other patched
      ui_print_wrap "! Boot image patched by other programs!"
      ui_print_wrap "! Cannot uninstall with this uninstaller"
      exit 1
    ;;
esac

# Sign chromeos boot
if [ -f chromeos ]; then
  echo > empty

  LD_LIBRARY_PATH=$SYSTEMLIB $CHROMEDIR/futility vbutil_kernel --pack stock_boot.img.signed \
  --keyblock $CHROMEDIR/kernel.keyblock --signprivate $CHROMEDIR/kernel_data_key.vbprivk \
  --version 1 --vmlinuz stock_boot.img --config empty --arch arm --bootloader empty --flags 0x1

  rm -f empty stock_boot.img
  mv stock_boot.img.signed stock_boot.img
fi

ui_print_wrap "- Flashing stock/reverted image"
if [ -L $BOOTIMAGE ]; then
  dd if=stock_boot.img of=$BOOTIMAGE bs=4096
else
  cat stock_boot.img /dev/zero | dd of=$BOOTIMAGE bs=4096
fi
rm -f stock_boot.img

ui_print_wrap "- Removing Magisk files"
rm -rf  /cache/magisk.log /cache/last_magisk.log /cache/magiskhide.log /cache/.disable_magisk \
        /cache/magisk /cache/magisk_merge /cache/magisk_mount  /cache/unblock /cache/magisk_uninstaller.sh \
        /data/Magisk.apk /data/magisk.apk /data/magisk.img /data/magisk_merge.img /data/magisk_debug.log \
        /data/busybox /data/magisk /data/custom_ramdisk_patch.sh 2>/dev/null

$BOOTMODE && reboot
