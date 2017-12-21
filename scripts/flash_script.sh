#MAGISK
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
APK=$COMMONDIR/magisk.apk
CHROMEDIR=$INSTALLER/chromeos
COREDIR=/magisk/.core

# Default permissions
umask 022

OUTFD=$2
ZIP=$3

if [ ! -f $COMMONDIR/util_functions.sh ]; then
  echo "! Unable to extract zip file!"
  exit 1
fi

# Load utility fuctions
. $COMMONDIR/util_functions.sh

get_outfd

##########################################################################################
# Detection
##########################################################################################

ui_print "************************"
ui_print "* Magisk v$MAGISK_VER Installer"
ui_print "************************"

is_mounted /data || mount /data || is_mounted /cache || mount /cache || abort "! Unable to mount partitions"
mount_partitions

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

##########################################################################################
# Environment
##########################################################################################

ui_print "- Constructing environment"

if is_mounted /data; then
  MAGISKBIN=/data/adb/magisk
  mkdir -p /data/adb 2>/dev/null
  chmod 700 /data/adb 2>/dev/null

  # Some legacy migration
  mv /data/magisk/stock_boot* /data 2>/dev/null
  [ -L /data/magisk.img ] || mv /data/magisk.img /data/adb/magisk.img
else
  MAGISKBIN=/cache/data_bin
fi

# Copy required files
rm -rf $MAGISKBIN/* 2>/dev/null
mkdir -p $MAGISKBIN 2>/dev/null
cp -af $BINDIR/. $COMMONDIR/. $CHROMEDIR $TMPDIR/bin/busybox $MAGISKBIN
chmod -R 755 $MAGISKBIN

# addon.d
if [ -d /system/addon.d ]; then
  ui_print "- Adding addon.d survival script"
  mount -o rw,remount /system
  cp -af $INSTALLER/addon.d/99-magisk.sh /system/addon.d/99-magisk.sh
  chmod 755 /system/addon.d/99-magisk.sh
fi

$BOOTMODE || recovery_actions

##########################################################################################
# Boot patching
##########################################################################################

[ -z $BOOTIMAGE ] && find_boot_image
[ -z $BOOTIMAGE ] && abort "! Unable to detect boot image"
ui_print "- Found boot image: $BOOTIMAGE"

find_dtbo_image
if [ ! -z $DTBOIMAGE ]; then
  ui_print "- Found dtbo image: $DTBOIMAGE"
  # Disable dtbo patch by default
  [ -z $KEEPVERITY ] && KEEPVERITY=true
fi

eval $BOOTSIGNER -verify < $BOOTIMAGE && BOOTSIGNED=true
$BOOTSIGNED && ui_print "- Signed boot image detected"

SOURCEDMODE=true
cd $MAGISKBIN

# Source the boot patcher
. $COMMONDIR/boot_patch.sh "$BOOTIMAGE"

flash_boot_image new-boot.img "$BOOTIMAGE"
rm -f new-boot.img

if [ -f stock_boot* ]; then
  rm -f /data/stock_boot* 2>/dev/null
  is_mounted /data && mv stock_boot* /data
fi

$KEEPVERITY || patch_dtbo_image

if [ -f stock_dtbo* ]; then
  rm -f /data/stock_dtbo* 2>/dev/null
  is_mounted /data && mv stock_dtbo* /data
fi

cd /
# Cleanups
$BOOTMODE || recovery_cleanup
rm -rf $TMPDIR

ui_print "- Done"
exit 0
