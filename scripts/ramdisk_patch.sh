# This file will be sourced by Magisk patch zip, so all variables in the main script should be present
# However, this file may also be called by SuperSU, so we still have to find our own variables in this case

RAMDISK=$1

if [ -z $MAGISK ]; then
  TMPDIR=/dev/tmp
  MAGISKBIN=/data/magisk
  [ ! -e $MAGISKBIN ] && MAGISKBIN=/cache/data_bin
  [ ! -e $MAGISKBIN ] && exit 1
  SYSTEMLIB=/system/lib
  [ -d /system/lib64 ] && SYSTEMLIB=/system/lib64
  KEEPVERITY=true
  KEEPFORCEENCRYPT=true
fi

cd $TMPDIR

# --cpio-add <incpio> <mode> <entry> <infile>
cpio_add() {
  LD_LIBRARY_PATH=$SYSTEMLIB $MAGISKBIN/magiskboot --cpio-add $RAMDISK $1 $2 $3
}

# --cpio-extract <incpio> <entry> <outfile>
cpio_extract() {
  LD_LIBRARY_PATH=$SYSTEMLIB $MAGISKBIN/magiskboot --cpio-extract $RAMDISK $1 $2
}

# --cpio-mkdir <incpio> <mode> <entry>
cpio_mkdir() {
  LD_LIBRARY_PATH=$SYSTEMLIB $MAGISKBIN/magiskboot --cpio-mkdir $RAMDISK $1 $2
}

# The common patches
$KEEPVERITY || LD_LIBRARY_PATH=$SYSTEMLIB $MAGISKBIN/magiskboot --cpio-patch-dmverity $RAMDISK
$KEEPFORCEENCRYPT || LD_LIBRARY_PATH=$SYSTEMLIB $MAGISKBIN/magiskboot --cpio-patch-forceencrypt $RAMDISK

# Add magisk entrypoint
cpio_extract init.rc init.rc
grep "import /init.magisk.rc" init.rc >/dev/null || sed -i '1,/.*import.*/s/.*import.*/import \/init.magisk.rc\n&/' init.rc
sed -i "/selinux.reload_policy/d" init.rc
cpio_add 750 init.rc init.rc

# sepolicy patches
cpio_extract sepolicy sepolicy
LD_LIBRARY_PATH=$SYSTEMLIB $MAGISKBIN/magiskpolicy --load sepolicy --save sepolicy --minimal
cpio_add 644 sepolicy sepolicy

# Add new items

cpio_mkdir 755 magisk

cp -af $MAGISKBIN/init.magisk.rc init.magisk.rc
[ ! -z $SHA1 ] && echo "# STOCKSHA1=$SHA1" >> init.magisk.rc
cpio_add 750 init.magisk.rc init.magisk.rc

cpio_add 750 sbin/magic_mask.sh $MAGISKBIN/magic_mask.sh
