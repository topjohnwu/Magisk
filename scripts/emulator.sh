#!/usr/bin/env bash
#####################################################################
#   AVD Magisk Setup
#####################################################################
#
# This script will setup an environment with minimal Magisk that
# the Magisk app will be happy to run properly within the official
# emulator bundled with Android Studio (AVD).
#
# ONLY use this script for developing the Magisk app or root apps
# in the emulator. The constructed Magisk environment is not a
# fully functional one as if it is running on an actual device.
#
# The script assumes you are using x64 emulator images.
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
  cd "$(dirname "$0")/.."
  adb push native/out/x86/busybox native/out/x86/magiskinit \
  native/out/x86_64/magisk scripts/emulator.sh /data/local/tmp
  adb shell sh /data/local/tmp/emulator.sh
  exit 0
fi

cd /data/local/tmp
chmod 777 busybox
chmod 777 magiskinit
chmod 777 magisk

if [ -z "$FIRST_STAGE" ]; then
  export FIRST_STAGE=1
  export ASH_STANDALONE=1
  if [ `./busybox id -u` -ne 0 ]; then
    # Re-exec script with root
    exec /system/xbin/su 0 ./busybox sh $0
  else
    # Re-exec script with busybox
    exec ./busybox sh $0
  fi
fi

# Remove previous setup if exist
pgrep magiskd >/dev/null && pkill -9 magiskd
[ -f /sbin/magisk ] && umount -l /sbin
[ -f /system/bin/magisk ] && umount -l /system/bin

# SELinux stuffs
SELINUX=false
[ -e /sys/fs/selinux ] && SELINUX=true
if $SELINUX; then
  ln -sf ./magiskinit magiskpolicy
  ./magiskpolicy --live --magisk
fi

BINDIR=/sbin

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
  if ! grep -q '/sbin/.magisk/mirror/system_root' /proc/mounts; then
    mkdir -p /sbin/.magisk/mirror/system_root
    block=$(mount | grep ' / ' | awk '{ print $1 }')
    [ $block = "/dev/root" ] && block=/dev/block/dm-0
    mount -o ro $block /sbin/.magisk/mirror/system_root
  fi
  for file in /sbin/.magisk/mirror/system_root/sbin/*; do
    [ ! -e $file ] && break
    if [ -L $file ]; then
      cp -af $file /sbin
    else
      sfile=/sbin/$(basename $file)
      touch $sfile
      mount -o bind $file $sfile
    fi
  done
else
  # Android Q+ without sbin, use overlayfs
  BINDIR=/system/bin
  rm -rf /dev/magisk
  mkdir -p /dev/magisk/upper
  mkdir /dev/magisk/work
  ./magisk --clone-attr /system/bin /dev/magisk/upper
  mount -t overlay overlay -olowerdir=/system/bin,upperdir=/dev/magisk/upper,workdir=/dev/magisk/work /system/bin
fi

# Magisk stuffs
cp -af ./magisk $BINDIR/magisk
chmod 755 $BINDIR/magisk
ln -s ./magisk $BINDIR/su
ln -s ./magisk $BINDIR/resetprop
ln -s ./magisk $BINDIR/magiskhide
mkdir -p /data/adb/modules 2>/dev/null
mkdir /data/adb/post-fs-data.d 2>/dev/null
mkdir /data/adb/services.d 2>/dev/null
$BINDIR/magisk --daemon
