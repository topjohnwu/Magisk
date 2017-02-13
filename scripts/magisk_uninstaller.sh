#!/system/bin/sh

[ -z $BOOTMODE ] && BOOTMODE=false
TMPDIR=/tmp
($BOOTMODE) && TMPDIR=/dev/tmp

BINDIR=/data/magisk
CHROMEDIR=$BINDIR/chromeos

NEWBOOT=$TMPDIR/boottmp/new-boot.img
UNPACKDIR=$TMPDIR/boottmp/bootunpack
RAMDISK=$TMPDIR/boottmp/ramdisk

SYSTEMLIB=/system/lib
[ -d /system/lib64 ] && SYSTEMLIB=/system/lib64

ui_print() {
  echo "$1"
}

grep_prop() {
  REGEX="s/^$1=//p"
  shift
  FILES=$@
  if [ -z "$FILES" ]; then
    FILES='/system/build.prop'
  fi
  cat $FILES 2>/dev/null | sed -n $REGEX | head -n 1
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

unpack_boot() {
  rm -rf $UNPACKDIR $RAMDISK 2>/dev/null
  mkdir -p $UNPACKDIR
  mkdir -p $RAMDISK
  cd $UNPACKDIR
  LD_LIBRARY_PATH=$SYSTEMLIB $BINDIR/bootimgtools --extract $1

  [ ! -f $UNPACKDIR/ramdisk.gz ] && return 1

  cd $RAMDISK
  gunzip -c < $UNPACKDIR/ramdisk.gz | cpio -i
}

repack_boot() {
  cd $RAMDISK
  find . | cpio -o -H newc 2>/dev/null | gzip -9 > $UNPACKDIR/ramdisk.gz
  cd $UNPACKDIR
  LD_LIBRARY_PATH=$SYSTEMLIB $BINDIR/bootimgtools --repack $BOOTIMAGE
  if [ -f chromeos ]; then
    echo " " > config
    echo " " > bootloader
    LD_LIBRARY_PATH=$SYSTEMLIB $CHROMEDIR/futility vbutil_kernel --pack new-boot.img.signed --keyblock $CHROMEDIR/kernel.keyblock --signprivate $CHROMEDIR/kernel_data_key.vbprivk --version 1 --vmlinuz new-boot.img --config config --arch arm --bootloader bootloader --flags 0x1
    rm -f new-boot.img
    mv new-boot.img.signed new-boot.img
  fi
  if ($SAMSUNG); then
    SAMSUNG_CHECK=$(cat new-boot.img | grep SEANDROIDENFORCE)
    if [ $? -ne 0 ]; then
      echo -n "SEANDROIDENFORCE" >> new-boot.img
    fi
  fi
  if ($LGE_G); then
    # Prevent secure boot error on LG G2/G3.
    # Just for know, It's a pattern which bootloader verifies at boot. Thanks to LG hackers.
    echo -n -e "\x41\xa9\xe4\x67\x74\x4d\x1d\x1b\xa4\x29\xf2\xec\xea\x65\x52\x79" >> new-boot.img
  fi
  mv new-boot.img $NEWBOOT
}

# Environments
# Set permissions
chmod -R 755 $CHROMEDIR/futility $BINDIR 2>/dev/null
# Temporary busybox for installation
mkdir -p $TMPDIR/busybox
$BINDIR/busybox --install -s $TMPDIR/busybox
rm -f $TMPDIR/busybox/su $TMPDIR/busybox/sh $TMPDIR/busybox/reboot
PATH=$TMPDIR/busybox:$PATH

# Find the boot image
find_boot_image
if [ -z "$BOOTIMAGE" ]; then
  ui_print "! Unable to detect boot image"
  exit 1
fi

ui_print "- Found Boot Image: $BOOTIMAGE"

# Detect special vendors 
SAMSUNG=false
SAMSUNG_CHECK=$(cat /system/build.prop | grep "ro.build.fingerprint=" | grep -i "samsung")
if [ $? -eq 0 ]; then
  SAMSUNG=true
fi
LGE_G=false
RBRAND=$(grep_prop ro.product.brand)
RMODEL=$(grep_prop ro.product.device)
if [ "$RBRAND" = "lge" ] || [ "$RBRAND" = "LGE" ];  then 
  if [ "$RMODEL" = "d800" ] ||
     [ "$RMODEL" = "d801" ] ||
     [ "$RMODEL" = "d802" ] ||
     [ "$RMODEL" = "d803" ] || 
     [ "$RMODEL" = "ls980" ] ||
     [ "$RMODEL" = "vs980" ] ||
     [ "$RMODEL" = "l01f" ] || 
     [ "$RMODEL" = "d850" ] ||
     [ "$RMODEL" = "d852" ] ||
     [ "$RMODEL" = "d855" ] ||
     [ "$RMODEL" = "ls990" ] ||
     [ "$RMODEL" = "vs985" ] ||
     [ "$RMODEL" = "f400" ]; then
    LGE_G=true
    ui_print "! Bump device detected"
  fi
fi

# First unpack the boot image
unpack_boot $BOOTIMAGE
if [ $? -ne 0 ]; then
  ui_print "! Unable to unpack boot image"
  exit 1
fi

# Detect boot image state
SUPERSU=false
[ -f sbin/launch_daemonsu.sh ] && SUPERSU=true
if [ ! -f init.magisk.rc ]; then
  ui_print "! Magisk is not installed!"
  exit 1
fi

if ($SUPERSU); then
  ui_print "- SuperSU patched image detected"
  rm -rf magisk init.magisk.rc sbin/magic_mask.sh
  repack_boot
else
  if [ -f /data/stock_boot.img ]; then
    ui_print "- Boot image backup found!"
    NEWBOOT=/data/stock_boot.img
  else
    ui_print "! Boot image backup unavailable"
    if [ -d ".backup" ]; then
      ui_print "- Restoring ramdisk with backup"
      cp -af .backup/. .
    fi
    rm -rf magisk init.magisk.rc sbin/magic_mask.sh .backup
    repack_boot
  fi
fi

chmod 644 $NEWBOOT

ui_print "- Flashing stock/reverted image"
[ ! -L "$BOOTIMAGE" ] && dd if=/dev/zero of=$BOOTIMAGE bs=4096 2>/dev/null
dd if=$NEWBOOT of=$BOOTIMAGE bs=4096

ui_print "- Removing Magisk files"
rm -rf  /cache/magisk.log /cache/last_magisk.log /cache/magiskhide.log /cache/.disable_magisk \
        /cache/magisk /cache/magisk_merge /cache/magisk_mount  /cache/unblock /cache/magisk_uninstaller.sh \
        /data/Magisk.apk /data/magisk.apk /data/magisk.img /data/magisk_merge.img \
        /data/busybox /data/magisk /data/custom_ramdisk_patch.sh 2>/dev/null

($BOOTMODE) && reboot
