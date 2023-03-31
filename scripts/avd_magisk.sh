#!/usr/bin/env bash
#####################################################################
#   AVD Magisk Setup
#####################################################################
#
# Support API level: 21 - 33
#
# With an emulator booted and accessible via ADB, usage:
# ./build.py emulator
#
# This script will stop zygote, simulate the Magisk start up process
# that would've happened before zygote was started, and finally
# restart zygote. This is useful for setting up the emulator for
# developing Magisk, testing modules, and developing root apps using
# the official Android emulator (AVD) instead of a real device.
#
# This only covers the "core" features of Magisk. For testing
# magiskinit, please checkout avd_patch.sh.
#
#####################################################################

mount_sbin() {
  mount -t tmpfs -o 'mode=0755' magisk /sbin
  chcon u:object_r:rootfs:s0 /sbin
}

if [ ! -f /system/build.prop ]; then
  # Running on PC
  echo 'Please run `./build.py emulator` instead of directly executing the script!'
  exit 1
fi

cd /data/local/tmp
chmod 755 busybox

if [ -z "$FIRST_STAGE" ]; then
  export FIRST_STAGE=1
  export ASH_STANDALONE=1
  if [ $(./busybox id -u) -ne 0 ]; then
    # Re-exec script with root
    exec /system/xbin/su 0 ./busybox sh $0
  else
    # Re-exec script with busybox
    exec ./busybox sh $0
  fi
fi

pm install -r $(pwd)/magisk.apk

# Extract files from APK
unzip -oj magisk.apk 'assets/util_functions.sh' 'assets/stub.apk'
. ./util_functions.sh

api_level_arch_detect

unzip -oj magisk.apk "lib/$ABI/*" "lib/$ABI32/libmagisk32.so" -x "lib/$ABI/libbusybox.so"
for file in lib*.so; do
  chmod 755 $file
  mv "$file" "${file:3:${#file}-6}"
done

# Stop zygote (and previous setup if exists)
magisk --stop 2>/dev/null
stop
if [ -d /dev/avd-magisk ]; then
  umount -l /dev/avd-magisk 2>/dev/null
  rm -rf /dev/avd-magisk 2>/dev/null
fi

# Mount /cache if not already mounted
if ! grep -q ' /cache ' /proc/mounts; then
  mount -t tmpfs -o 'mode=0755' tmpfs /cache
fi

MAGISKTMP=/sbin

# Setup bin overlay
if mount | grep -q rootfs; then
  # Legacy rootfs
  mount -o rw,remount /
  rm -rf /root
  mkdir /root
  chmod 750 /root
  ln /sbin/* /root
  mount -o ro,remount /
  mount_sbin
  ln -s /root/* /sbin
elif [ -e /sbin ]; then
  # Legacy SAR
  mount_sbin
  mkdir -p /dev/sysroot
  block=$(mount | grep ' / ' | awk '{ print $1 }')
  [ $block = "/dev/root" ] && block=/dev/block/dm-0
  mount -o ro $block /dev/sysroot
  for file in /dev/sysroot/sbin/*; do
    [ ! -e $file ] && break
    if [ -L $file ]; then
      cp -af $file /sbin
    else
      sfile=/sbin/$(basename $file)
      touch $sfile
      mount -o bind $file $sfile
    fi
  done
  umount -l /dev/sysroot
  rm -rf /dev/sysroot
else
  # Android Q+ without sbin
  MAGISKTMP=/dev/avd-magisk
  mkdir /dev/avd-magisk
  mount -t tmpfs -o 'mode=0755' magisk /dev/avd-magisk
fi

# Magisk stuff
mkdir -p $MAGISKBIN 2>/dev/null
unzip -oj magisk.apk 'assets/*.sh' -d $MAGISKBIN
mkdir $NVBASE/modules 2>/dev/null
mkdir $POSTFSDATAD 2>/dev/null
mkdir $SERVICED 2>/dev/null

for file in magisk32 magisk64 magiskpolicy stub.apk; do
  chmod 755 ./$file
  cp -af ./$file $MAGISKTMP/$file
  cp -af ./$file $MAGISKBIN/$file
done
cp -af ./magiskboot $MAGISKBIN/magiskboot
cp -af ./magiskinit $MAGISKBIN/magiskinit
cp -af ./busybox $MAGISKBIN/busybox

if $IS64BIT; then
  ln -s ./magisk64 $MAGISKTMP/magisk
else
  ln -s ./magisk32 $MAGISKTMP/magisk
fi
ln -s ./magisk $MAGISKTMP/su
ln -s ./magisk $MAGISKTMP/resetprop
ln -s ./magisk $MAGISKTMP/magiskhide
ln -s ./magiskpolicy $MAGISKTMP/supolicy

mkdir -p $MAGISKTMP/.magisk/mirror
mkdir $MAGISKTMP/.magisk/block
mkdir $MAGISKTMP/.magisk/worker
touch $MAGISKTMP/.magisk/config

export MAGISKTMP
MAKEDEV=1 $MAGISKTMP/magisk --preinit-device 2>&1

RULESCMD=""
for r in $MAGISKTMP/.magisk/preinit/*/sepolicy.rule; do
  [ -f "$r" ] || continue
  RULESCMD="$RULESCMD --apply $r"
done

# SELinux stuffs
if [ -d /sys/fs/selinux ]; then
  if [ -f /vendor/etc/selinux/precompiled_sepolicy ]; then
    ./magiskpolicy --load /vendor/etc/selinux/precompiled_sepolicy --live --magisk $RULESCMD 2>&1
  elif [ -f /sepolicy ]; then
    ./magiskpolicy --load /sepolicy --live --magisk $RULESCMD 2>&1
  else
    ./magiskpolicy --live --magisk $RULESCMD 2>&1
  fi
fi

# Boot up
$MAGISKTMP/magisk --post-fs-data
start
$MAGISKTMP/magisk --service
