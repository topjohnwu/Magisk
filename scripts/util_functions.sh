##########################################################################################
# 
# Magisk General Utility Functions
# by topjohnwu
# 
# Used in flash_script.sh, addon.d.sh, magisk module installers, and uninstaller
# 
##########################################################################################

MAGISK_VERSION_STUB

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
  if [ -z "$BOOTIMAGE" ]; then
    srch_str="boot_a BOOT_A kern-a KERN-A android_boot ANDROID_BOOT kernel KERNEL boot BOOT lnx LNX"
    if $isABDevice
    then
      srch_str="boot_$SLOT BOOT_$SLOT kern-$SLOT KERN-$SLOT"+$srch_str
    fi
    for BLOCK in $srch_str; do
      BOOTIMAGE=`ls /dev/block/by-name/$BLOCK || ls /dev/block/platform/*/by-name/$BLOCK || ls /dev/block/platform/*/*/by-name/$BLOCK` 2>/dev/null
      [ ! -z $BOOTIMAGE ] && break
    done
  fi
  # Recovery fallback
  if [ -z "$BOOTIMAGE" ]; then
    for FSTAB in /etc/*fstab*; do
      BOOTIMAGE=`grep -E '\b/boot\b' $FSTAB | grep -v "#" | grep -oE '/dev/[a-zA-Z0-9_./-]*'`
      [ ! -z $BOOTIMAGE ] && break
    done
  fi
  [ -L "$BOOTIMAGE" ] && BOOTIMAGE=`readlink $BOOTIMAGE`
}

is_mounted() {
  if [ ! -z "$2" ]; then
    cat /proc/mounts | grep $1 | grep $2, >/dev/null
  else
    cat /proc/mounts | grep $1 >/dev/null
  fi
  return $?
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

api_level_arch_detect() {
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
}

recovery_actions() {
  # TWRP bug fix
  mount -o bind /dev/urandom /dev/random
  # Temporarily block out all custom recovery binaries/libs
  mv /sbin /sbin_tmp
  # Add all possible library paths
  OLD_LD_PATH=$LD_LIBRARY_PATH
  $IS64BIT && export LD_LIBRARY_PATH=$SYSTEM/lib64:$SYSTEM/vendor/lib64 || export LD_LIBRARY_PATH=$SYSTEM/lib:$SYSTEM/vendor/lib
}

recovery_cleanup() {
  mv /sbin_tmp /sbin
  # Clear LD_LIBRARY_PATH
  export LD_LIBRARY_PATH=$OLD_LD_PATH
  ui_print "- Unmounting partitions"
  umount -l /system
  umount -l /vendor 2>/dev/null
  umount -l /dev/random
}

abort() {
  ui_print "$1"
  mv /sbin_tmp /sbin 2>/dev/null
  exit 1
}

set_perm() {
  chown $2:$3 $1 || exit 1
  chmod $4 $1 || exit 1
  if [ ! -z $5 ]; then
    chcon $5 $1 2>/dev/null
  else
    chcon 'u:object_r:system_file:s0' $1 2>/dev/null
  fi
}

set_perm_recursive() {
  find $1 -type d 2>/dev/null | while read dir; do
    set_perm $dir $2 $3 $4 $6
  done
  find $1 -type f 2>/dev/null | while read file; do
    set_perm $file $2 $3 $5 $6
  done
}

mktouch() {
  mkdir -p ${1%/*}
  if [ -z "$2" ]; then
    touch $1
  else
    echo $2 > $1
  fi
  chmod 644 $1
}

request_size_check() {
  reqSizeM=`du -s $1 | cut -f1`
  reqSizeM=$((reqSizeM / 1024 + 1))
}

image_size_check() {
  if $isABDevice
  then
    SIZE="`$MAGISKBIN/magisk_utils --imgsize $IMG`"
  else
    SIZE="`$MAGISKBIN/magisk --imgsize $IMG`"
  fi
  curUsedM=`echo "$SIZE" | cut -d" " -f1`
  curSizeM=`echo "$SIZE" | cut -d" " -f2`
  curFreeM=$((curSizeM - curUsedM))
}

ABdevice_check() {
  SYSTEM=/system
  ABDeviceCheck=$(cat /proc/cmdline | grep slot_suffix | wc -l)
  if [ $ABDeviceCheck -gt 0 ];
  then
    isABDevice=true
    SLOT=$(for i in `cat /proc/cmdline`; do echo $i | grep slot_suffix | awk -F "=" '{print $2}';done)
    SYSTEM=$SYSTEM/system
  else
    isABDevice=false
  fi
}
