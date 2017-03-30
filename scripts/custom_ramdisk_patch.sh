#!/system/bin/sh

RAMDISK=$1

TMPDIR=/dev/tmp
MAGISKBIN=/data/magisk
[ ! -e $MAGISKBIN ] && MAGISKBIN=/cache/data_bin
[ ! -e $MAGISKBIN ] && exit 1
SYSTEMLIB=/system/lib
[ -d /system/lib64 ] && SYSTEMLIB=/system/lib64

mkdir -p $TMPDIR 2>/dev/null
cd $TMPDIR

cpio_add() {
  LD_LIBRARY_PATH=$SYSTEMLIB /su/bin/sukernel --cpio-add $RAMDISK $RAMDISK $1 $2 $3
}

cpio_extract() {
  LD_LIBRARY_PATH=$SYSTEMLIB /su/bin/sukernel --cpio-extract $RAMDISK $1 $2
}

cpio_mkdir() {
  LD_LIBRARY_PATH=$SYSTEMLIB /su/bin/sukernel --cpio-mkdir $RAMDISK $RAMDISK $1 $2
}

# Recursive
cpio_rm() {
  if [ "$1" = "-r" ]; then
    LD_LIBRARY_PATH=$SYSTEMLIB /su/bin/sukernel --cpio-ls $RAMDISK | grep "^$2/" | while read i ; do
      LD_LIBRARY_PATH=$SYSTEMLIB /su/bin/sukernel --cpio-rm $RAMDISK $RAMDISK $i
    done
    LD_LIBRARY_PATH=$SYSTEMLIB /su/bin/sukernel --cpio-rmdir $RAMDISK $RAMDISK $2
  else
    LD_LIBRARY_PATH=$SYSTEMLIB /su/bin/sukernel --cpio-rm $RAMDISK $RAMDISK $1
  fi
}

# Cleanup SuperSU backups
cpio_rm -r .subackup

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
cpio_add 750 init.magisk.rc $MAGISKBIN/init.magisk.rc
cpio_add 750 sbin/magic_mask.sh $MAGISKBIN/magic_mask.sh

