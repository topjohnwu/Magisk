# Detect whether in boot mode
ps | grep zygote | grep -v grep >/dev/null && BOOTMODE=true || BOOTMODE=false

# This path should work in any cases
TMPDIR=/dev/tmp

BOOTTMP=$TMPDIR/boottmp
MAGISKBIN=/data/magisk
CHROMEDIR=$MAGISKBIN/chromeos

SYSTEMLIB=/system/lib
[ -d /system/lib64 ] && SYSTEMLIB=/system/lib64

# Default permissions
umask 022

ui_print_wrapper() {
  type ui_print_wrapper >/dev/null && ui_print "$1" || echo "$1"
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
# Temporary busybox for installation
mkdir -p $TMPDIR/busybox
$MAGISKBIN/busybox --install -s $TMPDIR/busybox
rm -f $TMPDIR/busybox/su $TMPDIR/busybox/sh $TMPDIR/busybox/reboot
PATH=$TMPDIR/busybox:$PATH

# Find the boot image
find_boot_image
if [ -z "$BOOTIMAGE" ]; then
  ui_print_wrapper "! Unable to detect boot image"
  exit 1
fi

ui_print_wrapper "- Found Boot Image: $BOOTIMAGE"

rm -rf $BOOTTMP 2>/dev/null
mkdir -p $BOOTTMP
cd $BOOTTMP

ui_print_wrapper "- Unpacking boot image"
LD_LIBRARY_PATH=$SYSTEMLIB $MAGISKBIN/magiskboot --unpack $BOOTIMAGE
if [ $? -ne 0 ]; then
  ui_print_wrapper "! Unable to unpack boot image"
  exit 1
fi

# Update our previous backup to new format if exists
if [ -f /data/stock_boot.img ]; then
  SHA1=`LD_LIBRARY_PATH=$SYSTEMLIB $MAGISKBIN/magiskboot --sha1 /data/stock_boot.img | tail -n 1`
  STOCKDUMP=/data/stock_boot_${SHA1}.img
  mv /data/stock_boot.img $STOCKDUMP
  LD_LIBRARY_PATH=$SYSTEMLIB $MAGISKBIN/magiskboot --compress $STOCKDUMP
fi

# Detect boot image state
LD_LIBRARY_PATH=$SYSTEMLIB $MAGISKBIN/magiskboot --cpio-test ramdisk.cpio
case $? in
  0 )
    ui_print_wrapper "! Magisk is not installed!"
    ui_print_wrapper "! Nothing to uninstall"
    exit
    ;;
  1 )
    # Find SHA1 of stock boot image
    if [ -z $SHA1 ]; then
      LD_LIBRARY_PATH=$SYSTEMLIB $MAGISKBIN/magiskboot --cpio-extract ramdisk.cpio init.magisk.rc init.magisk.rc
      SHA1=`grep_prop "# STOCKSHA1" init.magisk.rc`
      [ ! -z $SHA1 ] && STOCKDUMP=/data/stock_boot_${SHA1}.img
      rm -f init.magisk.rc
    fi
    if [ -f ${STOCKDUMP}.gz ]; then
      ui_print_wrapper "- Boot image backup found!"
      LD_LIBRARY_PATH=$SYSTEMLIB $MAGISKBIN/magiskboot --decompress ${STOCKDUMP}.gz stock_boot.img
    else
      ui_print_wrapper "! Boot image backup unavailable"
      ui_print_wrapper "- Restoring ramdisk with backup"
      LD_LIBRARY_PATH=$SYSTEMLIB $MAGISKBIN/magiskboot --cpio-restore ramdisk.cpio
      LD_LIBRARY_PATH=$SYSTEMLIB $MAGISKBIN/magiskboot --repack $BOOTIMAGE stock_boot.img
    fi
    ;;
  2 )
    ui_print_wrapper "- SuperSU patched image detected"
    LD_LIBRARY_PATH=$SYSTEMLIB $MAGISKBIN/magiskboot --cpio-restore ramdisk.cpio
    LD_LIBRARY_PATH=$SYSTEMLIB $MAGISKBIN/magiskboot --repack $BOOTIMAGE stock_boot.img
    ;;
esac

# Sign chromeos boot
if [ -f chromeos ]; then
  echo > config
  echo > bootloader
  LD_LIBRARY_PATH=$SYSTEMLIB $CHROMEDIR/futility vbutil_kernel --pack stock_boot.img.signed --keyblock $CHROMEDIR/kernel.keyblock --signprivate $CHROMEDIR/kernel_data_key.vbprivk --version 1 --vmlinuz stock_boot.img --config config --arch arm --bootloader bootloader --flags 0x1
  rm -f stock_boot.img
  mv stock_boot.img.signed stock_boot.img
fi

ui_print_wrapper "- Flashing stock/reverted image"
[ ! -L "$BOOTIMAGE" ] && dd if=/dev/zero of=$BOOTIMAGE bs=4096 2>/dev/null
dd if=stock_boot.img of=$BOOTIMAGE bs=4096

ui_print_wrapper "- Removing Magisk files"
rm -rf  /cache/magisk.log /cache/last_magisk.log /cache/magiskhide.log /cache/.disable_magisk \
        /cache/magisk /cache/magisk_merge /cache/magisk_mount  /cache/unblock /cache/magisk_uninstaller.sh \
        /data/Magisk.apk /data/magisk.apk /data/magisk.img /data/magisk_merge.img \
        /data/busybox /data/magisk /data/custom_ramdisk_patch.sh 2>/dev/null

$BOOTMODE && reboot
