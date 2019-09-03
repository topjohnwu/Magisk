#!/usr/bin/env bash
################################################################
#   AVD Magisk Setup
################################################################
#
# This script will setup an environment with minimal Magisk that
# Magisk Manager will be happy to run properly within the official
# emulator bundled with Android Studio (AVD).
#
# ONLY use this script for developing Magisk Manager in the
# emulator. The constructed Magisk environment is not a fully
# functional one as if it is running on an actual device.
# The script only supports non system-as-root images, meaning
# AVD images that are newer than API 25 are NOT supported.
# This script is meant to be used for emulators running obscure
# Android versions; use an actual device for modern Androids.
#
# The script assumes you are using x86 emulator images.
# Build binaries with `./build.py binary` before running this script.
#
################################################################

abort() {
  echo "$@"
  exit 1
}

if [ ! -f /system/build.prop ]; then
  # Running on PC
  cd "`dirname "$0"`/.."
  adb push native/out/x86/busybox native/out/x86/magisk scripts/emulator.sh /data/local/tmp
  adb root || abort 'adb root failed'
  adb shell sh /data/local/tmp/emulator.sh
  exit 0
fi

cd /data/local/tmp
chmod 777 busybox

# Currently only support rootfs based emulators
./busybox mount | ./busybox grep -q rootfs || abort 'Only support non system-as-root emulators'

# Emulator's adb shell should have root
[ `./busybox id -u` -eq 0 ] || abort 'ADB shell should have root access'

# Check whether already setup
[ -f /sbin/magisk ] && abort "Magisk is already setup"

# First setup a good env to work with
rm -rf bin
./busybox mkdir bin
./busybox --install -s bin
OLD_PATH="$PATH"
PATH="/data/local/tmp/bin:$PATH"

# Setup sbin mirror
mount -o rw,remount /
rm -rf /root
mkdir /root
chmod 750 /root
ln /sbin/* /root
mount -o ro,remount /

# Setup sbin overlay
mount -t tmpfs tmpfs /sbin
chmod 755 /sbin
ln -s /root/* /sbin
cp ./magisk /sbin/magisk
chmod 755 /sbin/magisk
ln -s ./magisk /sbin/su
mkdir -p /sbin/.magisk/busybox
cp -af ./busybox /sbin/.magisk/busybox/busybox
/sbin/.magisk/busybox/busybox --install -s /sbin/.magisk/busybox

# Magisk stuffs
mkdir -p /data/adb/modules 2>/dev/null
mkdir /data/adb/post-fs-data.d 2>/dev/null
mkdir /data/adb/services.d 2>/dev/null
magisk --daemon
