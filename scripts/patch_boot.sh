#!/bin/sh
INSTALL_DIR=/data/local/tmp/magisk
MAGISK_APK=/data/local/tmp/magisk.apk
mkdir -p $INSTALL_DIR
cd $INSTALL_DIR || exit 1
MAGISKBIN=/data/adb/magisk
MAGISKTMP=/sbin
[ -d /sbin ] || MAGISKTMP=/debug_ramdisk
if [ ! -f "$MAGISK_APK" ]; then
    echo "No Magisk apk found"
    exit 1
fi
ABI=$(getprop ro.product.cpu.abi)
for file in busybox magiskpolicy magiskboot magiskinit magisk init-ld; do
    rm -f "$INSTALL_DIR/$file"
    unzip -d $INSTALL_DIR -oj "$MAGISK_APK" "lib/$ABI/lib$file.so"
    mv $INSTALL_DIR/lib$file.so $INSTALL_DIR/$file
    chmod 755 "$INSTALL_DIR/$file"
done
for file in util_functions.sh boot_patch.sh stub.apk; do
    rm -f "$INSTALL_DIR/$file"
    unzip -d $INSTALL_DIR -oj "$MAGISK_APK" "assets/$file"
    chmod 755 "$INSTALL_DIR/$file"
done
rm -rf "${INSTALL_DIR:?}/chromeos"
mkdir -p "$INSTALL_DIR/chromeos"
for file in futility kernel_data_key.vbprivk kernel.keyblock; do
    unzip -d "$INSTALL_DIR/chromeos" -oj "$MAGISK_APK" "assets/chromeos/$file"
    chmod 755 "$INSTALL_DIR/chromeos/$file"
done
rm "$MAGISK_APK"
. ./util_functions.sh
get_flags
api_level_arch_detect
if [ -n "$(getprop ro.boot.vbmeta.device)" ] || [ -n "$(getprop ro.boot.vbmeta.size)" ]; then
    PATCHVBMETAFLAG=false
elif getprop ro.product.ab_ota_partitions | grep -wq vbmeta; then
    PATCHVBMETAFLAG=false
else
    PATCHVBMETAFLAG=true
fi
LEGACYSAR=false
grep ' / ' /proc/mounts | grep -q '/dev/root' && LEGACYSAR=true
SOURCEDMODE=true
. ./boot_patch.sh
mv ./new-boot.img /data/local/tmp/
./magiskboot cleanup
cd /data/local/tmp
rm "$1" "$0" /data/local/tmp/busybox
rm -rf "${INSTALL_DIR:?}"
