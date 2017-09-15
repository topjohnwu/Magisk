#!/sbin/sh
##########################################################################################
#
# Magisk Uninstaller (used in recovery)
# by topjohnwu
#
# This script will load the real uninstaller in a flashable zip
#
##########################################################################################

##########################################################################################
# Preparation
##########################################################################################

BOOTMODE=false
# This path should work in any cases
TMPDIR=/dev/tmp

INSTALLER=$TMPDIR/install
# Default permissions
umask 022

OUTFD=$2
ZIP=$3

if [ ! -f $INSTALLER/util_functions.sh ]; then
  echo "! Unable to extract zip file!"
  exit 1
fi

# Load utility functions
. $INSTALLER/util_functions.sh
get_outfd

##########################################################################################
# Main
##########################################################################################

ui_print "************************"
ui_print "   Magisk Uninstaller   "
ui_print "************************"

is_mounted /data || mount /data
mount_partitions

api_level_arch_detect

ui_print "- Device platform: $ARCH"
CHROMEDIR=$INSTALLER/chromeos
BINDIR=$INSTALLER/$ARCH

##########################################################################################
# Detection all done, start installing
##########################################################################################

MAGISKBIN=/data/magisk

if is_mounted /data; then
  # Save our stock boot image dump before removing it
  [ -f $MAGISKBIN/stock_boot* ] && mv $MAGISKBIN/stock_boot* /data
  # Copy the binaries to /data/magisk, in case they do not exist
  rm -rf $MAGISKBIN 2>/dev/null
  mkdir -p $MAGISKBIN
  cp -af $BINDIR/. $CHROMEDIR $TMPDIR/bin/busybox $INSTALLER/util_functions.sh $MAGISKBIN
  chmod -R 755 $MAGISKBIN
  # Run the acttual uninstallation
  . $INSTALLER/magisk_uninstaller.sh
else
  ui_print "! Data unavailable"
  ui_print "! Placing uninstall script to /cache"
  ui_print "! The device might reboot multiple times"
  cp -af $INSTALLER/magisk_uninstaller.sh /cache/magisk_uninstaller.sh
  umount /system
  ui_print "- Rebooting....."
  sleep 5
  reboot
fi

ui_print "- Done"
exit 0
