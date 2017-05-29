#!/sbin/sh
##########################################################################################
#
# Magisk Boot Image Patcher
# by topjohnwu
# 
# This zip will patch your boot image with Magisk support
#
##########################################################################################

MAGISK=true

# Detect whether in boot mode
ps | grep zygote | grep -v grep >/dev/null && BOOTMODE=true || BOOTMODE=false

# This path should work in any cases
TMPDIR=/dev/tmp

INSTALLER=$TMPDIR/magisk
COMMONDIR=$INSTALLER/common
BOOTTMP=$TMPDIR/boottmp
COREDIR=/magisk/.core
CHROMEDIR=$INSTALLER/chromeos
SYSTEM=/system

# Default permissions
umask 022

ABDeviceCheck=$(cat /proc/cmdline | grep slot_suffix | wc -l)
if [ $ABDeviceCheck -gt 0 ];
then
  isABDevice=true
  SLOT=$(for i in `cat /proc/cmdline`; do echo $i | grep slot_suffix | awk -F "=" '{print $2}';done)
  SYSTEM=/system/system
else
  isABDevice=false
fi

##########################################################################################
# Flashable update-binary preparation
##########################################################################################

OUTFD=$2
ZIP=$3

readlink /proc/$$/fd/$OUTFD 2>/dev/null | grep /tmp >/dev/null
if [ "$?" -eq "0" ]; then
  OUTFD=0

  for FD in `ls /proc/$$/fd`; do
    readlink /proc/$$/fd/$FD 2>/dev/null | grep pipe >/dev/null
    if [ "$?" -eq "0" ]; then
      ps | grep " 3 $FD " | grep -v grep >/dev/null
      if [ "$?" -eq "0" ]; then
        OUTFD=$FD
        break
      fi
    fi
  done
fi

mkdir -p $INSTALLER
cd $INSTALLER
unzip -o "$ZIP"

##########################################################################################
# Functions
##########################################################################################

ui_print() {
  if $BOOTMODE; then
    echo "$1"
  else 
    echo -n -e "ui_print $1\n" >> /proc/self/fd/$OUTFD
    echo -n -e "ui_print\n" >> /proc/self/fd/$OUTFD
  fi
}

getvar() {
  local VARNAME=$1
  local VALUE=$(eval echo \$"$VARNAME");
  for FILE in /dev/.magisk /data/.magisk /cache/.magisk $SYSTEM/.magisk; do
    if [ -z "$VALUE" ]; then
      LINE=$(cat $FILE 2>/dev/null | grep "$VARNAME=")
      if [ ! -z "$LINE" ]; then
        VALUE=${LINE#*=}
      fi
    fi
  done
  eval $VARNAME=\$VALUE
}

find_boot_image() {
srch_str="kern-a KERN-A android_boot ANDROID_BOOT kernel KERNEL boot BOOT lnx LNX"
  if $isABDevice
  then
    srch_str="boot_$SLOT BOOT_$SLOT kern-$SLOT KERN-$SLOT"+$srch_str
  fi
  if [ -z "$BOOTIMAGE" ]; then
    for PARTITION in $srch_str; do
      BOOTIMAGE=`readlink /dev/block/by-name/$PARTITION || readlink /dev/block/platform/*/by-name/$PARTITION || readlink /dev/block/platform/*/*/by-name/$PARTITION`
      if [ ! -z "$BOOTIMAGE" ]; then break; fi
    done
  fi
  if [ -z "$BOOTIMAGE" ]; then
    FSTAB="/etc/recovery.fstab"
    [ ! -f "$FSTAB" ] && FSTAB="/etc/recovery.fstab.bak"
    [ -f "$FSTAB" ] && BOOTIMAGE=`grep -E '\b/boot\b' "$FSTAB" | grep -oE '/dev/[a-zA-Z0-9_./-]*'`
  fi
}

is_mounted() {
  if [ ! -z "$2" ]; then
    cat /proc/mounts | grep $1 | grep $2, >/dev/null
  else
    cat /proc/mounts | grep $1 >/dev/null
  fi
  return $?
}

mount_image() {
  if [ ! -d "$2" ]; then
    mount -o rw,remount rootfs /
    mkdir -p $2 2>/dev/null
    ($BOOTMODE) && mount -o ro,remount rootfs /
    [ ! -d "$2" ] && return 1
  fi
  if (! is_mounted $2); then
    LOOPDEVICE=
    for LOOP in 0 1 2 3 4 5 6 7; do
      if (! is_mounted $2); then
        LOOPDEVICE=/dev/block/loop$LOOP
        if [ ! -f "$LOOPDEVICE" ]; then
          mknod $LOOPDEVICE b 7 $LOOP 2>/dev/null
        fi
        losetup $LOOPDEVICE $1
        if [ "$?" -eq "0" ]; then
          mount -t ext4 -o loop $LOOPDEVICE $2
          if (! is_mounted $2); then
            $SYSTEM/bin/toolbox mount -t ext4 -o loop $LOOPDEVICE $2
          fi
          if (! is_mounted $2); then
            $SYSTEM/bin/toybox mount -t ext4 -o loop $LOOPDEVICE $2
          fi
        fi
        if (is_mounted $2); then
          ui_print "- Mounting $1 to $2"
          break;
        fi
      fi
    done
  fi
}

grep_prop() {
  REGEX="s/^$1=//p"
  shift
  FILES=$@
  if [ -z "$FILES" ]; then
    FILES="$SYSTEM/build.prop"
  fi
  cat $FILES 2>/dev/null | sed -n "$REGEX" | head -n 1
}

remove_system_su() {
  if [ -f $SYSTEM/bin/su -o -f $SYSTEM/xbin/su ] && [ ! -f /su/bin/su ]; then
    ui_print "! System installed root detected, mount rw :("
    mount -o rw,remount /system
    # SuperSU
    if [ -e $SYSTEM/bin/.ext/.su ]; then
      mv -f $SYSTEM/bin/app_process32_original $SYSTEM/bin/app_process32 2>/dev/null
      mv -f $SYSTEM/bin/app_process64_original $SYSTEM/bin/app_process64 2>/dev/null
      mv -f $SYSTEM/bin/install-recovery_original.sh $SYSTEM/bin/install-recovery.sh 2>/dev/null
      cd $SYSTEM/bin
      if [ -e app_process64 ]; then
        ln -sf app_process64 app_process
      else
        ln -sf app_process32 app_process
      fi
    fi
    rm -rf $SYSTEM/.pin $SYSTEM/bin/.ext $SYSTEM/etc/.installed_su_daemon $SYSTEM/etc/.has_su_daemon \
    $SYSTEM/xbin/daemonsu $SYSTEM/xbin/su $SYSTEM/xbin/sugote $SYSTEM/xbin/sugote-mksh $SYSTEM/xbin/supolicy \
    $SYSTEM/bin/app_process_init $SYSTEM/bin/su /cache/su $SYSTEM/lib/libsupol.so $SYSTEM/lib64/libsupol.so \
    $SYSTEM/su.d $SYSTEM/etc/install-recovery.sh $SYSTEM/etc/init.d/99SuperSUDaemon /cache/install-recovery.sh \
    $SYSTEM/.supersu /cache/.supersu /data/.supersu \
    $SYSTEM/app/Superuser.apk $SYSTEM/app/SuperSU /cache/Superuser.apk  2>/dev/null
  fi
}

# --cpio-add <incpio> <mode> <entry> <infile>
cpio_add() {
  LD_LIBRARY_PATH=$SYSTEMLIB $BINDIR/magiskboot --cpio-add ramdisk.cpio $1 $2 $3
}

# --cpio-extract <incpio> <entry> <outfile>
cpio_extract() {
  LD_LIBRARY_PATH=$SYSTEMLIB $BINDIR/magiskboot --cpio-extract ramdisk.cpio $1 $2
}

# --cpio-mkdir <incpio> <mode> <entry>
cpio_mkdir() {
  LD_LIBRARY_PATH=$SYSTEMLIB $BINDIR/magiskboot --cpio-mkdir ramdisk.cpio $1 $2
}

##########################################################################################
# Detection
##########################################################################################

ui_print "************************"
ui_print "* MAGISK_VERSION_STUB"
ui_print "************************"

if [ ! -d "$COMMONDIR" ]; then
  ui_print "! Failed: Unable to extract zip file!"
  exit 1
fi

ui_print "- Mounting /system(ro), /cache, /data"
mount -o ro /system 2>/dev/null
if $isABDevice
then
  mount -o rw,remount /system 2>/dev/null
fi
mount /cache 2>/dev/null
mount /data 2>/dev/null

if [ ! -f "$SYSTEM/build.prop" ]; then
  ui_print "! Failed: /system could not be mounted!"
  exit 1
fi

# read override variables
getvar KEEPVERITY
getvar KEEPFORCEENCRYPT
getvar BOOTIMAGE

[ -z $KEEPVERITY ] && KEEPVERITY=false
[ -z $KEEPFORCEENCRYPT ] && KEEPFORCEENCRYPT=false

# Check if system root is installed and remove
remove_system_su

API=`grep_prop ro.build.version.sdk`
ABI=`grep_prop ro.product.cpu.abi | cut -c-3`
ABI2=`grep_prop ro.product.cpu.abi2 | cut -c-3`
ABILONG=`grep_prop ro.product.cpu.abi`

ARCH=arm
IS64BIT=false
if [ "$ABI" = "x86" ]; then ARCH=x86; fi;
if [ "$ABI2" = "x86" ]; then ARCH=x86; fi;
if [ "$ABILONG" = "arm64-v8a" ]; then ARCH=arm64; IS64BIT=true; fi;
if [ "$ABILONG" = "x86_64" ]; then ARCH=x64; IS64BIT=true; fi;


if [ "$API" -lt "21" ]; then
  ui_print "! Magisk is only for Lollipop 5.0+ (SDK 21+)"
  exit 1
fi

ui_print "- Device platform: $ARCH"

BINDIR=$INSTALLER/$ARCH
chmod -R 755 $CHROMEDIR $BINDIR

$IS64BIT && SYSTEMLIB=$SYSTEM/lib64 || SYSTEMLIB=$SYSTEM/lib

find_boot_image
if [ -z $BOOTIMAGE ]; then
  ui_print "! Unable to detect boot image"
  exit 1
fi

##########################################################################################
# Environment
##########################################################################################

ui_print "- Constructing environment"

is_mounted /data && MAGISKBIN=/data/magisk || MAGISKBIN=/cache/data_bin

# Copy required files
rm -rf $MAGISKBIN 2>/dev/null
mkdir -p $MAGISKBIN
cp -af $BINDIR/. $COMMONDIR/magisk.apk $COMMONDIR/init.magisk.rc  $COMMONDIR/custom_ramdisk_patch.sh $MAGISKBIN

chmod -R 755 $MAGISKBIN
chcon -h u:object_r:system_file:s0 $MAGISKBIN $MAGISKBIN/*

##########################################################################################
# Magisk Image
##########################################################################################

# Fix SuperSU.....
$BOOTMODE && $BINDIR/magisk magiskpolicy --live "allow fsck * * *"

if (is_mounted /data); then
  IMG=/data/magisk.img
else
  IMG=/cache/magisk.img
  ui_print "- Data unavailable, use cache workaround"
fi

if [ -f $IMG ]; then
  ui_print "- $IMG detected!"
else
  ui_print "- Creating $IMG"
  make_ext4fs -l 32M -a /magisk -S $COMMONDIR/file_contexts_image $IMG
fi

mount_image $IMG /magisk
if (! is_mounted /magisk); then
  ui_print "! Magisk image mount failed..."
  exit 1
fi
MAGISKLOOP=$LOOPDEVICE

# Core folders and scripts
mkdir -p $COREDIR/props $COREDIR/magiskhide $COREDIR/post-fs-data.d $COREDIR/service.d 2>/dev/null
cp -af $COMMONDIR/magiskhide/. $COREDIR/magiskhide

chmod -R 755 $COREDIR/bin $COREDIR/magiskhide $COREDIR/post-fs-data.d $COREDIR/service.d
chown -R 0.0 $COREDIR/bin $COREDIR/magiskhide $COREDIR/post-fs-data.d $COREDIR/service.d

##########################################################################################
# Unpack boot
##########################################################################################

ui_print "- Found Boot Image: $BOOTIMAGE"

rm -rf $BOOTTMP 2>/dev/null
mkdir -p $BOOTTMP
cd $BOOTTMP

ui_print "- Unpacking boot image"
LD_LIBRARY_PATH=$SYSTEMLIB $BINDIR/magiskboot --unpack $BOOTIMAGE

case $? in
  1 )
    ui_print "! Unable to unpack boot image"
    exit 1
    ;;
  2 )
    ui_print "! Sony ELF32 format detected"
    ui_print "! Please use BootBridge from @AdrianDC to flash Magisk"
    exit 1
    ;;
  3 )
    ui_print "! Sony ELF64 format detected"
    ui_print "! Stock kernel cannot be patched, please use a custom kernel"
    exit 1
esac

##########################################################################################
# Ramdisk restores
##########################################################################################

# Update our previous backup to new format if exists
if [ -f /data/stock_boot.img ]; then
  SHA1=`LD_LIBRARY_PATH=$SYSTEMLIB $BINDIR/magiskboot --sha1 /data/stock_boot.img | tail -n 1`
  STOCKDUMP=/data/stock_boot_${SHA1}.img
  mv /data/stock_boot.img $STOCKDUMP
  LD_LIBRARY_PATH=$SYSTEMLIB $BINDIR/magiskboot --compress $STOCKDUMP
fi

# Test patch status and do restore, after this section, ramdisk.cpio.orig is guaranteed to exist
SUPERSU=false
ui_print "- Checking patch status"
LD_LIBRARY_PATH=$SYSTEMLIB $BINDIR/magiskboot --cpio-test ramdisk.cpio
case $? in
  0 )  # Stock boot
    ui_print "- Backing up stock boot image"
    rm -f /data/stock_boot*
    SHA1=`LD_LIBRARY_PATH=$SYSTEMLIB $BINDIR/magiskboot --sha1 $BOOTIMAGE | tail -n 1`
    is_mounted /data && STOCKDUMP=/data/stock_boot_${SHA1}.img || STOCKDUMP=/cache/stock_boot_${SHA1}.img
    dd if=$BOOTIMAGE of=$STOCKDUMP
    LD_LIBRARY_PATH=$SYSTEMLIB $BINDIR/magiskboot --compress $STOCKDUMP
    cp -af ramdisk.cpio ramdisk.cpio.orig
    ;;
  1 )  # Magisk patched
    # Find SHA1 of stock boot image
    if [ -z $SHA1 ]; then
      LD_LIBRARY_PATH=$SYSTEMLIB $BINDIR/magiskboot --cpio-extract ramdisk.cpio init.magisk.rc init.magisk.rc
      SHA1=`grep_prop "# STOCKSHA1" init.magisk.rc`
      [ ! -z $SHA1 ] && STOCKDUMP=/data/stock_boot_${SHA1}.img
      rm -f init.magisk.rc
    fi
    ui_print "- Restoring ramdisk backup"
    LD_LIBRARY_PATH=$SYSTEMLIB $BINDIR/magiskboot --cpio-restore ramdisk.cpio
    if [ $? -ne 0 ]; then
      # Restore failed, try to find original 
      ui_print "! Cannot restore from ramdisk backup"
      ui_print "- Finding stock boot image backup"
      if [ -f ${STOCKDUMP}.gz ]; then
        LD_LIBRARY_PATH=$SYSTEMLIB $BINDIR/magiskboot --decompress ${STOCKDUMP}.gz stock_boot.img
        LD_LIBRARY_PATH=$SYSTEMLIB $BINDIR/magiskboot --unpack stock_boot.img
        rm -f stock_boot.img
      else
        ui_print "! Cannot find stock boot image backup"
        ui_print "! Will still try to complete installation"
      fi
    fi
    cp -af ramdisk.cpio ramdisk.cpio.orig
    ;;
  2 ) # SuperSU patched
    SUPERSU=true
    ui_print "- SuperSU patched boot detected!"
    ui_print "- Adding ramdisk patch script for SuperSU"
    cp -af $COMMONDIR/custom_ramdisk_patch.sh /data/custom_ramdisk_patch.sh
    ui_print "- We are using SuperSU's own tools, mounting su.img"
    is_mounted /data && SUIMG=/data/su.img || SUIMG=/cache/su.img
    mount_image $SUIMG /su
    SUPERSULOOP=$LOOPDEVICE
    if (is_mounted /su); then
      ui_print "- Restoring ramdisk backup with sukernel"
      LD_LIBRARY_PATH=$SYSTEMLIB /su/bin/sukernel --cpio-restore ramdisk.cpio ramdisk.cpio.orig
      if [ $? -ne 0 ]; then
        ui_print "! Cannot restore from ramdisk"
        ui_print "- Finding stock boot image backup with sukernel"
        LD_LIBRARY_PATH=$SYSTEMLIB /su/bin/sukernel --restore ramdisk.cpio stock_boot.img
        if [ $? -eq 0 ]; then
          LD_LIBRARY_PATH=$SYSTEMLIB $BINDIR/magiskboot --unpack stock_boot.img
          cp -af ramdisk.cpio ramdisk.cpio.orig
          rm stock_boot.img
        else
          ui_print "! Cannot find stock boot image backup"
          exit 1
        fi
      fi
    else
      ui_print "! SuperSU image mount failed..."
      ui_print "! Magisk scripts are placed correctly"
      ui_print "! Flash SuperSU immediately to finish installation"
      exit 1
    fi
    ;;
esac

##########################################################################################
# Boot image patches
##########################################################################################

# All ramdisk patch commands are stored in a separate script
ui_print "- Patching ramdisk"

if $SUPERSU; then
  # Use sukernel to patch ramdisk, so we can use its own tools to backup
  sh $COMMONDIR/custom_ramdisk_patch.sh $BOOTTMP/ramdisk.cpio

  # Create ramdisk backups
  LD_LIBRARY_PATH=$SYSTEMLIB /su/bin/sukernel --cpio-backup ramdisk.cpio.orig ramdisk.cpio ramdisk.cpio

else
  # The common patches
  $KEEPVERITY || LD_LIBRARY_PATH=$SYSTEMLIB $BINDIR/magiskboot --cpio-patch-dmverity ramdisk.cpio
  $KEEPFORCEENCRYPT || LD_LIBRARY_PATH=$SYSTEMLIB $BINDIR/magiskboot --cpio-patch-forceencrypt ramdisk.cpio

  # Add magisk entrypoint
  cpio_extract init.rc init.rc
  grep "import /init.magisk.rc" init.rc >/dev/null || sed -i '1,/.*import.*/s/.*import.*/import \/init.magisk.rc\n&/' init.rc
  sed -i "/selinux.reload_policy/d" init.rc
  cpio_add 750 init.rc init.rc

  # sepolicy patches
  cpio_extract sepolicy sepolicy
if $isABDevice
then
  LD_LIBRARY_PATH=$SYSTEMLIB $BINDIR/magiskpolicy --load sepolicy --save sepolicy --minimal
else
  LD_LIBRARY_PATH=$SYSTEMLIB $BINDIR/magisk magiskpolicy --load sepolicy --save sepolicy --minimal
fi
#  LD_LIBRARY_PATH=$SYSTEMLIB $BINDIR/magiskpolicy --load sepolicy --save sepolicy --minimal
  cpio_add 644 sepolicy sepolicy

  # Add new items
  [ ! -z $SHA1 ] && echo "# STOCKSHA1=$SHA1" >> $COMMONDIR/init.magisk.rc
  cpio_add 750 init.magisk.rc $COMMONDIR/init.magisk.rc
  cpio_add 755 sbin/magisk $BINDIR/magisk

  # Create ramdisk backups
  LD_LIBRARY_PATH=$SYSTEMLIB $BINDIR/magiskboot --cpio-backup ramdisk.cpio ramdisk.cpio.orig
fi

if $isABDevice
then
#  LD_LIBRARY_PATH=$SYSTEMLIB $BINDIR/magiskpolicy --load /system/sepolicy --save /system/sepolicy --minimal
  LD_LIBRARY_PATH=$SYSTEMLIB:/system/lib64:/system/lib $BINDIR/magiskpolicy --load /system/sepolicy --save /system/sepolicy --minimal
  mkdir -p /system/magisk 2>/dev/null
  cp -af $COMMONDIR/init.magisk.rc /system/init.magisk.rc
  cp -af $BINDIR/magisk /system/sbin/magisk
  chmod 0755 /system/magisk
  chmod 0750 /system/init.magisk.rc
  chown 0.0 /system/magisk /system/init.magisk.rc
  grep "import /init.magisk.rc" /system/init.rc >/dev/null || sed -i '1,/.*import.*/s/.*import.*/import \/init.magisk.rc\n&/' /system/init.rc
fi

rm -f ramdisk.cpio.orig

##########################################################################################
# Repack and flash
##########################################################################################

# Hexpatches

# Remove Samsung RKP in stock kernel
LD_LIBRARY_PATH=$SYSTEMLIB $BINDIR/magiskboot --hexpatch kernel \
49010054011440B93FA00F71E9000054010840B93FA00F7189000054001840B91FA00F7188010054 \
A1020054011440B93FA00F7140020054010840B93FA00F71E0010054001840B91FA00F7181010054

ui_print "- Repacking boot image"
LD_LIBRARY_PATH=$SYSTEMLIB $BINDIR/magiskboot --repack $BOOTIMAGE

case $? in
  1 )
    ui_print "! Unable to repack boot image!"
    exit 1
    ;;
  2 )
    ui_print "! Boot partition space insufficient"
    ui_print "! Remove ramdisk backups and try again"
    LD_LIBRARY_PATH=$SYSTEMLIB $BINDIR/magiskboot --cpio-rm ramdisk.cpio -r .backup
    LD_LIBRARY_PATH=$SYSTEMLIB $BINDIR/magiskboot --repack $BOOTIMAGE
    if [ $? -eq 2 ]; then
      ui_print "! Boot partition size still too small..."
      ui_print "! Unable to install Magisk"
      exit 1
    fi
    ;;
esac

# Sign chromeos boot
if [ -f chromeos ]; then
  cp -af $CHROMEDIR/. $MAGISKBIN/chromeos
  echo > config
  echo > bootloader
  LD_LIBRARY_PATH=$SYSTEMLIB $CHROMEDIR/futility vbutil_kernel --pack new-boot.img.signed --keyblock $CHROMEDIR/kernel.keyblock --signprivate $CHROMEDIR/kernel_data_key.vbprivk --version 1 --vmlinuz new-boot.img --config config --arch arm --bootloader bootloader --flags 0x1
  rm -f new-boot.img
  mv new-boot.img.signed new-boot.img
fi

ui_print "- Flashing new boot image"
[ ! -L $BOOTIMAGE ] && dd if=/dev/zero of=$BOOTIMAGE bs=4096 2>/dev/null
dd if=new-boot.img of=$BOOTIMAGE bs=4096

cd /

if ! $BOOTMODE; then
  ui_print "- Unmounting partitions"
  umount /magisk
  losetup -d $MAGISKLOOP 2>/dev/null
  rmdir /magisk
  if $SUPERSU; then
    umount /su
    losetup -d $SUPERSULOOP 2>/dev/null
    rmdir /su
  fi
  umount /system
fi

if $isABDevice
then
  mount -o ro /system 2>/dev/null
fi
umount /cache 2>/dev/null
umount /data 2>/dev/null

ui_print "- Done"
exit 0
