#!/system/bin/sh
##########################################################################################
#
# Magisk System Patcher
# by topjohnwu
#
# Usage: system_patch.sh
#
# The following flags can be set in environment variables:
# FORCESYSTEMMODE
#
# This script should be placed in a directory with the following files:
#
# File name          Type      Description
#
# system_patch.sh      script    A script to patch system.
#                  (this file) The script will use binaries and files in its same directory
#                              to complete the patching process
# util_functions.sh  script    A script which hosts all functions requires for this script
#                              to work properly
# magiskinit         binary    The binary to replace /init, which has the magisk binary embedded
#
##########################################################################################
##########################################################################################
# Patching System
##########################################################################################

ui_print "- Patching System"

mount -o rw,remount /system

# Add files to system
cp $MAGISKBIN/magiskinit /system/xbin/magiskinit
./system/xbin/magiskinit -x magisk /system/xbin/magisk
ln -nsf /system/xbin/magiskinit /system/xbin/magiskpolicy
ln -nsf /system/xbin/magisk /system/xbin/imgtool
ln -nsf /system/xbin/magisk /system/xbin/magiskhide
ln -nsf /system/xbin/magisk /system/xbin/resetprop
ln -nsf /system/xbin/magisk /system/xbin/su

if [ ! -f "/system/.Magisk" ]; then
  touch /system/.Magisk
fi

if [ -f "/system/bin/install-recovery.sh" ]; then
  if [ ! -f "/system/bin/install-recovery_original.sh" ]; then
    mv /system/bin/install-recovery.sh /system/bin/install-recovery_original.sh
    chcon -h u:object_r:system_file:s0 /system/bin/install-recovery_original.sh
  fi
fi
if [ -f "/system/etc/install-recovery.sh" ]; then
  if [ ! -f "/system/etc/install-recovery_original.sh" ]; then
    mv /system/etc/install-recovery.sh /system/etc/install-recovery_original.sh
    chcon -h u:object_r:system_file:s0 /system/etc/install-recovery_original.sh
  fi
fi

cp -fp $MAGISKBIN/install-recovery.sh /system/etc/install-recovery.sh
chcon -h u:object_r:system_file:s0 /system/etc/install-recovery.sh
ln -nsf /system/etc/install-recovery.sh /system/bin/install-recovery.sh

# Reset any error code
true
