##########################################################################################
#
# Magisk General Utility Functions
# by topjohnwu
#
# Used everywhere in Magisk
#
##########################################################################################

#MAGISK_VERSION_STUB

# Detect whether in boot mode
ps | grep zygote | grep -v grep >/dev/null && BOOTMODE=true || BOOTMODE=false
$BOOTMODE || ps -A 2>/dev/null | grep zygote | grep -v grep >/dev/null && BOOTMODE=true
$BOOTMODE || id | grep -q 'uid=0' || BOOTMODE=true

# Default location, will override if needed
MAGISKBIN=/data/adb/magisk
[ -z $MOUNTPATH ] && MOUNTPATH=/sbin/.core/img
[ -z $IMG ] && IMG=/data/adb/magisk.img

BOOTSIGNER="/system/bin/dalvikvm -Xnodex2oat -Xnoimage-dex2oat -cp \$APK com.topjohnwu.magisk.utils.BootSigner"
BOOTSIGNED=false

get_outfd() {
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
}

ui_print() {
  $BOOTMODE && echo "$1" || echo -e "ui_print $1\nui_print" >> /proc/self/fd/$OUTFD
}

toupper() {
  echo "$@" | tr '[:lower:]' '[:upper:]'
}

find_block() {
  for uevent in /sys/dev/block/*/uevent; do
    local DEVNAME=`grep_prop DEVNAME $uevent`
    local PARTNAME=`grep_prop PARTNAME $uevent`
    for p in "$@"; do
      if [ "`toupper $p`" = "`toupper $PARTNAME`" ]; then
        echo /dev/block/$DEVNAME
        return 0
      fi
    done
  done
  return 1
}

mount_partitions() {
  # Check A/B slot
  SLOT=`grep_cmdline androidboot.slot_suffix`
  if [ -z $SLOT ]; then
    SLOT=_`grep_cmdline androidboot.slot`
    [ $SLOT = "_" ] && SLOT=
  fi
  [ -z $SLOT ] || ui_print "- Current boot slot: $SLOT"

  ui_print "- Mounting /system, /vendor"
  [ -f /system/build.prop ] || is_mounted /system || mount -o ro /system 2>/dev/null
  if ! is_mounted /system && ! [ -f /system/build.prop ]; then
    SYSTEMBLOCK=`find_block system$SLOT`
    mount -t ext4 -o ro $SYSTEMBLOCK /system
  fi
  [ -f /system/build.prop ] || is_mounted /system || abort "! Cannot mount /system"
  cat /proc/mounts | grep -E '/dev/root|/system_root' >/dev/null && SYSTEM_ROOT=true || SYSTEM_ROOT=false
  if [ -f /system/init ]; then
    SYSTEM_ROOT=true
    mkdir /system_root 2>/dev/null
    mount --move /system /system_root
    mount -o bind /system_root/system /system
  fi
  $SYSTEM_ROOT && ui_print "- Device using system_root_image"
  if [ -L /system/vendor ]; then
    # Seperate /vendor partition
    is_mounted /vendor || mount -o ro /vendor 2>/dev/null
    if ! is_mounted /vendor; then
      VENDORBLOCK=`find_block vendor$SLOT`
      mount -t ext4 -o ro $VENDORBLOCK /vendor
    fi
    is_mounted /vendor || abort "! Cannot mount /vendor"
  fi
}

get_flags() {
  # override variables
  getvar KEEPVERITY
  getvar KEEPFORCEENCRYPT
  HIGHCOMP=false
  if [ -z $KEEPVERITY ]; then
    KEEPVERITY=false
    hardware=`grep_cmdline androidboot.hardware`
    for hw in taimen walleye; do
      if [ "$hw" = "$hardware" ]; then
        KEEPVERITY=true
        ui_print "- Device on whitelist, keep avb-verity"
        break
      fi
    done
  fi
  if [ -z $KEEPFORCEENCRYPT ]; then
    if [ "`getprop ro.crypto.state`" = "encrypted" ]; then
      KEEPFORCEENCRYPT=true
      ui_print "- Encrypted data detected, keep forceencrypt"
    else
      KEEPFORCEENCRYPT=false
    fi
  fi
}

grep_cmdline() {
  local REGEX="s/^$1=//p"
  sed -E 's/ +/\n/g' /proc/cmdline | sed -n "$REGEX" 2>/dev/null
}

grep_prop() {
  local REGEX="s/^$1=//p"
  shift
  local FILES=$@
  [ -z "$FILES" ] && FILES='/system/build.prop'
  sed -n "$REGEX" $FILES 2>/dev/null | head -n 1
}

getvar() {
  local VARNAME=$1
  local VALUE=
  VALUE=`grep_prop $VARNAME /.backup/.magisk /data/.magisk /cache/.magisk /system/.magisk`
  [ ! -z $VALUE ] && eval $VARNAME=\$VALUE
}

run_migrations() {
  # Update the broken boot backup
  if [ -f /data/stock_boot_.img.gz ]; then
    $MAGISKBIN/magiskboot --decompress /data/stock_boot_.img.gz /data/stock_boot.img
  fi
  # Update our previous backup to new format if exists
  if [ -f /data/stock_boot.img ]; then
    ui_print "- Migrating boot image backup"
    SHA1=`$MAGISKBIN/magiskboot --sha1 /data/stock_boot.img 2>/dev/null`
    STOCKDUMP=/data/stock_boot_${SHA1}.img
    mv /data/stock_boot.img $STOCKDUMP
    $MAGISKBIN/magiskboot --compress $STOCKDUMP
  fi
  # Move the stock backups
  if [ -f /data/magisk/stock_boot* ]; then
    rm -rf /data/stock_boot*
    mv /data/magisk/stock_boot* /data 2>/dev/null
  fi
  if [ -f /data/adb/magisk/stock_boot* ]; then
    rm -rf /data/stock_boot*
    mv /data/adb/magisk/stock_boot* /data 2>/dev/null
  fi
  # Remove old dbs
  rm -f /data/user*/*/magisk.db
  [ -L /data/magisk.img ] || mv /data/magisk.img /data/adb/magisk.img 2>/dev/null
}

find_boot_image() {
  BOOTIMAGE=
  if [ ! -z $SLOT ]; then
    BOOTIMAGE=`find_block boot$SLOT ramdisk$SLOT`
  else
    BOOTIMAGE=`find_block boot_a kern-a android_boot kernel boot lnx bootimg`
  fi
}

flash_boot_image() {
  # Make sure all blocks are writable
  $MAGISKBIN/magisk --unlock-blocks 2>/dev/null
  case "$1" in
    *.gz) COMMAND="gzip -d < '$1'";;
    *)    COMMAND="cat '$1'";;
  esac
  if $BOOTSIGNED; then
    SIGNCOM="$BOOTSIGNER -sign"
    ui_print "- Sign boot image with test keys"
  else
    SIGNCOM="cat -"
  fi
  case "$2" in
    /dev/block/*)
      ui_print "- Flashing new boot image"
      eval $COMMAND | eval $SIGNCOM | cat - /dev/zero 2>/dev/null | dd of="$2" bs=4096 2>/dev/null
      ;;
    *)
      ui_print "- Storing new boot image"
      eval $COMMAND | eval $SIGNCOM | dd of="$2" bs=4096 2>/dev/null
      ;;
  esac
}

find_dtbo_image() {
  DTBOIMAGE=`find_block dtbo$SLOT`
}

patch_dtbo_image() {
  if [ ! -z $DTBOIMAGE ]; then
    if $MAGISKBIN/magiskboot --dtb-test $DTBOIMAGE; then
      ui_print "- Backing up stock dtbo image"
      $MAGISKBIN/magiskboot --compress $DTBOIMAGE $MAGISKBIN/stock_dtbo.img.gz
      ui_print "- Patching fstab in dtbo to remove avb-verity"
      $MAGISKBIN/magiskboot --dtb-patch $DTBOIMAGE
      return 0
    fi
  fi
  return 1
}

restore_imgs() {
  STOCKBOOT=/data/stock_boot_${1}.img.gz
  STOCKDTBO=/data/stock_dtbo.img.gz

  # Make sure all blocks are writable
  $MAGISKBIN/magisk --unlock-blocks 2>/dev/null
  find_dtbo_image
  if [ ! -z "$DTBOIMAGE" -a -f "$STOCKDTBO" ]; then
    ui_print "- Restoring stock dtbo image"
    gzip -d < $STOCKDTBO | dd of=$DTBOIMAGE
  fi
  BOOTSIGNED=false
  find_boot_image
  if [ ! -z "$BOOTIMAGE" -a -f "$STOCKBOOT" ]; then
    ui_print "- Restoring stock boot image"
    gzip -d < $STOCKBOOT | cat - /dev/zero 2>/dev/null | dd of="$BOOTIMAGE" bs=4096 2>/dev/null
    return 0
  fi
  return 1
}

sign_chromeos() {
  ui_print "- Signing ChromeOS boot image"

  echo > empty
  ./chromeos/futility vbutil_kernel --pack new-boot.img.signed \
  --keyblock ./chromeos/kernel.keyblock --signprivate ./chromeos/kernel_data_key.vbprivk \
  --version 1 --vmlinuz new-boot.img --config empty --arch arm --bootloader empty --flags 0x1

  rm -f empty new-boot.img
  mv new-boot.img.signed new-boot.img
}

is_mounted() {
  cat /proc/mounts | grep -q " `readlink -f $1` " 2>/dev/null
  return $?
}

remove_system_su() {
  if [ -f /system/bin/su -o -f /system/xbin/su ] && [ ! -f /su/bin/su ]; then
    ui_print "! System installed root detected, mount rw :("
    mount -o rw,remount /system
    # SuperSU
    if [ -e /system/bin/.ext/.su ]; then
      mv -f /system/bin/app_process32_original /system/bin/app_process32 2>/dev/null
      mv -f /system/bin/app_process64_original /system/bin/app_process64 2>/dev/null
      mv -f /system/bin/install-recovery_original.sh /system/bin/install-recovery.sh 2>/dev/null
      cd /system/bin
      if [ -e app_process64 ]; then
        ln -sf app_process64 app_process
      else
        ln -sf app_process32 app_process
      fi
    fi
    rm -rf /system/.pin /system/bin/.ext /system/etc/.installed_su_daemon /system/etc/.has_su_daemon \
    /system/xbin/daemonsu /system/xbin/su /system/xbin/sugote /system/xbin/sugote-mksh /system/xbin/supolicy \
    /system/bin/app_process_init /system/bin/su /cache/su /system/lib/libsupol.so /system/lib64/libsupol.so \
    /system/su.d /system/etc/install-recovery.sh /system/etc/init.d/99SuperSUDaemon /cache/install-recovery.sh \
    /system/.supersu /cache/.supersu /data/.supersu \
    /system/app/Superuser.apk /system/app/SuperSU /cache/Superuser.apk  2>/dev/null
  fi
}

api_level_arch_detect() {
  API=`grep_prop ro.build.version.sdk`
  ABI=`grep_prop ro.product.cpu.abi | cut -c-3`
  ABI2=`grep_prop ro.product.cpu.abi2 | cut -c-3`
  ABILONG=`grep_prop ro.product.cpu.abi`

  ARCH=arm
  ARCH32=arm
  IS64BIT=false
  if [ "$ABI" = "x86" ]; then ARCH=x86; ARCH32=x86; fi;
  if [ "$ABI2" = "x86" ]; then ARCH=x86; ARCH32=x86; fi;
  if [ "$ABILONG" = "arm64-v8a" ]; then ARCH=arm64; ARCH32=arm; IS64BIT=true; fi;
  if [ "$ABILONG" = "x86_64" ]; then ARCH=x64; ARCH32=x86; IS64BIT=true; fi;
}

boot_actions() {
  if [ ! -d /sbin/.core/mirror/bin ]; then
    mkdir -p /sbin/.core/mirror/bin
    mount -o bind $MAGISKBIN /sbin/.core/mirror/bin
  fi
  MAGISKBIN=/sbin/.core/mirror/bin
}

recovery_actions() {
  # TWRP bug fix
  mount -o bind /dev/urandom /dev/random
  # Preserve environment varibles
  OLD_PATH=$PATH
  if [ ! -d $TMPDIR/bin ]; then
    # Add busybox to PATH
    mkdir -p $TMPDIR/bin
    ln -s $MAGISKBIN/busybox $TMPDIR/bin/busybox
    $MAGISKBIN/busybox --install -s $TMPDIR/bin
    export PATH=$TMPDIR/bin:$PATH
  fi
  # Temporarily block out all custom recovery binaries/libs
  mv /sbin /sbin_tmp
  # Unset library paths
  OLD_LD_LIB=$LD_LIBRARY_PATH
  OLD_LD_PRE=$LD_PRELOAD
  unset LD_LIBRARY_PATH
  unset LD_PRELOAD
}

recovery_cleanup() {
  mv /sbin_tmp /sbin 2>/dev/null
  [ -z $OLD_PATH ] || export PATH=$OLD_PATH
  [ -z $OLD_LD_LIB ] || export LD_LIBRARY_PATH=$OLD_LD_LIB
  [ -z $OLD_LD_PRE ] || export LD_PRELOAD=$OLD_LD_PRE
  ui_print "- Unmounting partitions"
  umount -l /system_root 2>/dev/null
  umount -l /system 2>/dev/null
  umount -l /vendor 2>/dev/null
  umount -l /dev/random 2>/dev/null
}

abort() {
  ui_print "$1"
  $BOOTMODE || recovery_cleanup
  exit 1
}

set_perm() {
  chown $2:$3 $1 || return 1
  chmod $4 $1 || return 1
  [ -z $5 ] && chcon 'u:object_r:system_file:s0' $1 || chcon $5 $1 || return 1
}

set_perm_recursive() {
  find $1 -type d 2>/dev/null | while read dir; do
    set_perm $dir $2 $3 $4 $6
  done
  find $1 -type f -o -type l 2>/dev/null | while read file; do
    set_perm $file $2 $3 $5 $6
  done
}

mktouch() {
  mkdir -p ${1%/*} 2>/dev/null
  [ -z $2 ] && touch $1 || echo $2 > $1
  chmod 644 $1
}

request_size_check() {
  reqSizeM=`du -s $1 | cut -f1`
  reqSizeM=$((reqSizeM / 1024 + 1))
}

request_zip_size_check() {
  reqSizeM=`unzip -l "$1" | tail -n 1 | awk '{ print int($1 / 1048567 + 1) }'`
}

image_size_check() {
  SIZE="`$MAGISKBIN/magisk --imgsize $IMG`"
  curUsedM=`echo "$SIZE" | cut -d" " -f1`
  curSizeM=`echo "$SIZE" | cut -d" " -f2`
  curFreeM=$((curSizeM - curUsedM))
}

mount_magisk_img() {
  [ -z reqSizeM ] && reqSizeM=0
  if [ -f "$IMG" ]; then
    ui_print "- Found $IMG"
    image_size_check $IMG
    if [ "$reqSizeM" -gt "$curFreeM" ]; then
      newSizeM=$(((reqSizeM + curUsedM) / 32 * 32 + 64))
      ui_print "- Resizing $IMG to ${newSizeM}M"
      $MAGISKBIN/magisk --resizeimg $IMG $newSizeM >&2
    fi
  else
    newSizeM=$((reqSizeM / 32 * 32 + 64));
    ui_print "- Creating $IMG with size ${newSizeM}M"
    $MAGISKBIN/magisk --createimg $IMG $newSizeM >&2
  fi

  ui_print "- Mounting $IMG to $MOUNTPATH"
  mkdir -p $MOUNTPATH 2>/dev/null
  MAGISKLOOP=`$MAGISKBIN/magisk --mountimg $IMG $MOUNTPATH`
  is_mounted $MOUNTPATH || abort "! $IMG mount failed..."
}

unmount_magisk_img() {
  $MAGISKBIN/magisk --umountimg $MOUNTPATH $MAGISKLOOP

  # Shrink the image if possible
  image_size_check $IMG
  newSizeM=$((curUsedM / 32 * 32 + 64))
  if [ $curSizeM -gt $newSizeM ]; then
    ui_print "- Shrinking $IMG to ${newSizeM}M"
    $MAGISKBIN/magisk --resizeimg $IMG $newSizeM
  fi
}

