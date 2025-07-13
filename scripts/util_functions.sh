############################################
# Magisk General Utility Functions
############################################

#MAGISK_VERSION_STUB

###################
# Global Variables
###################

# True if the script is running on booted Android, not something like recovery
# BOOTMODE=

# The path to store temporary files that don't need to persist
# TMPDIR=

# The non-volatile path where magisk executables are stored
# MAGISKBIN=

###################
# Helper Functions
###################

ui_print() {
  if $BOOTMODE; then
    echo "$1"
  else
    echo -e "ui_print $1\nui_print" >> /proc/self/fd/$OUTFD
  fi
}

toupper() {
  echo "$@" | tr '[:lower:]' '[:upper:]'
}

grep_cmdline() {
  local REGEX="s/^$1=//p"
  { echo $(cat /proc/cmdline)$(sed -e 's/[^"]//g' -e 's/""//g' /proc/cmdline) | xargs -n 1; \
    sed -e 's/ = /=/g' -e 's/, /,/g' -e 's/"//g' /proc/bootconfig; \
  } 2>/dev/null | sed -n "$REGEX"
}

grep_prop() {
  local REGEX="s/^$1=//p"
  shift
  local FILES=$@
  [ -z "$FILES" ] && FILES='/system/build.prop'
  cat $FILES 2>/dev/null | dos2unix | sed -n "$REGEX" | head -n 1
}

grep_get_prop() {
  local result=$(grep_prop $@)
  if [ -z "$result" ]; then
    # Fallback to getprop
    getprop "$1"
  else
    echo $result
  fi
}

getvar() {
  local VARNAME=$1
  local VALUE
  local PROPPATH='/data/.magisk /cache/.magisk'
  [ ! -z $MAGISKTMP ] && PROPPATH="$MAGISKTMP/.magisk/config $PROPPATH"
  VALUE=$(grep_prop $VARNAME $PROPPATH)
  [ ! -z $VALUE ] && eval $VARNAME=\$VALUE
}

is_mounted() {
  grep -q " $(readlink -f $1) " /proc/mounts 2>/dev/null
  return $?
}

abort() {
  ui_print "$1"
  $BOOTMODE || recovery_cleanup
  [ ! -z $MODPATH ] && rm -rf $MODPATH
  rm -rf $TMPDIR
  exit 1
}

print_title() {
  local len line1len line2len bar
  line1len=$(echo -n $1 | wc -c)
  line2len=$(echo -n $2 | wc -c)
  len=$line2len
  [ $line1len -gt $line2len ] && len=$line1len
  len=$((len + 2))
  bar=$(printf "%${len}s" | tr ' ' '*')
  ui_print "$bar"
  ui_print " $1 "
  [ "$2" ] && ui_print " $2 "
  ui_print "$bar"
}

######################
# Environment Related
######################

setup_flashable() {
  ensure_bb
  $BOOTMODE && return
  if [ -z $OUTFD ] || readlink /proc/$$/fd/$OUTFD | grep -q /tmp; then
    # We will have to manually find out OUTFD
    for FD in $(ls /proc/$$/fd); do
      if readlink /proc/$$/fd/$FD | grep -q pipe; then
        if ps | grep -v grep | grep -qE " 3 $FD |status_fd=$FD"; then
          OUTFD=$FD
          break
        fi
      fi
    done
  fi
  recovery_actions
}

ensure_bb() {
  if set -o | grep -q standalone; then
    # We are definitely in busybox ash
    set -o standalone
    return
  fi

  # Find our busybox binary
  local bb
  if [ -f $TMPDIR/busybox ]; then
    bb=$TMPDIR/busybox
  elif [ -f $MAGISKBIN/busybox ]; then
    bb=$MAGISKBIN/busybox
  else
    abort "! Cannot find BusyBox"
  fi
  chmod 755 $bb

  # Busybox could be a script, make sure /system/bin/sh exists
  if [ ! -f /system/bin/sh ]; then
    umount -l /system 2>/dev/null
    mkdir -p /system/bin
    ln -s $(command -v sh) /system/bin/sh
  fi

  export ASH_STANDALONE=1

  # Find our current arguments
  # Run in busybox environment to ensure consistent results
  # /proc/<pid>/cmdline shall be <interpreter> <script> <arguments...>
  local cmds="$($bb sh -c "
  for arg in \$(tr '\0' '\n' < /proc/$$/cmdline); do
    if [ -z \"\$cmds\" ]; then
      # Skip the first argument as we want to change the interpreter
      cmds=\"sh\"
    else
      cmds=\"\$cmds '\$arg'\"
    fi
  done
  echo \$cmds")"

  # Re-exec our script
  echo $cmds | $bb xargs $bb
  exit
}

recovery_actions() {
  # Make sure random won't get blocked
  mount -o bind /dev/urandom /dev/random
  # Unset library paths
  OLD_LD_LIB=$LD_LIBRARY_PATH
  OLD_LD_PRE=$LD_PRELOAD
  OLD_LD_CFG=$LD_CONFIG_FILE
  unset LD_LIBRARY_PATH
  unset LD_PRELOAD
  unset LD_CONFIG_FILE
}

recovery_cleanup() {
  local DIR
  ui_print "- Unmounting partitions"
  (
  if [ ! -d /postinstall/tmp ]; then
    umount -l /system
    umount -l /system_root
  fi
  umount -l /vendor
  umount -l /persist
  umount -l /metadata
  for DIR in /apex /system /system_root; do
    if [ -L "${DIR}_link" ]; then
      rmdir $DIR
      mv -f ${DIR}_link $DIR
    fi
  done
  umount -l /dev/random
  ) 2>/dev/null
  [ -z $OLD_LD_LIB ] || export LD_LIBRARY_PATH=$OLD_LD_LIB
  [ -z $OLD_LD_PRE ] || export LD_PRELOAD=$OLD_LD_PRE
  [ -z $OLD_LD_CFG ] || export LD_CONFIG_FILE=$OLD_LD_CFG
}

#######################
# Installation Related
#######################

# find_block [partname...]
find_block() {
  local BLOCK DEV DEVICE DEVNAME PARTNAME UEVENT
  for BLOCK in "$@"; do
    DEVICE=$(find /dev/block \( -type b -o -type c -o -type l \) -iname $BLOCK | head -n 1) 2>/dev/null
    if [ ! -z $DEVICE ]; then
      echo $DEVICE
      return 0
    fi
  done
  # Fallback by parsing sysfs uevents
  for UEVENT in /sys/dev/block/*/uevent; do
    DEVNAME=$(grep_prop DEVNAME $UEVENT)
    PARTNAME=$(grep_prop PARTNAME $UEVENT)
    for BLOCK in "$@"; do
      if [ "$(toupper $BLOCK)" = "$(toupper $PARTNAME)" ]; then
        echo /dev/block/$DEVNAME
        return 0
      fi
    done
  done
  # Look just in /dev in case we're dealing with MTD/NAND without /dev/block devices/links
  for DEV in "$@"; do
    DEVICE=$(find /dev \( -type b -o -type c -o -type l \) -maxdepth 1 -iname $DEV | head -n 1) 2>/dev/null
    if [ ! -z $DEVICE ]; then
      echo $DEVICE
      return 0
    fi
  done
  return 1
}

# setup_mntpoint <mountpoint>
setup_mntpoint() {
  local POINT=$1
  [ -L $POINT ] && mv -f $POINT ${POINT}_link
  if [ ! -d $POINT ]; then
    rm -f $POINT
    mkdir -p $POINT
  fi
}

# mount_name <partname(s)> <mountpoint> <flag>
mount_name() {
  local PART=$1
  local POINT=$2
  local FLAG=$3
  setup_mntpoint $POINT
  is_mounted $POINT && return
  # First try mounting with fstab
  mount $FLAG $POINT 2>/dev/null
  if ! is_mounted $POINT; then
    local BLOCK=$(find_block $PART)
    mount $FLAG $BLOCK $POINT || return
  fi
  ui_print "- Mounting $POINT"
}

# mount_ro_ensure <partname(s)> <mountpoint>
mount_ro_ensure() {
  # We handle ro partitions only in recovery
  $BOOTMODE && return
  local PART=$1
  local POINT=$2
  mount_name "$PART" $POINT '-o ro'
  is_mounted $POINT || abort "! Cannot mount $POINT"
}

# After calling this method, the following variables will be set:
# SLOT, SYSTEM_AS_ROOT, LEGACYSAR
mount_partitions() {
  # Check A/B slot
  SLOT=$(grep_cmdline androidboot.slot_suffix)
  if [ -z $SLOT ]; then
    SLOT=$(grep_cmdline androidboot.slot)
    [ -z $SLOT ] || SLOT=_${SLOT}
  fi
  [ "$SLOT" = "normal" ] && unset SLOT
  [ -z $SLOT ] || ui_print "- Current boot slot: $SLOT"

  # Mount ro partitions
  if is_mounted /system_root; then
    umount /system 2>/dev/null
    umount /system_root 2>/dev/null
  fi
  mount_ro_ensure "system$SLOT app$SLOT" /system
  if [ -f /system/init -o -L /system/init ]; then
    SYSTEM_AS_ROOT=true
    setup_mntpoint /system_root
    if ! mount --move /system /system_root; then
      umount /system
      umount -l /system 2>/dev/null
      mount_ro_ensure "system$SLOT app$SLOT" /system_root
    fi
    mount -o bind /system_root/system /system
  else
    if grep ' / ' /proc/mounts | grep -qv 'rootfs' || grep -q ' /system_root ' /proc/mounts; then
      SYSTEM_AS_ROOT=true
    else
      SYSTEM_AS_ROOT=false
    fi
  fi
  $SYSTEM_AS_ROOT && ui_print "- Device is system-as-root"

  LEGACYSAR=false
  if $BOOTMODE; then
    grep ' / ' /proc/mounts | grep -q '/dev/root' && LEGACYSAR=true
  else
    # Recovery mode, assume devices that don't use dynamic partitions are legacy SAR
    local IS_DYNAMIC=false
    if grep -q 'androidboot.super_partition' /proc/cmdline; then
      IS_DYNAMIC=true
    elif [ -n "$(find_block super)" ]; then
      IS_DYNAMIC=true
    fi
    if $SYSTEM_AS_ROOT && ! $IS_DYNAMIC; then
      LEGACYSAR=true
      ui_print "- Legacy SAR, force kernel to load rootfs"
    fi
  fi
}

# After calling this method, the following variables will be set:
# ISENCRYPTED, PATCHVBMETAFLAG,
# KEEPVERITY, KEEPFORCEENCRYPT, RECOVERYMODE
get_flags() {
  if grep ' /data ' /proc/mounts | grep -q 'dm-'; then
    ISENCRYPTED=true
  elif [ "$(getprop ro.crypto.state)" = "encrypted" ]; then
    ISENCRYPTED=true
  elif [ "$DATA" = "false" ]; then
    # No data access means unable to decrypt in recovery
    ISENCRYPTED=true
  else
    ISENCRYPTED=false
  fi
  if [ -n "$(find_block vbmeta vbmeta_a)" ]; then
    PATCHVBMETAFLAG=false
  else
    PATCHVBMETAFLAG=true
    ui_print "- No vbmeta partition, patch vbmeta in boot image"
  fi

  # Overridable config flags with safe defaults
  getvar KEEPVERITY
  getvar KEEPFORCEENCRYPT
  getvar RECOVERYMODE
  if [ -z $KEEPVERITY ]; then
    if $SYSTEM_AS_ROOT; then
      KEEPVERITY=true
      ui_print "- System-as-root, keep dm-verity"
    else
      KEEPVERITY=false
    fi
  fi
  if [ -z $KEEPFORCEENCRYPT ]; then
    if $ISENCRYPTED; then
      KEEPFORCEENCRYPT=true
      ui_print "- Encrypted data, keep forceencrypt"
    else
      KEEPFORCEENCRYPT=false
    fi
  fi
  [ -z $RECOVERYMODE ] && RECOVERYMODE=false
}

find_boot_image() {
  BOOTIMAGE=
  if $RECOVERYMODE; then
    BOOTIMAGE=$(find_block "recovery$SLOT" "sos")
  elif [ -e "/dev/block/by-name/init_boot$SLOT" ] && [ "$(uname -r | cut -d. -f1)" -ge 5 ] && uname -r | grep -Evq "android12-|^5\.4"; then
    # init_boot is only used with GKI 13+. It is possible that some devices with init_boot
    # partition still uses Android 12 GKI or previous kernels, so we need to explicitly detect that scenario.
    BOOTIMAGE="/dev/block/by-name/init_boot$SLOT"
  elif [ -e "/dev/block/by-name/boot$SLOT" ]; then
    # Standard location since AOSP Android 10+
    BOOTIMAGE="/dev/block/by-name/boot$SLOT"
  elif [ -n "$SLOT" ]; then
    # Fallback for A/B devices running < Android 10
    BOOTIMAGE=$(find_block "ramdisk$SLOT" "boot$SLOT")
  else
    # Fallback for all legacy and non-standard devices
    BOOTIMAGE=$(find_block ramdisk kern-a android_boot kernel bootimg boot lnx boot_a)
  fi
  if [ -z $BOOTIMAGE ]; then
    # Lets see what fstabs tells me
    BOOTIMAGE=$(grep -v '#' /etc/*fstab* | grep -E '/boot(img)?[^a-zA-Z]' | grep -oE '/dev/[a-zA-Z0-9_./-]*' | head -n 1)
  fi
}

flash_image() {
  local CMD1
  case "$1" in
    *.gz) CMD1="gzip -d < '$1' 2>/dev/null";;
    *)    CMD1="cat '$1'";;
  esac
  if [ -b "$2" ]; then
    local img_sz=$(stat -c '%s' "$1")
    local blk_sz=$(blockdev --getsize64 "$2")
    [ "$img_sz" -gt "$blk_sz" ] && return 1
    blockdev --setrw "$2"
    local blk_ro=$(blockdev --getro "$2")
    [ "$blk_ro" -eq 1 ] && return 2
    eval "$CMD1" | cat - /dev/zero > "$2" 2>/dev/null
  elif [ -c "$2" ]; then
    flash_eraseall "$2" >&2
    eval "$CMD1" | nandwrite -p "$2" - >&2
  else
    ui_print "- Not block or char device, storing image"
    eval "$CMD1" > "$2" 2>/dev/null
  fi
  return 0
}

# Common installation script for flash_script.sh and addon.d.sh
install_magisk() {
  cd $MAGISKBIN

  # Source the boot patcher
  SOURCEDMODE=true
  . ./boot_patch.sh "$BOOTIMAGE"

  ui_print "- Flashing new boot image"
  flash_image new-boot.img "$BOOTIMAGE"
  case $? in
    1)
      abort "! Insufficient partition size"
      ;;
    2)
      abort "! $BOOTIMAGE is read only"
      ;;
  esac

  ./magiskboot cleanup
  rm -f new-boot.img

  run_migrations
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

remove_system_su() {
  [ -d /postinstall/tmp ] && POSTINST=/postinstall
  cd $POSTINST/system
  if [ -f bin/su -o -f xbin/su ] && [ ! -f /su/bin/su ]; then
    ui_print "- Removing system installed root"
    blockdev --setrw /dev/block/mapper/system$SLOT 2>/dev/null
    mount -o rw,remount $POSTINST/system
    # SuperSU
    cd bin
    if [ -e .ext/.su ]; then
      mv -f app_process32_original app_process32 2>/dev/null
      mv -f app_process64_original app_process64 2>/dev/null
      mv -f install-recovery_original.sh install-recovery.sh 2>/dev/null
      if [ -e app_process64 ]; then
        ln -sf app_process64 app_process
      elif [ -e app_process32 ]; then
        ln -sf app_process32 app_process
      fi
    fi
    # More SuperSU, SuperUser & ROM su
    cd ..
    rm -rf .pin bin/.ext etc/.installed_su_daemon etc/.has_su_daemon \
    xbin/daemonsu xbin/su xbin/sugote xbin/sugote-mksh xbin/supolicy \
    bin/app_process_init bin/su /cache/su lib/libsupol.so lib64/libsupol.so \
    su.d etc/init.d/99SuperSUDaemon etc/install-recovery.sh /cache/install-recovery.sh \
    .supersu /cache/.supersu /data/.supersu \
    app/Superuser.apk app/SuperSU /cache/Superuser.apk
  elif [ -f /cache/su.img -o -f /data/su.img -o -d /data/su -o -d /data/adb/su ]; then
    ui_print "- Removing systemless installed root"
    umount -l /su 2>/dev/null
    rm -rf /cache/su.img /data/su.img /data/su /data/adb/su /data/adb/suhide \
    /cache/.supersu /data/.supersu /cache/supersu_install /data/supersu_install
  fi
  cd $TMPDIR
}

api_level_arch_detect() {
  API=$(grep_get_prop ro.build.version.sdk)
  ABI=$(grep_get_prop ro.product.cpu.abi)
  if [ "$ABI" = "arm64-v8a" ]; then
    ARCH=arm64
    ABI32=armeabi-v7a
    IS64BIT=true
  elif [ "$ABI" = "x86_64" ]; then
    ARCH=x64
    ABI32=x86
    IS64BIT=true
  elif [ "$ABI" = "armeabi-v7a" ]; then
    ARCH=arm
    ABI32=armeabi-v7a
    IS64BIT=false
  elif [ "$ABI" = "x86" ]; then
    ARCH=x86
    ABI32=x86
    IS64BIT=false
  elif [ "$ABI" = "riscv64" ]; then
    ARCH=riscv64
    ABI32=riscv32
    IS64BIT=true
  fi
}

check_data() {
  DATA=false
  DATA_DE=false
  if grep ' /data ' /proc/mounts | grep -vq 'tmpfs'; then
    # Test if data is writable
    touch /data/.rw && rm /data/.rw && DATA=true
    # Test if data is decrypted
    $DATA && [ -d /data/adb ] && touch /data/adb/.rw && rm /data/adb/.rw && DATA_DE=true
    $DATA_DE && [ -d /data/adb/magisk ] || mkdir /data/adb/magisk || DATA_DE=false
  fi
  MAGISKBIN="/data/magisk"
  $DATA || MAGISKBIN="/cache/data_adb/magisk"
  $DATA_DE && MAGISKBIN="/data/adb/magisk"
}

run_migrations() {
  local SHA1
  local TARGET
  # Legacy app installation
  local BACKUP=$MAGISKBIN/stock_boot*.gz
  if [ -f $BACKUP ]; then
    cp $BACKUP /data
    rm -f $BACKUP
  fi

  # Legacy backup
  for gz in /data/stock_boot*.gz; do
    [ -f $gz ] || break
    SHA1=$(basename $gz | sed -e 's/stock_boot_//' -e 's/.img.gz//')
    [ -z $SHA1 ] && break
    mkdir /data/magisk_backup_${SHA1} 2>/dev/null
    mv $gz /data/magisk_backup_${SHA1}/boot.img.gz
  done

  # Stock backups
  SHA1=
  for name in boot dtb dtbo dtbs; do
    BACKUP=$MAGISKBIN/stock_${name}.img
    [ -f $BACKUP ] || continue
    if [ $name = 'boot' ]; then
      SHA1=$($MAGISKBIN/magiskboot sha1 $BACKUP)
      mkdir /data/magisk_backup_${SHA1} 2>/dev/null
    fi
    [ -z $SHA1 ] && break
    TARGET=/data/magisk_backup_${SHA1}/${name}.img
    cp $BACKUP $TARGET
    rm -f $BACKUP
    gzip -9f $TARGET
  done

  copy_preinit_files
}

copy_preinit_files() {
  local PREINITDIR=$MAGISKTMP/.magisk/preinit
  if [ ! -d $PREINITDIR ]; then
    ui_print "- Unable to find preinit dir"
    return 1
  fi

  # Copy all enabled sepolicy.rule
  for r in /data/adb/modules*/*/sepolicy.rule; do
    [ -f "$r" ] || continue
    local MODDIR=${r%/*}
    [ -f $MODDIR/disable ] && continue
    [ -f $MODDIR/remove ] && continue
    [ -f $MODDIR/update ] && continue
    cat $r
    echo
  done > $PREINITDIR/sepolicy.rule
}

#################
# Module Related
#################

set_perm() {
  chown $2:$3 $1 || return 1
  chmod $4 $1 || return 1
  local CON=$5
  [ -z $CON ] && CON=u:object_r:system_file:s0
  chcon $CON $1 || return 1
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

boot_actions() { return; }

# Require ZIPFILE to be set
is_legacy_script() {
  unzip -l "$ZIPFILE" install.sh | grep -q install.sh
  return $?
}

# $1 = MODPATH
set_default_perm() {
  set_perm_recursive $1 0 0 0755 0644
  set_perm_recursive $1/system/bin 0 2000 0755 0755
  set_perm_recursive $1/system/xbin 0 2000 0755 0755
  set_perm_recursive $1/system/system_ext/bin 0 2000 0755 0755
  set_perm_recursive $1/system/vendor/bin 0 2000 0755 0755 u:object_r:vendor_file:s0
}

# Require OUTFD, ZIPFILE to be set
install_module() {
  rm -rf $TMPDIR
  mkdir -p $TMPDIR
  chcon u:object_r:system_file:s0 $TMPDIR
  cd $TMPDIR

  setup_flashable
  mount_partitions
  api_level_arch_detect

  # Setup busybox and binaries
  if $BOOTMODE; then
    boot_actions
  else
    recovery_actions
  fi

  # Extract prop file
  unzip -o "$ZIPFILE" module.prop -d $TMPDIR >&2
  [ ! -f $TMPDIR/module.prop ] && abort "! This zip is not a Magisk module!"

  local MODDIRNAME=modules
  $BOOTMODE && MODDIRNAME=modules_update
  local MODULEROOT=/data/adb/$MODDIRNAME
  MODID=$(grep_prop id $TMPDIR/module.prop)
  MODNAME=$(grep_prop name $TMPDIR/module.prop)
  MODAUTH=$(grep_prop author $TMPDIR/module.prop)
  MODPATH=$MODULEROOT/$MODID

  # Create mod paths
  rm -rf $MODPATH
  mkdir -p $MODPATH
  chcon u:object_r:system_file:s0 $MODPATH

  if is_legacy_script; then
    unzip -oj "$ZIPFILE" module.prop install.sh uninstall.sh 'common/*' -d $TMPDIR >&2

    # Load install script
    . $TMPDIR/install.sh

    # Callbacks
    print_modname
    on_install

    [ -f $TMPDIR/uninstall.sh ] && cp -af $TMPDIR/uninstall.sh $MODPATH/uninstall.sh
    $SKIPMOUNT && touch $MODPATH/skip_mount
    $PROPFILE && cp -af $TMPDIR/system.prop $MODPATH/system.prop
    cp -af $TMPDIR/module.prop $MODPATH/module.prop
    $POSTFSDATA && cp -af $TMPDIR/post-fs-data.sh $MODPATH/post-fs-data.sh
    $LATESTARTSERVICE && cp -af $TMPDIR/service.sh $MODPATH/service.sh

    ui_print "- Setting permissions"
    set_permissions
  else
    print_title "$MODNAME" "by $MODAUTH"
    print_title "Powered by Magisk"

    unzip -o "$ZIPFILE" customize.sh -d $MODPATH >&2

    if ! grep -q '^SKIPUNZIP=1$' $MODPATH/customize.sh 2>/dev/null; then
      ui_print "- Extracting module files"
      unzip -o "$ZIPFILE" -x 'META-INF/*' -d $MODPATH >&2
      set_default_perm $MODPATH
    fi

    # Load customization script
    [ -f $MODPATH/customize.sh ] && . $MODPATH/customize.sh
  fi

  # Handle replace folders
  for TARGET in $REPLACE; do
    ui_print "- Replace target: $TARGET"
    mktouch $MODPATH$TARGET/.replace
  done

  for TARGET in $REMOVE; do
    ui_print "- Remove target: $TARGET"
    mkdir -p $(dirname $MODPATH$TARGET) 2>/dev/null
    mknod $MODPATH$TARGET c 0 0
  done

  if $BOOTMODE; then
    # Update info for Magisk app
    mktouch /data/adb/modules/$MODID/update
    rm -rf /data/adb/modules/$MODID/remove 2>/dev/null
    rm -rf /data/adb/modules/$MODID/disable 2>/dev/null
    cp -af $MODPATH/module.prop /data/adb/modules/$MODID/module.prop
  fi

  # Copy over custom sepolicy rules
  if [ -f $MODPATH/sepolicy.rule ]; then
    ui_print "- Installing custom sepolicy rules"
    copy_preinit_files
  fi

  # Remove stuff that doesn't belong to modules and clean up any empty directories
  rm -rf \
  $MODPATH/system/placeholder $MODPATH/customize.sh \
  $MODPATH/README.md $MODPATH/.git*
  rmdir -p $MODPATH 2>/dev/null

  cd /
  $BOOTMODE || recovery_cleanup
  rm -rf $TMPDIR

  ui_print "- Done"
}

##########
# Presets
##########

# Detect whether in boot mode
[ -z $BOOTMODE ] && ps | grep zygote | grep -qv grep && BOOTMODE=true
[ -z $BOOTMODE ] && ps -A 2>/dev/null | grep zygote | grep -qv grep && BOOTMODE=true
[ -z $BOOTMODE ] && BOOTMODE=false

TMPDIR=/dev/tmp
MAGISKBIN="/data/adb/magisk"
