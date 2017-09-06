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
CHROMEDIR=$INSTALLER/chromeos
COREDIR=/magisk/.core

# Default permissions
umask 022

OUTFD=$2
ZIP=$3

if [ ! -d "$COMMONDIR" ]; then
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

ui_print "- Mounting /system, /vendor, /cache, /data"
mount -o ro /system 2>/dev/null
mount -o ro /vendor 2>/dev/null
mount /cache 2>/dev/null
mount /data 2>/dev/null

[ -f /system/build.prop ] || abort "! /system could not be mounted!"

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

is_mounted /data && MAGISKBIN=/data/magisk || MAGISKBIN=/cache/data_bin

if $BOOTMODE; then
  # Cleanup binary mirrors
  umount -l /dev/magisk/mirror/bin 2>/dev/null
  rm -rf /dev/magisk/mirror/bin 2>/dev/null
fi

# Copy required files
rm -rf $MAGISKBIN 2>/dev/null
mkdir -p $MAGISKBIN
cp -af $BINDIR/. $COMMONDIR/. $CHROMEDIR $TMPDIR/bin/busybox $MAGISKBIN
chmod -R 755 $MAGISKBIN

# addon.d
if [ -d /system/addon.d ]; then
  ui_print "- Adding addon.d survival script"
  mount -o rw,remount /system
  cp -af $INSTALLER/addon.d/99-magisk.sh /system/addon.d/99-magisk.sh
  chmod 755 /system/addon.d/99-magisk.sh
fi

$BOOTMODE && boot_actions || recovery_actions

##########################################################################################
# Boot patching
##########################################################################################

find_boot_image
[ -z $BOOTIMAGE ] && abort "! Unable to detect boot image"
ui_print "- Found Boot Image: $BOOTIMAGE"

SOURCEDMODE=true
cd $MAGISKBIN

# Source the boot patcher
. $COMMONDIR/boot_patch.sh "$BOOTIMAGE"

[ -f stock_boot* ] && rm -f /data/stock_boot* 2>/dev/null

ui_print "- Flashing new boot image"
if [ -L "$BOOTIMAGE" ]; then
  dd if=new-boot.img of="$BOOTIMAGE" bs=4096
else
  cat new-boot.img /dev/zero | dd of="$BOOTIMAGE" bs=4096 >/dev/null 2>&1
fi
rm -f new-boot.img

cd /

# Cleanups
$BOOTMODE || recovery_cleanup
rm -rf $TMPDIR

ui_print "- Done"
exit 0
