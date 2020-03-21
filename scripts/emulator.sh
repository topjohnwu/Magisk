#!/usr/bin/env bash
#####################################################################
#   AVD Magisk Setup
#####################################################################
#
# This script will setup an environment with minimal Magisk that
# Magisk Manager will be happy to run properly within the official
# emulator bundled with Android Studio (AVD).
#
# ONLY use this script for developing Magisk Manager or root apps
# in the emulator. The constructed Magisk environment is not a
# fully functional one as if it is running on an actual device.
#
# The script assumes you are using x86/x64 emulator images.
# Build binaries with `./build.py binary` before running this script.
#
#####################################################################

abort() {
  echo "$@"
  exit 1
}

mount_sbin() {
  mount -t tmpfs -o 'mode=0755' tmpfs /sbin
  $SELINUX && chcon u:object_r:rootfs:s0 /sbin
}

if [ ! -f /system/build.prop ]; then
  # Running on PC
  cd "`dirname "$0"`/.."
  adb push native/out/x86/busybox scripts/emulator.sh /data/local/tmp
  emu_arch=`adb shell uname -m`
  if [ "$emu_arch" = "x86_64" ]; then
    adb push native/out/x86/magiskinit64 /data/local/tmp/magiskinit
  else
    adb push native/out/x86/magiskinit /data/local/tmp
  fi
  adb shell sh /data/local/tmp/emulator.sh
  exit 0
fi

cd /data/local/tmp
chmod 777 busybox
chmod 777 magiskinit

if [ `./busybox id -u` -ne 0 ]; then
  # Re-exec script with root
  exec /system/xbin/su 0 ./busybox sh -o standalone $0
fi

# Remove previous setup if exist
pgrep magiskd >/dev/null && pkill -9 magiskd
[ -f /sbin/magisk ] && umount -l /sbin

# SELinux stuffs
[ -e /sys/fs/selinux ] && SELINUX=true || SELINUX=false
if $SELINUX; then
  ln -sf ./magiskinit magiskpolicy
  ./magiskpolicy --live --magisk 'allow magisk * * *'
fi

# Setup sbin overlay
if mount | grep -q rootfs; then
  mount -o rw,remount /
  rm -rf /root
  mkdir /root
  chmod 750 /root
  ln /sbin/* /root
  mount -o ro,remount /
  mount_sbin
  ln -s /root/* /sbin
else
  mount_sbin
  mkdir -p /sbin/.magisk/mirror/system_root
  block=`mount | grep ' / ' | awk '{ print $1 }'`
  [ $block = "/dev/root" ] && block=/dev/block/dm-0
  mount -o ro $block /sbin/.magisk/mirror/system_root
  for file in /sbin/.magisk/mirror/system_root/sbin/*; do
    [ ! -e $file ] && break
    if [ -L $file ]; then
      cp -af $file /sbin
    else
      sfile=/sbin/`basename $file`
      touch $sfile
      mount -o bind $file $sfile
    fi
  done
fi

# Magisk stuffs
./magiskinit -x magisk /sbin/magisk
chmod 755 /sbin/magisk
ln -s ./magisk /sbin/su
ln -s ./magisk /sbin/resetprop
ln -s ./magisk /sbin/magiskhide
mkdir -p /sbin/.magisk/busybox
cp -af ./busybox /sbin/.magisk/busybox/busybox
/sbin/.magisk/busybox/busybox --install -s /sbin/.magisk/busybox
mkdir -p /data/adb/modules 2>/dev/null
mkdir /data/adb/post-fs-data.d 2>/dev/null
mkdir /data/adb/services.d 2>/dev/null
/sbin/magisk --daemon
