#!/system/bin/sh

RAMDISK=$1
BINDIR=$2
[ -z $BINDIR ] && BINDIR=/data/magisk
SYSTEMLIB=/system/lib
[ -d /system/lib64 ] && SYSTEMLIB=/system/lib64

cpio_add() {
  LD_LIBRARY_PATH=$SYSTEMLIB /su/bin/sukernel --cpio-add $RAMDISK $RAMDISK $2 $1 $1
}

cpio_extract() {
  LD_LIBRARY_PATH=$SYSTEMLIB /su/bin/sukernel --cpio-extract $RAMDISK $1 $1
}

cpio_mkdir() {
  LD_LIBRARY_PATH=$SYSTEMLIB /su/bin/sukernel --cpio-mkdir $RAMDISK $RAMDISK $2 $1
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

rm -rf /tmp/magisk/ramdisk 2>/dev/null
mkdir -p /tmp/magisk/ramdisk
cd /tmp/magisk/ramdisk

cat $RAMDISK | cpio -i

# Patch ramdisk
echo "- Patching ramdisk"

# Cleanup SuperSU backups
cpio_rm -r .subackup

# Add magisk entrypoint
for INIT in init*.rc; do
  if [ `grep -c "import /init.environ.rc" $INIT` -ne "0" ] && [ `grep -c "import /init.magisk.rc" $INIT` -eq "0" ]; then
    sed -i "/import \/init\.environ\.rc/iimport /init.magisk.rc" $INIT
    cpio_add $INIT 750
    break
  fi
done

# sepolicy patches
LD_LIBRARY_PATH=$SYSTEMLIB $BINDIR/sepolicy-inject --magisk -P sepolicy
cpio_add sepolicy 644

# Add new items
cp -af $BINDIR/init.magisk.rc init.magisk.rc
cp -af $BINDIR/magic_mask.sh sbin/magic_mask.sh

cpio_mkdir magisk 755
cpio_add init.magisk.rc 750
cpio_add sbin/magic_mask.sh 750
