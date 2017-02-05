#!/system/bin/sh

RAMDISK=$1
BINDIR=/data/magisk
[ ! -e $BINDIR ] && BINDIR=/cache/data_bin
[ ! -e $BINDIR ] && exit
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

file_contain() {
  grep "$1" "$2" >/dev/null 2>&1
  return $?
}

rm -rf /tmp/magisk/ramdisk 2>/dev/null
mkdir -p /tmp/magisk/ramdisk
cd /tmp/magisk/ramdisk

cat $RAMDISK | cpio -i

# Cleanup SuperSU backups
cpio_rm -r .subackup

# Add magisk entrypoint
for RC in init*.rc; do
  if file_contain "import /init.environ.rc" $RC && ! file_contain "import /init.magisk.rc" $RC; then
    sed -i "/import \/init\.environ\.rc/iimport /init.magisk.rc" $RC
    cpio_add $RC 750
  fi
  if file_contain "selinux.reload_policy" $RC; then
    sed -i "/selinux.reload_policy/d" $RC
    cpio_add $RC 750
  fi
done

# sepolicy patches
LD_LIBRARY_PATH=$SYSTEMLIB $BINDIR/sepolicy-inject --load sepolicy --save sepolicy --minimal
cpio_add sepolicy 644

# Add new items
cp -af $BINDIR/init.magisk.rc init.magisk.rc
cp -af $BINDIR/magic_mask.sh sbin/magic_mask.sh

cpio_mkdir magisk 755
cpio_add init.magisk.rc 750
cpio_add sbin/magic_mask.sh 750
