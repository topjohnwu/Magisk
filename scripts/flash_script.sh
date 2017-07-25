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

##########################################################################################
# Preparation
##########################################################################################

# Detect whether in boot mode
ps | grep zygote | grep -v grep >/dev/null && BOOTMODE=true || BOOTMODE=false
$BOOTMODE || ps -A 2>/dev/null | grep zygote | grep -v grep >/dev/null && BOOTMODE=true

# This path should work in any cases
TMPDIR=/dev/tmp

INSTALLER=$TMPDIR/install
COMMONDIR=$INSTALLER/common
CHROMEDIR=$INSTALLER/chromeos
COREDIR=/magisk/.core

# Default permissions
umask 022

OUTFD=$2
ZIP=$3

rm -rf $TMPDIR 2>/dev/null
mkdir -p $INSTALLER
unzip -o "$ZIP" -d $INSTALLER 2>/dev/null

if [ ! -d "$COMMONDIR" ]; then
  echo "! Unable to extract zip file!"
  exit 1
fi

# Load utility fuctions
. $COMMONDIR/util_functions.sh

isABDevice=false
SYSTEM=/system
ABdevice_check

get_outfd

if $isABDevice
then
  cat $COMMONDIR/init.magisk.rc.pixel > $COMMONDIR/init.magisk.rc
fi

##########################################################################################
# Detection
##########################################################################################

ui_print "************************"
ui_print "* MAGISK_VERSION_STUB"
ui_print "************************"

ui_print "- Mounting /system, /vendor, /cache, /data"
mount -o ro /system 2>/dev/null
if $isABDevice
then
  mount -o rw,remount /system 2>/dev/null
fi
mount -o ro /vendor 2>/dev/null
mount /cache 2>/dev/null
mount /data 2>/dev/null

[ -f $SYSTEM/build.prop ] || abort "! /system could not be mounted!"

# read override variables
getvar KEEPVERITY
getvar KEEPFORCEENCRYPT
getvar BOOTIMAGE

# Detect version and architecture
api_level_arch_detect

[ $API -lt 21 ] && abort "! Magisk is only for Lollipop 5.0+ (SDK 21+)"

# Check if system root is installed and remove
remove_system_su

ui_print "- Device platform: $ARCH"

BINDIR=$INSTALLER/$ARCH
chmod -R 755 $CHROMEDIR $BINDIR

find_boot_image
[ -z $BOOTIMAGE ] && abort "! Unable to detect boot image"

##########################################################################################
# Environment
##########################################################################################

ui_print "- Constructing environment"
  
is_mounted /data && MAGISKBIN=/data/magisk || MAGISKBIN=/cache/data_bin

# Copy required files
rm -rf $MAGISKBIN 2>/dev/null
mkdir -p $MAGISKBIN
cp -af $BINDIR/. $COMMONDIR/. $MAGISKBIN
cp -af $CHROMEDIR $MAGISKBIN
chmod -R 755 $MAGISKBIN

# addon.d
if [ -d $SYSTEM/addon.d ]; then
  ui_print "- Adding addon.d survival script"
  mount -o rw,remount /system
  cp $INSTALLER/addon.d/99-magisk.sh $SYSTEM/addon.d/99-magisk.sh
  chmod 755 $SYSTEM/addon.d/99-magisk.sh
fi

##########################################################################################
# Magisk Image
##########################################################################################

if ! $isABDevice
then
  $BOOTMODE || recovery_actions
fi

# Fix SuperSU.....
if $isABDevice
then
  $BOOTMODE && $MAGISKBIN/magisk magiskpolicy --live "allow fsck * * *"
else
  $BOOTMODE && $MAGISKBIN/magiskpolicy --live "allow fsck * * *"
fi

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
  if $isABDevice
  then
    $MAGISKBIN/magisk_utils --createimg $IMG 64M
  else
    $MAGISKBIN/magisk --createimg $IMG 64M
  fi
fi

if ! is_mounted /magisk; then
  ui_print "- Mounting $IMG to /magisk"
  if $isABDevice
  then
    MAGISKLOOP=`$MAGISKBIN/magisk_utils --mountimg $IMG /magisk`
  else
    MAGISKLOOP=`$MAGISKBIN/magisk --mountimg $IMG /magisk`
  fi
fi
is_mounted /magisk || abort "! Magisk image mount failed..."

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
  SHA1=`$MAGISKBIN/magiskboot --sha1 /data/stock_boot.img | tail -n 1`
  STOCKDUMP=/data/stock_boot_${SHA1}.img
  mv /data/stock_boot.img $STOCKDUMP
  $MAGISKBIN/magiskboot --compress $STOCKDUMP
fi

SOURCEDMODE=true
cd $MAGISKBIN

# Source the boot patcher
. $COMMONDIR/boot_patch.sh "$BOOTIMAGE"

if $isABDevice
then
  $MAGISKBIN/magiskpolicy --load /system/sepolicy --save /system/sepolicy --minimal
  mkdir -p /system/magisk 2>/dev/null
  cp -af $COMMONDIR/init.magisk.rc /system/init.magisk.rc
  cp -af $BINDIR/magisk /system/sbin/magisk
  chmod 0755 /system/magisk
  chmod 0750 /system/init.magisk.rc
  chown 0.0 /system/magisk /system/init.magisk.rc
  grep "import /init.magisk.rc" /system/init.rc >/dev/null || sed -i '1,/.*import.*/s/.*import.*/import \/init.magisk.rc\n&/' /system/init.rc
fi

if [ -f stock_boot* ]; then
  rm -f /data/stock_boot* 2>/dev/null
  mv stock_boot* /data
fi

ui_print "- Flashing new boot image"
if [ -L "$BOOTIMAGE" ]; then
  dd if=new-boot.img of="$BOOTIMAGE" bs=4096
else
  cat new-boot.img /dev/zero | dd of="$BOOTIMAGE" bs=4096 >/dev/null 2>&1
fi
rm -f new-boot.img

cd /

if ! $BOOTMODE; then
  if $isABDevice
  then
    $MAGISKBIN/magisk_utils --umountimg /magisk $MAGISKLOOP
  else
    $MAGISKBIN/magisk --umountimg /magisk $MAGISKLOOP
  fi
  rmdir /magisk
  recovery_cleanup
fi

if $isABDevice
then
  mount -o ro /system 2>/dev/null
fi
umount /cache 2>/dev/null
umount /data 2>/dev/null

ui_print "- Done"
exit 0
