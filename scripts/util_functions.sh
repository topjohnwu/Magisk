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
[ -z $BOOTMODE ] && BOOTMODE=false
$BOOTMODE || ps | grep zygote | grep -qv grep && BOOTMODE=true
$BOOTMODE || ps -A | grep zygote | grep -qv grep && BOOTMODE=true

# Presets
MAGISKTMP=/sbin/.magisk
[ -z $NVBASE ] && NVBASE=/data/adb
[ -z $MAGISKBIN ] && MAGISKBIN=$NVBASE/magisk
[ -z $IMG ] && IMG=$NVBASE/magisk.img

# Bootsigner related stuff
BOOTSIGNERCLASS=a.a
BOOTSIGNER="/system/bin/dalvikvm -Xnodex2oat -Xnoimage-dex2oat -cp \$APK \$BOOTSIGNERCLASS"
BOOTSIGNED=false

setup_flashable() {
  $BOOTMODE && return
  # Preserve environment varibles
  OLD_PATH=$PATH
  setup_bb
  if [ -z $OUTFD ] || readlink /proc/$$/fd/$OUTFD | grep -q /tmp; then
    # We will have to manually find out OUTFD
    for FD in `ls /proc/$$/fd`; do
      if readlink /proc/$$/fd/$FD | grep -q pipe; then
        if ps | grep -v grep | grep -q " 3 $FD "; then
          OUTFD=$FD
          break
        fi
      fi
    done
  fi
}

# Backward compatibility
get_outfd() {
  setup_flashable
}

ui_print() {
  $BOOTMODE && echo "$1" || echo -e "ui_print $1\nui_print" >> /proc/self/fd/$OUTFD
}

toupper() {
  echo "$@" | tr '[:lower:]' '[:upper:]'
}

find_block() {
  for BLOCK in "$@"; do
    DEVICE=`find /dev/block -type l -iname $BLOCK | head -n 1` 2>/dev/null
    if [ ! -z $DEVICE ]; then
      readlink -f $DEVICE
      return 0
    fi
  done
  # Fallback by parsing sysfs uevents
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
  grep -qE '/dev/root|/system_root' /proc/mounts && SYSTEM_ROOT=true || SYSTEM_ROOT=false
  if [ -f /system/init ]; then
    SYSTEM_ROOT=true
    mkdir /system_root 2>/dev/null
    mount --move /system /system_root
    mount -o bind /system_root/system /system
  fi
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
  if [ -z $KEEPVERITY ]; then
    if $SYSTEM_ROOT; then
      KEEPVERITY=true
      ui_print "- Using system_root_image, keep dm/avb-verity"
    else
      KEEPVERITY=false
    fi
  fi
  if [ -z $KEEPFORCEENCRYPT ]; then
    grep ' /data ' /proc/mounts | grep -q 'dm-' && FDE=true || FDE=false
    [ -d /data/unencrypted ] && FBE=true || FBE=false
    # No data access means unable to decrypt in recovery
    if $FDE || $FBE || ! $DATA; then
      KEEPFORCEENCRYPT=true
      ui_print "- Encrypted data detected, keep forceencrypt"
    else
      KEEPFORCEENCRYPT=false
    fi
  fi
}

grep_cmdline() {
  local REGEX="s/^$1=//p"
  cat /proc/cmdline | tr '[:space:]' '\n' | sed -n "$REGEX" 2>/dev/null
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
  VALUE=`grep_prop $VARNAME /sbin/.magisk/config /.backup/.magisk /data/.magisk /cache/.magisk`
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
    mv /data/magisk/stock_boot* /data 2>/dev/null
  fi
  if [ -f /data/adb/magisk/stock_boot* ]; then
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
    BOOTIMAGE=`find_block boot ramdisk boot_a kern-a android_boot kernel lnx bootimg`
  fi
  if [ -z $BOOTIMAGE ]; then
    # Lets see what fstabs tells me
    BOOTIMAGE=`grep -v '#' /etc/*fstab* | grep -E '/boot[^a-zA-Z]' | grep -oE '/dev/[a-zA-Z0-9_./-]*' | head -n 1`
  fi
}

flash_image() {
  # Make sure all blocks are writable
  $MAGISKBIN/magisk --unlock-blocks 2>/dev/null
  case "$1" in
    *.gz) COM1="$MAGISKBIN/magiskboot --decompress '$1' - 2>/dev/null";;
    *)    COM1="cat '$1'";;
  esac
  if $BOOTSIGNED; then
    COM2="$BOOTSIGNER -sign"
    ui_print "- Sign image with test keys"
  else
    COM2="cat -"
  fi
  if [ -b "$2" ]; then
    local s_size=`stat -c '%s' "$1"`
    local t_size=`blockdev --getsize64 "$2"`
    [ $s_size -gt $t_size ] && return 1
    eval $COM1 | eval $COM2 | cat - /dev/zero > "$2" 2>/dev/null
  else
    ui_print "- Not block device, storing image"
    eval $COM1 | eval $COM2 > "$2" 2>/dev/null
  fi
  return 0
}

find_dtbo_image() {
  DTBOIMAGE=`find_block dtbo$SLOT`
}

patch_dtbo_image() {
  if [ ! -z $DTBOIMAGE ]; then
    if $MAGISKBIN/magiskboot --dtb-test $DTBOIMAGE; then
      ui_print "- Backing up stock DTBO image"
      $MAGISKBIN/magiskboot --compress $DTBOIMAGE $MAGISKBIN/stock_dtbo.img.gz
      ui_print "- Patching DTBO to remove avb-verity"
      $MAGISKBIN/magiskboot --dtb-patch $DTBOIMAGE
      return 0
    fi
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
    ui_print "- Removing system installed root"
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

check_data() {
  DATA=false
  DATA_DE=false
  if grep ' /data ' /proc/mounts | grep -vq 'tmpfs'; then
    # Test if data is writable
    touch /data/.rw && rm /data/.rw && DATA=true
    # Test if DE storage is writable
    $DATA && [ -d /data/adb ] && touch /data/adb/.rw && rm /data/adb/.rw && DATA_DE=true
  fi
}

setup_bb() {
  if [ -x $MAGISKTMP/busybox/busybox ]; then
    # Make sure this path is in the front
    echo $PATH | grep -q "^$MAGISKTMP/busybox" || export PATH=$MAGISKTMP/busybox:$PATH
  elif [ -x $TMPDIR/bin/busybox ]; then
    # Make sure this path is in the front
    echo $PATH | grep -q "^$TMPDIR/bin" || export PATH=$TMPDIR/bin:$PATH
  else
    # Construct the PATH
    mkdir -p $TMPDIR/bin
    ln -s $MAGISKBIN/busybox $TMPDIR/bin/busybox
    $MAGISKBIN/busybox --install -s $TMPDIR/bin
    export PATH=$TMPDIR/bin:$PATH
  fi
}

boot_actions() {
  if [ ! -d $MAGISKTMP/mirror/bin ]; then
    mkdir -p $MAGISKTMP/mirror/bin
    mount -o bind $MAGISKBIN $MAGISKTMP/mirror/bin
  fi
  MAGISKBIN=$MAGISKTMP/mirror/bin
  setup_bb
}

recovery_actions() {
  # TWRP bug fix
  mount -o bind /dev/urandom /dev/random
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
  reqSizeM=`du -ms $1 | cut -f1`
}

request_zip_size_check() {
  reqSizeM=`unzip -l "$1" | tail -n 1 | awk '{ print int(($1 - 1) / 1048576 + 1) }'`
}

check_filesystem() {
  curSizeM=`wc -c < $1`
  curSizeM=$((curSizeM / 1048576))
  local DF=`df -Pk $2 | grep $2`
  curUsedM=`echo $DF | awk '{ print int($3 / 1024) }'`
  curFreeM=`echo $DF | awk '{ print int($4 / 1024) }'`
}

mount_snippet() {
  MAGISKLOOP=`$MAGISKBIN/magisk imgtool mount $IMG $MOUNTPATH`
  is_mounted $MOUNTPATH || abort "! $IMG mount failed..."
}

mount_magisk_img() {
  [ -z reqSizeM ] && reqSizeM=0
  mkdir -p $MOUNTPATH 2>/dev/null
  if [ -f "$IMG" ]; then
    ui_print "- Found $IMG"
    mount_snippet
    check_filesystem $IMG $MOUNTPATH
    if [ $reqSizeM -gt $curFreeM ]; then
      newSizeM=$(((curSizeM + reqSizeM - curFreeM) / 32 * 32 + 64))
      ui_print "- Resizing $IMG to ${newSizeM}M"
      $MAGISKBIN/magisk imgtool umount $MOUNTPATH $MAGISKLOOP
      $MAGISKBIN/magisk imgtool resize $IMG $newSizeM >&2
      mount_snippet
    fi
    ui_print "- Mount $IMG to $MOUNTPATH"
  else
    newSizeM=$((reqSizeM / 32 * 32 + 64))
    ui_print "- Creating $IMG with size ${newSizeM}M"
    $MAGISKBIN/magisk imgtool create $IMG $newSizeM >&2
    mount_snippet
  fi
}

unmount_magisk_img() {
  check_filesystem $IMG $MOUNTPATH
  newSizeM=$((curUsedM / 32 * 32 + 64))
  $MAGISKBIN/magisk imgtool umount $MOUNTPATH $MAGISKLOOP
  if [ $curSizeM -gt $newSizeM ]; then
    ui_print "- Shrinking $IMG to ${newSizeM}M"
    $MAGISKBIN/magisk imgtool resize $IMG $newSizeM >&2
  fi
}

find_manager_apk() {
  APK=/data/adb/magisk.apk
  [ -f $APK ] || APK=/data/magisk/magisk.apk
  [ -f $APK ] || APK=/data/app/com.topjohnwu.magisk*/*.apk
  if [ ! -f $APK ]; then
    DBAPK=`magisk --sqlite "SELECT value FROM strings WHERE key='requester'" | cut -d= -f2`
    [ -z "$DBAPK" ] || APK=/data/app/$DBAPK*/*.apk
  fi
}
