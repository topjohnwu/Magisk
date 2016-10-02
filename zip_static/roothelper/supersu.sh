#!/system/bin/sh

mount -o rw,remount rootfs /
mkdir /su 2>/dev/null
mount -o ro,remount rootfs /

chmod 755 /magisk/99supersu/launch_daemonsu.sh
/magisk/99supersu/launch_daemonsu.sh post-fs-data

rm -rf /magisk/.core/bin
ln -s /su/bin /magisk/.core/bin
