#!/sbin/sh
##########################################################################################
#
# Magisk Flash Script
# by topjohnwu
# 
# This script will detect, construct the environment for Magisk
# It will then call boot_patch.sh to patch the boot image
#
##########################################################################################

# Detect whether in boot mode
ps | grep zygote | grep -v grep >/dev/null && BOOTMODE=true || BOOTMODE=false
$BOOTMODE || ps -A 2>/dev/null | grep zygote | grep -v grep >/dev/null && BOOTMODE=true

# This path should work in any cases
TMPDIR=/dev/tmp

INSTALLER=$TMPDIR/magisk
BUSYBOX=$TMPDIR/busybox
COMMONDIR=$INSTALLER/common
CHROMEDIR=$INSTALLER/chromeos
COREDIR=/magisk/.core

# Default permissions
umask 022

##########################################################################################
# Flashable update-binary preparation
##########################################################################################

OUTFD=$2
ZIP=$3

readlink /proc/$$/fd/$OUTFD 2>/dev/null | grep /tmp >/dev/null
if [ "$?" -eq "0" ]; then
  OUTFD=0

  for FD in `ls /proc/$$/fd`; do
    readlink /proc/$$/fd/$FD 2>/dev/null | grep pipe >/dev/null
    if [ "$?" -eq "0" ]; then
      ps | grep " 3 $FD " | grep -v grep >/dev/null
      if [ "$?" -eq "0" ]; then
        OUTFD=$FD
        break
      fi
    fi
  done
fi

rm -rf $TMPDIR 2>/dev/null
mkdir -p $INSTALLER $BUSYBOX
unzip -o "$ZIP" -d $INSTALLER

##########################################################################################
# Detection
##########################################################################################

# Load all fuctions
. $COMMONDIR/util_functions.sh

ui_print "************************"
ui_print "* MAGISK_VERSION_STUB"
ui_print "************************"

[ -d "$COMMONDIR" ] || abort "! Unable to extract zip file!"

ui_print "- Mounting /system(ro), /cache, /data"
mount -o ro /system 2>/dev/null
mount /cache 2>/dev/null
mount /data 2>/dev/null

[ -f /system/build.prop ] || abort "! /system could not be mounted!"

# read override variables
getvar KEEPVERITY
getvar KEEPFORCEENCRYPT
getvar BOOTIMAGE

# Check if system root is installed and remove
remove_system_su

API=`grep_prop ro.build.version.sdk`
ABI=`grep_prop ro.product.cpu.abi | cut -c-3`
ABI2=`grep_prop ro.product.cpu.abi2 | cut -c-3`
ABILONG=`grep_prop ro.product.cpu.abi`

ARCH=arm
BBPATH=armeabi-v7a
if [ "$ABI" = "x86" ]; then ARCH=x86; BBPATH=x86; fi;
if [ "$ABI2" = "x86" ]; then ARCH=x86; BBPATH=x86; fi;
if [ "$ABILONG" = "arm64-v8a" ]; then ARCH=arm64; fi;
if [ "$ABILONG" = "x86_64" ]; then ARCH=x64; BBPATH=x86; fi;

[ $API -lt 21 ] && abort "! Magisk is only for Lollipop 5.0+ (SDK 21+)"

ui_print "- Device platform: $ARCH"

BINDIR=$INSTALLER/$ARCH
chmod -R 755 $CHROMEDIR $BINDIR

find_boot_image
[ -z $BOOTIMAGE ] && abort "! Unable to detect boot image"

##########################################################################################
# Environment
##########################################################################################

ui_print "- Constructing environment"

if ! $BOOTMODE; then
  # We are in custom recovery, don't trust anything in recovery, use those in /system
  # Extract busybox from Magisk Manager, it's pure static binary so there will be no issues
  unzip -o $COMMONDIR/magisk.apk lib/$BBPATH/libbusybox.so -d $BUSYBOX
  mv $BUSYBOX/lib/*/libbusybox.so $BUSYBOX/busybox
  rm -rf $BUSYBOX/lib
  chmod +x $BUSYBOX/busybox
  $BUSYBOX/busybox --install -s $BUSYBOX
  # Prefer bin in /system, fallback to bundled busybox if a tool doesn't exist
  export PATH=/system/bin:/system/xbin:$BUSYBOX
  # Clear out possible lib paths, let the binary find them itself
  export LD_LIBRARY_PATH=
  # Temporarily block out all custom recovery binaries/libs
  mv /sbin /sbin_tmp
fi

is_mounted /data && MAGISKBIN=/data/magisk || MAGISKBIN=/cache/data_bin

# Copy required files
rm -rf $MAGISKBIN 2>/dev/null
mkdir -p $MAGISKBIN
cp -af $BINDIR/. $COMMONDIR/. $MAGISKBIN

chmod -R 755 $MAGISKBIN
chcon -hR u:object_r:system_file:s0 $MAGISKBIN

# addon.d
if [ -d /system/addon.d ]; then
  ui_print "- Adding addon.d survival script"
  mount -o rw,remount /system
  cp $INSTALLER/addon.d/99-magisk.sh /system/addon.d/99-magisk.sh
  chmod 755 /system/addon.d/99-magisk.sh
fi

##########################################################################################
# Magisk Image
##########################################################################################

# Fix SuperSU.....
$BOOTMODE && $BINDIR/magiskpolicy --live "allow fsck * * *"

if (is_mounted /data); then
  IMG=/data/magisk.img
else
  IMG=/cache/magisk.img
  ui_print "- Data unavailable, use cache workaround"
fi

if [ -f $IMG ]; then
  ui_print "- $IMG detected!"
else
  ui_print "- Creating $IMG"
  $BINDIR/magisk --createimg $IMG 64M
fi

mount_image $IMG /magisk
is_mounted /magisk || abort "! Magisk image mount failed..."
MAGISKLOOP=$LOOPDEVICE

# Core folders
mkdir -p $COREDIR/props $COREDIR/post-fs-data.d $COREDIR/service.d 2>/dev/null

chmod 755 $COREDIR/post-fs-data.d $COREDIR/service.d
chown 0.0 $COREDIR/post-fs-data.d $COREDIR/service.d

# Legacy cleanup
mv $COREDIR/magiskhide/hidelist $COREDIR/hidelist 2>/dev/null
rm -rf $COREDIR/magiskhide $COREDIR/bin

##########################################################################################
# Unpack boot
##########################################################################################

ui_print "- Found Boot Image: $BOOTIMAGE"

# Update our previous backup to new format if exists
if [ -f /data/stock_boot.img ]; then
  SHA1=`$BINDIR/magiskboot --sha1 /data/stock_boot.img | tail -n 1`
  STOCKDUMP=/data/stock_boot_${SHA1}.img
  mv /data/stock_boot.img $STOCKDUMP
  $BINDIR/magiskboot --compress $STOCKDUMP
fi

SOURCEDMODE=true
cd $MAGISKBIN

# Source the boot patcher
. $COMMONDIR/boot_patch.sh "$BOOTIMAGE"

# Sign chromeos boot
if [ -f chromeos ]; then
  echo > empty

  $CHROMEDIR/futility vbutil_kernel --pack new-boot.img.signed \
  --keyblock $CHROMEDIR/kernel.keyblock --signprivate $CHROMEDIR/kernel_data_key.vbprivk \
  --version 1 --vmlinuz new-boot.img --config empty --arch arm --bootloader empty --flags 0x1

  rm -f empty new-boot.img
  mv new-boot.img.signed new-boot.img
fi

[ -f stock_boot* ] && rm -f /data/stock_boot* 2>/dev/null

ui_print "- Flashing new boot image"
if [ -L "$BOOTIMAGE" ]; then
  dd if=new-boot.img of="$BOOTIMAGE" bs=4096
else
  cat new-boot.img /dev/zero | dd of="$BOOTIMAGE" bs=4096
fi
rm -f new-boot.img

cd /

if ! $BOOTMODE; then
  mv /sbin_tmp /sbin
  ui_print "- Unmounting partitions"
  umount /magisk
  losetup -d $MAGISKLOOP 2>/dev/null
  rmdir /magisk
  umount -l /system
fi

ui_print "- Done"
exit 0
