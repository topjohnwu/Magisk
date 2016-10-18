#!/system/bin/sh

RAMDISK=$1
BINDIR=/data/magisk

cpio_add() {
  /su/bin/sukernel --cpio-add $RAMDISK $RAMDISK $2 $1 $1
}

cpio_extract() {
  /su/bin/sukernel --cpio-extract $RAMDISK $1 $1
}

cpio_mkdir() {
  /su/bin/sukernel --cpio-mkdir $RAMDISK $RAMDISK $2 $1
}

rm -rf /tmp/magisk/ramdisk 2>/dev/null
mkdir -p /tmp/magisk/ramdisk
cd /tmp/magisk/ramdisk

cat $RAMDISK | cpio -i

# Patch ramdisk
echo "- Patching ramdisk"

# Add magisk entrypoint
for INIT in init*.rc; do
  if [ $(grep -c "import /init.environ.rc" $INIT) -ne "0" ] && [ $(grep -c "import /init.magisk.rc" $INIT) -eq "0" ]; then
    sed -i "/import \/init\.environ\.rc/iimport /init.magisk.rc" $INIT
    cpio_add $INIT 750
    break
  fi
done

# Add magisk PATH
if [ $(grep -c "/magisk/.core/busybox" init.environ.rc) -eq "0" ]; then
  sed -i "/export PATH/ s/\/system\/xbin/\/system\/xbin:\/magisk\/.core\/busybox/g" init.environ.rc
  cpio_add init.environ.rc 750
fi

# sepolicy patches
$BINDIR/sepolicy-inject --magisk -P sepolicy
cpio_add sepolicy 644

# Add new items
mkdir -p magisk 2>/dev/null
cp -af $BINDIR/init.magisk.rc init.magisk.rc
cp -af $BINDIR/magic_mask.sh sbin/magic_mask.sh

cpio_mkdir magisk 755
cpio_add init.magisk.rc 750
cpio_add sbin/magic_mask.sh 750
