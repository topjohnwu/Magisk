#####################################################################
#   AVD MagiskInit Setup
#####################################################################
#
# Support API level: 23 - 36
#
# With an emulator booted and accessible via ADB, usage:
# ./build.py patch --avd path/to/booted/avd-image/ramdisk.img
#
# The purpose of this script is to patch AVD ramdisk.img and do a
# full integration test of magiskinit under several circumstances.
# After patching ramdisk.img, close the emulator, then select
# "Cold Boot Now" in AVD Manager to force a full reboot.
#
#####################################################################
# AVD Init Configurations:
#
# rootfs w/o early mount: API 23 - 25
# rootfs with early mount: API 26 - 27
# Legacy system-as-root: API 28
# 2 stage init: API 29 - 35
#####################################################################

if [ ! -f /system/build.prop ]; then
  # Running on PC
  echo 'Please run `./build.py patch --avd` instead of directly executing the script!'
  exit 1
fi

cd /data/local/tmp
chmod 755 busybox magisk*

if [ -z "$FIRST_STAGE" ]; then
  export FIRST_STAGE=1
  export ASH_STANDALONE=1
  # Re-exec script with busybox
  exec ./busybox sh $0 "$@"
fi

TARGET_FILE="$1"
OUTPUT_FILE="$1.magisk"

./magiskboot decompress "$TARGET_FILE" ramdisk.cpio
cp ramdisk.cpio ramdisk.cpio.orig

export KEEPVERITY=true
export KEEPFORCEENCRYPT=true

echo "KEEPVERITY=$KEEPVERITY" > config
echo "KEEPFORCEENCRYPT=$KEEPFORCEENCRYPT" >> config
echo "PREINITDEVICE=$(./magisk --preinit-device)" >> config
# For API 28, we also manually disable SystemAsRoot
# Explicitly override skip_initramfs by setting RECOVERYMODE=true
API=$(getprop ro.build.version.sdk)
[ "$API" = "28" ] && echo 'RECOVERYMODE=true' >> config
cat config

./magiskboot compress=xz magisk magisk.xz
./magiskboot compress=xz stub.apk stub.xz
./magiskboot compress=xz init-ld init-ld.xz

./magiskboot cpio ramdisk.cpio \
"add 0750 init magiskinit" \
"mkdir 0750 overlay.d" \
"mkdir 0750 overlay.d/sbin" \
"add 0644 overlay.d/sbin/magisk.xz magisk.xz" \
"add 0644 overlay.d/sbin/stub.xz stub.xz" \
"add 0644 overlay.d/sbin/init-ld.xz init-ld.xz" \
"patch" \
"backup ramdisk.cpio.orig" \
"mkdir 000 .backup" \
"add 000 .backup/.magisk config"

rm -f ramdisk.cpio.orig config *.xz
./magiskboot compress=gzip ramdisk.cpio "$OUTPUT_FILE"
