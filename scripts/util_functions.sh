#########################################
#
# Magisk General Utility Functions
# by topjohnwu
#
#########################################

#MAGISK_VERSION_STUB

###################
# Helper Functions
###################

ui_print() {
  $BOOTMODE && echo "$1" || echo -e "ui_print $1\nui_print" >> /proc/self/fd/$OUTFD
}

toupper() {
  echo "$@" | tr '[:lower:]' '[:upper:]'
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
  VALUE=`grep_prop $VARNAME /sbin/.magisk/config /data/.magisk /cache/.magisk`
  [ ! -z $VALUE ] && eval $VARNAME=\$VALUE
}

is_mounted() {
  grep -q " `readlink -f $1` " /proc/mounts 2>/dev/null
  return $?
}

abort() {
  ui_print "$1"
  $BOOTMODE || recovery_cleanup
  exit 1
}

resolve_vars() {
  MAGISKBIN=$NVBASE/magisk
  POSTFSDATAD=$NVBASE/post-fs-data.d
  SERVICED=$NVBASE/service.d
}

print_title() {
  local len=$(echo -n $1 | wc -c)
  len=$((len + 2))
  local pounds=$(printf "%${len}s" | tr ' ' '*')
  ui_print "$pounds"
  ui_print " $1 "
  ui_print "$pounds"
}

######################
# Environment Related
######################

setup_flashable() {
  ensure_bb "$@"
  $BOOTMODE && return
  if [ -z $OUTFD ] || readlink /proc/$$/fd/$OUTFD | grep -q /tmp; then
    # We will have to manually find out OUTFD
    for FD in `ls /proc/$$/fd`; do
      if readlink /proc/$$/fd/$FD | grep -q pipe; then
        if ps | grep -v grep | grep -qE " 3 $FD |status_fd=$FD"; then
          OUTFD=$FD
          break
        fi
      fi
    done
  fi
}

ensure_bb() {
  [ $(echo $- | grep "S") ] && return

  # At this point, we are not running in BusyBox ash
  # Find our busybox binary
  local BUSYBOX
  if [ -f $TMPDIR/busybox ]; then
    BUSYBOX=$TMPDIR/busybox
  elif [ -f $MAGISKBIN/busybox ]; then
    BUSYBOX=$MAGISKBIN/busybox
  else
    abort "! Cannot find BusyBox"
  fi

  # Re-exec our script
  chmod 755 $BUSYBOX
  exec $BUSYBOX sh -o standalone $0 "$@"
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
  (umount_apex
  if [ ! -d /postinstall/tmp ]; then
    umount -l /system
    umount -l /system_root
  fi
  umount -l /vendor
  umount -l /persist
  for DIR in /apex /system /system_root; do
    if [ -L "${DIR}_link" ]; then
      rmdir $DIR
      mv -f ${DIR}_link $DIR
    fi
  done
  umount -l /dev/random) 2>/dev/null
  export PATH=$OLD_PATH
  [ -z $OLD_LD_LIB ] || export LD_LIBRARY_PATH=$OLD_LD_LIB
  [ -z $OLD_LD_PRE ] || export LD_PRELOAD=$OLD_LD_PRE
  [ -z $OLD_LD_CFG ] || export LD_CONFIG_FILE=$OLD_LD_CFG
}

#######################
# Installation Related
#######################

# find_block [partname...]
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
    for BLOCK in "$@"; do
      if [ "`toupper $BLOCK`" = "`toupper $PARTNAME`" ]; then
        echo /dev/block/$DEVNAME
        return 0
      fi
    done
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
  ui_print "- Mounting $POINT"
  # First try mounting with fstab
  mount $FLAG $POINT 2>/dev/null
  if ! is_mounted $POINT; then
    local BLOCK=`find_block $PART`
    mount $FLAG $BLOCK $POINT
  fi
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

mount_partitions() {
  # Check A/B slot
  SLOT=`grep_cmdline androidboot.slot_suffix`
  if [ -z $SLOT ]; then
    SLOT=`grep_cmdline androidboot.slot`
    [ -z $SLOT ] || SLOT=_${SLOT}
  fi
  [ -z $SLOT ] || ui_print "- Current boot slot: $SLOT"

  # Mount ro partitions
  mount_ro_ensure "system$SLOT app$SLOT" /system
  if [ -f /system/init.rc ]; then
    SYSTEM_ROOT=true
    setup_mntpoint /system_root
    if ! mount --move /system /system_root; then
      umount /system
      umount -l /system 2>/dev/null
      mount_ro_ensure "system$SLOT app$SLOT" /system_root
    fi
    mount -o bind /system_root/system /system
  else
    grep ' / ' /proc/mounts | grep -qv 'rootfs' || grep -q ' /system_root ' /proc/mounts \
    && SYSTEM_ROOT=true || SYSTEM_ROOT=false
  fi
  [ -L /system/vendor ] && mount_ro_ensure vendor$SLOT /vendor
  $SYSTEM_ROOT && ui_print "- Device is system-as-root"

  # Allow /system/bin commands (dalvikvm) on Android 10+ in recovery
  $BOOTMODE || mount_apex

  # Mount persist partition in recovery
  if ! $BOOTMODE && [ ! -z $PERSISTDIR ]; then
    # Try to mount persist
    PERSISTDIR=/persist
    mount_name persist /persist
    if ! is_mounted /persist; then
      # Fallback to cache
      mount_name "cache cac" /cache
      is_mounted /cache && PERSISTDIR=/cache || PERSISTDIR=
    fi
  fi
}

# loop_setup <ext4_img>, sets LOOPDEV
loop_setup() {
  unset LOOPDEV
  local LOOP
  local MINORX=1
  [ -e /dev/block/loop1 ] && MINORX=$(stat -Lc '%T' /dev/block/loop1)
  local NUM=0
  while [ $NUM -lt 64 ]; do
    LOOP=/dev/block/loop$NUM
    [ -e $LOOP ] || mknod $LOOP b 7 $((NUM * MINORX))
    if losetup $LOOP "$1" 2>/dev/null; then
      LOOPDEV=$LOOP
      break
    fi
    NUM=$((NUM + 1))
  done
}

mount_apex() {
  $BOOTMODE || [ ! -d /system/apex ] && return
  local APEX DEST
  setup_mntpoint /apex
  for APEX in /system/apex/*; do
    DEST=/apex/$(basename $APEX .apex)
    [ "$DEST" == /apex/com.android.runtime.release ] && DEST=/apex/com.android.runtime
    mkdir -p $DEST 2>/dev/null
    if [ -f $APEX ]; then
      # APEX APKs, extract and loop mount
      unzip -qo $APEX apex_payload.img -d /apex
      loop_setup apex_payload.img
      if [ ! -z $LOOPDEV ]; then
        ui_print "- Mounting $DEST"
        mount -t ext4 -o ro,noatime $LOOPDEV $DEST
      fi
      rm -f apex_payload.img
    elif [ -d $APEX ]; then
      # APEX folders, bind mount directory
      ui_print "- Mounting $DEST"
      mount -o bind $APEX $DEST
    fi
  done
  export ANDROID_RUNTIME_ROOT=/apex/com.android.runtime
  export ANDROID_TZDATA_ROOT=/apex/com.android.tzdata
  local APEXRJPATH=/apex/com.android.runtime/javalib
  local SYSFRAME=/system/framework
  export BOOTCLASSPATH=\
$APEXRJPATH/core-oj.jar:$APEXRJPATH/core-libart.jar:$APEXRJPATH/okhttp.jar:\
$APEXRJPATH/bouncycastle.jar:$APEXRJPATH/apache-xml.jar:$SYSFRAME/framework.jar:\
$SYSFRAME/ext.jar:$SYSFRAME/telephony-common.jar:$SYSFRAME/voip-common.jar:\
$SYSFRAME/ims-common.jar:$SYSFRAME/android.test.base.jar:$SYSFRAME/telephony-ext.jar:\
/apex/com.android.conscrypt/javalib/conscrypt.jar:\
/apex/com.android.media/javalib/updatable-media.jar
}

umount_apex() {
  [ -d /apex ] || return
  local DEST SRC
  for DEST in /apex/*; do
    [ "$DEST" = '/apex/*' ] && break
    SRC=$(grep $DEST /proc/mounts | awk '{ print $1 }')
    umount -l $DEST
    # Detach loop device just in case
    losetup -d $SRC 2>/dev/null
  done
  rm -rf /apex
  unset ANDROID_RUNTIME_ROOT
  unset ANDROID_TZDATA_ROOT
  unset BOOTCLASSPATH
}

get_flags() {
  # override variables
  getvar KEEPVERITY
  getvar KEEPFORCEENCRYPT
  getvar RECOVERYMODE
  if [ -z $KEEPVERITY ]; then
    if $SYSTEM_ROOT; then
      KEEPVERITY=true
      ui_print "- System-as-root, keep dm/avb-verity"
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
    BOOTIMAGE=`find_block recovery_ramdisk$SLOT recovery sos`
  elif [ ! -z $SLOT ]; then
    BOOTIMAGE=`find_block ramdisk$SLOT recovery_ramdisk$SLOT boot$SLOT`
  else
    BOOTIMAGE=`find_block ramdisk recovery_ramdisk kern-a android_boot kernel boot lnx bootimg boot_a`
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
    *.gz) CMD1="$MAGISKBIN/magiskboot decompress '$1' - 2>/dev/null";;
    *)    CMD1="cat '$1'";;
  esac
  if $BOOTSIGNED; then
    CMD2="$BOOTSIGNER -sign"
    ui_print "- Sign image with verity keys"
  else
    CMD2="cat -"
  fi
  if [ -b "$2" ]; then
    local img_sz=`stat -c '%s' "$1"`
    local blk_sz=`blockdev --getsize64 "$2"`
    [ $img_sz -gt $blk_sz ] && return 1
    eval $CMD1 | eval $CMD2 | cat - /dev/zero > "$2" 2>/dev/null
  else
    ui_print "- Not block device, storing image"
    eval $CMD1 | eval $CMD2 > "$2" 2>/dev/null
  fi
  return 0
}

patch_dtb_partitions() {
  local result=1
  cd $MAGISKBIN
  for name in dtb dtbo; do
    local IMAGE=`find_block $name$SLOT`
    if [ ! -z $IMAGE ]; then
      ui_print "- $name image: $IMAGE"
      if ./magiskboot dtb $IMAGE patch dt.patched; then
        result=0
        ui_print "- Backing up stock $name image"
        cat $IMAGE > stock_${name}.img
        ui_print "- Flashing patched $name"
        cat dt.patched /dev/zero > $IMAGE
        rm -f dt.patched
      fi
    fi
  done
  cd /
  return $result
}

# Common installation script for flash_script.sh and addon.d.sh
install_magisk() {
  cd $MAGISKBIN

  eval $BOOTSIGNER -verify < $BOOTIMAGE && BOOTSIGNED=true
  $BOOTSIGNED && ui_print "- Boot image is signed with AVB 1.0"

  $IS64BIT && mv -f magiskinit64 magiskinit 2>/dev/null || rm -f magiskinit64

  # Source the boot patcher
  SOURCEDMODE=true
  . ./boot_patch.sh "$BOOTIMAGE"

  ui_print "- Flashing new boot image"

  if ! flash_image new-boot.img "$BOOTIMAGE"; then
    ui_print "- Compressing ramdisk to fit in partition"
    ./magiskboot cpio ramdisk.cpio compress
    ./magiskboot repack "$BOOTIMAGE"
    flash_image new-boot.img "$BOOTIMAGE" || abort "! Insufficient partition size"
  fi

  ./magiskboot cleanup
  rm -f new-boot.img

  patch_dtb_partitions
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
      elif [ -e app_process32 ]; then
        ln -sf app_process32 app_process
      fi
    fi
    rm -rf /system/.pin /system/bin/.ext /system/etc/.installed_su_daemon /system/etc/.has_su_daemon \
    /system/xbin/daemonsu /system/xbin/su /system/xbin/sugote /system/xbin/sugote-mksh /system/xbin/supolicy \
    /system/bin/app_process_init /system/bin/su /cache/su /system/lib/libsupol.so /system/lib64/libsupol.so \
    /system/su.d /system/etc/install-recovery.sh /system/etc/init.d/99SuperSUDaemon /cache/install-recovery.sh \
    /system/.supersu /cache/.supersu /data/.supersu \
    /system/app/Superuser.apk /system/app/SuperSU /cache/Superuser.apk
  elif [ -f /cache/su.img -o -f /data/su.img -o -d /data/adb/su -o -d /data/su ]; then
    ui_print "- Removing systemless installed root"
    umount -l /su 2>/dev/null
    rm -rf /cache/su.img /data/su.img /data/adb/su /data/adb/suhide /data/su /cache/.supersu /data/.supersu \
    /cache/supersu_install /data/supersu_install
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
  $DATA && NVBASE=/data || NVBASE=/cache/data_adb
  $DATA_DE && NVBASE=/data/adb
  resolve_vars
}

find_manager_apk() {
  [ -z $APK ] && APK=/data/adb/magisk.apk
  [ -f $APK ] || APK=/data/magisk/magisk.apk
  [ -f $APK ] || APK=/data/app/com.topjohnwu.magisk*/*.apk
  if [ ! -f $APK ]; then
    DBAPK=`magisk --sqlite "SELECT value FROM strings WHERE key='requester'" 2>/dev/null | cut -d= -f2`
    [ -z $DBAPK ] && DBAPK=`strings /data/adb/magisk.db | grep 5requester | cut -c11-`
    [ -z $DBAPK ] || APK=/data/user_de/*/$DBAPK/dyn/*.apk
    [ -f $APK ] || [ -z $DBAPK ] || APK=/data/app/$DBAPK*/*.apk
  fi
  [ -f $APK ] || ui_print "! Unable to detect Magisk Manager APK for BootSigner"
}

run_migrations() {
  local LOCSHA1
  local TARGET
  # Legacy app installation
  local BACKUP=/data/adb/magisk/stock_boot*.gz
  if [ -f $BACKUP ]; then
    cp $BACKUP /data
    rm -f $BACKUP
  fi

  # Legacy backup
  for gz in /data/stock_boot*.gz; do
    [ -f $gz ] || break
    LOCSHA1=`basename $gz | sed -e 's/stock_boot_//' -e 's/.img.gz//'`
    [ -z $LOCSHA1 ] && break
    mkdir /data/magisk_backup_${LOCSHA1} 2>/dev/null
    mv $gz /data/magisk_backup_${LOCSHA1}/boot.img.gz
  done

  # Stock backups
  LOCSHA1=$SHA1
  for name in boot dtb dtbo; do
    BACKUP=/data/adb/magisk/stock_${name}.img
    [ -f $BACKUP ] || continue
    if [ $name = 'boot' ]; then
      LOCSHA1=`$MAGISKBIN/magiskboot sha1 $BACKUP`
      mkdir /data/magisk_backup_${LOCSHA1} 2>/dev/null
    fi
    TARGET=/data/magisk_backup_${LOCSHA1}/${name}.img
    cp $BACKUP $TARGET
    rm -f $BACKUP
    gzip -9f $TARGET
  done
}

#################
# Module Related
#################

set_perm() {
  chown $2:$3 $1 || return 1
  chmod $4 $1 || return 1
  CON=$5
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

request_size_check() {
  reqSizeM=`du -ms "$1" | cut -f1`
}

request_zip_size_check() {
  reqSizeM=`unzip -l "$1" | tail -n 1 | awk '{ print int(($1 - 1) / 1048576 + 1) }'`
}

boot_actions() { return; }

# Require ZIPFILE to be set
is_legacy_script() {
  unzip -l "$ZIPFILE" install.sh | grep -q install.sh
  return $?
}

# Require OUTFD, ZIPFILE to be set
install_module() {
  local PERSISTDIR=/sbin/.magisk/mirror/persist

  setup_flashable "$@"
  mount_partitions
  api_level_arch_detect

  # Setup busybox and binaries
  $BOOTMODE && boot_actions || recovery_actions

  # Extract prop file
  unzip -o "$ZIPFILE" module.prop -d $TMPDIR >&2
  [ ! -f $TMPDIR/module.prop ] && abort "! Unable to extract zip file!"

  $BOOTMODE && MODDIRNAME=modules_update || MODDIRNAME=modules
  MODULEROOT=$NVBASE/$MODDIRNAME
  MODID=`grep_prop id $TMPDIR/module.prop`
  MODPATH=$MODULEROOT/$MODID
  MODNAME=`grep_prop name $TMPDIR/module.prop`

  # Create mod paths
  rm -rf $MODPATH 2>/dev/null
  mkdir -p $MODPATH

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
    print_title "$MODNAME"
    print_title "Powered by Magisk"

    unzip -o "$ZIPFILE" customize.sh -d $MODPATH >&2

    if ! grep -q '^SKIPUNZIP=1$' $MODPATH/customize.sh 2>/dev/null; then
      ui_print "- Extracting module files"
      unzip -o "$ZIPFILE" -x 'META-INF/*' -d $MODPATH >&2

      # Default permissions
      set_perm_recursive $MODPATH 0 0 0755 0644
    fi

    # Load customization script
    [ -f $MODPATH/customize.sh ] && . $MODPATH/customize.sh
  fi

  # Handle replace folders
  for TARGET in $REPLACE; do
    ui_print "- Replace target: $TARGET"
    mktouch $MODPATH$TARGET/.replace
  done

  if $BOOTMODE; then
    # Update info for Magisk Manager
    mktouch $NVBASE/modules/$MODID/update
    cp -af $MODPATH/module.prop $NVBASE/modules/$MODID/module.prop
  fi

  # Copy over custom sepolicy rules
  if [ -f $MODPATH/sepolicy.rule -a -e $PERSISTDIR ]; then
    ui_print "- Installing custom sepolicy patch"
    PERSISTMOD=$PERSISTDIR/magisk/$MODID
    mkdir -p $PERSISTMOD
    cp -af $MODPATH/sepolicy.rule $PERSISTMOD/sepolicy.rule
  fi

  # Remove stuffs that don't belong to modules
  rm -rf \
  $MODPATH/system/placeholder $MODPATH/customize.sh \
  $MODPATH/README.md $MODPATH/.git* 2>/dev/null

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

MAGISKTMP=/sbin/.magisk
NVBASE=/data/adb
[ -z $TMPDIR ] && TMPDIR=/dev/tmp

# Bootsigner related stuff
BOOTSIGNERCLASS=a.a
BOOTSIGNER="/system/bin/dalvikvm -Xnoimage-dex2oat -cp \$APK \$BOOTSIGNERCLASS"
BOOTSIGNED=false

resolve_vars
