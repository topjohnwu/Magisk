##################################
# Magisk Manager internal scripts
##################################

env_check() {
  for file in busybox magisk magiskboot magiskinit util_functions.sh boot_patch.sh; do
    [ -f $MAGISKBIN/$file ] || return 1
  done
  return 0
}

fix_env() {
  cd $MAGISKBIN
  PATH=/sbin:/system/bin sh update-binary -x
  ./busybox rm -f $MAGISKTMP/busybox/*
  cp -af busybox $MAGISKTMP/busybox/busybox
  $MAGISKTMP/busybox/busybox --install -s $MAGISKTMP/busybox
  rm -f update-binary magisk.apk
  chmod -R 755 .
  ./magiskinit -x magisk magisk
  cd /
}

direct_install() {
  rm -rf $MAGISKBIN/* 2>/dev/null
  mkdir -p $MAGISKBIN 2>/dev/null
  chmod 700 $NVBASE
  cp -af $1/. $MAGISKBIN
  rm -f $MAGISKBIN/new-boot.img
  echo "- Flashing new boot image"
  flash_image $1/new-boot.img $2
  if [ $? -ne 0 ]; then
    echo "! Insufficient partition size"
    return 1
  fi
  rm -rf $1
  return 0
}

mm_patch_dtb() {
  (local result=1
  local PATCHED=$TMPDIR/dt.patched
  for name in dtb dtbo; do
    local IMAGE=`find_block $name$SLOT`
    if [ ! -z $IMAGE ]; then
      if $MAGISKBIN/magiskboot dtb $IMAGE patch $PATCHED; then
        result=0
        if [ ! -z $SHA1 ]; then
          # Backup stuffs
          mkdir /data/magisk_backup_${SHA1} 2>/dev/null
          cat $IMAGE | gzip -9 > /data/magisk_backup_${SHA1}/${name}.img.gz
        fi
        cat $PATCHED /dev/zero > $IMAGE
        rm -f $PATCHED
      fi
    fi
  done
  # Run broadcast command passed from app
  eval $1
  )& >/dev/null 2>&1
}

restore_imgs() {
  [ -z $SHA1 ] && return 1
  local BACKUPDIR=/data/magisk_backup_$SHA1
  [ -d $BACKUPDIR ] || return 1

  get_flags
  find_boot_image

  for name in dtb dtbo; do
    [ -f $BACKUPDIR/${name}.img.gz ] || continue
    local IMAGE=`find_block $name$SLOT`
    [ -z $IMAGE ] && continue
    flash_image $BACKUPDIR/${name}.img.gz $IMAGE
  done
  [ -f $BACKUPDIR/boot.img.gz ] || return 1
  flash_image $BACKUPDIR/boot.img.gz $BOOTIMAGE
}

post_ota() {
  cd $1
  chmod 755 bootctl
  ./bootctl hal-info || return
  [ `./bootctl get-current-slot` -eq 0 ] && SLOT_NUM=1 || SLOT_NUM=0
  ./bootctl set-active-boot-slot $SLOT_NUM
  cat << EOF > post-fs-data.d/post_ota.sh
${1}/bootctl mark-boot-successful
rm -f ${1}/bootctl
rm -f ${1}/post-fs-data.d/post_ota.sh
EOF
  chmod 755 post-fs-data.d/post_ota.sh
  cd /
}

add_hosts_module() {
  # Do not touch existing hosts module
  [ -d /sbin/.magisk/modules/hosts ] && return
  cd /sbin/.magisk/modules
  mkdir -p hosts/system/etc
  cat << EOF > hosts/module.prop
id=hosts
name=Systemless Hosts
version=1.0
versionCode=1
author=Magisk Manager
description=Magisk Manager built-in systemless hosts module
EOF
  cp -f /system/etc/hosts hosts/system/etc/hosts
  magisk --clone-attr /system/etc/hosts hosts/system/etc/hosts
  touch hosts/update
  cd /
}

force_pm_install() {
  local APK=$1
  local VERIFY=`settings get global package_verifier_enable`
  [ "$VERIFY" -eq 1 ] && settings put global package_verifier_enable 0
  pm install -r $APK
  local res=$?
  [ "$VERIFY" -eq 1 ] && settings put global package_verifier_enable 1
  return $res
}

check_boot_ramdisk() {
  # Create boolean ISAB
  [ -z $SLOT ] && ISAB=false || ISAB=true

  # If we are running as recovery mode, then we do not have ramdisk in boot
  $RECOVERYMODE && return 1

  # If we are A/B, then we must have ramdisk
  $ISAB && return 0

  # If we are using legacy SAR, but not AB, we do not have ramdisk in boot
  if grep ' / ' /proc/mounts | grep -q '/dev/root'; then
    # Override recovery mode to true
    RECOVERYMODE=true
    return 1
  fi

  return 0
}

##########################
# Non-root util_functions
##########################

mount_partitions() {
  [ "`getprop ro.build.ab_update`" = "true" ] && SLOT=`getprop ro.boot.slot_suffix`
  # Check whether non rootfs root dir exists
  grep ' / ' /proc/mounts | grep -qv 'rootfs' && SYSTEM_ROOT=true || SYSTEM_ROOT=false
}

get_flags() {
  $SYSTEM_ROOT && KEEPVERITY=true || KEEPVERITY=false
  [ "`getprop ro.crypto.state`" = "encrypted" ] && KEEPFORCEENCRYPT=true || KEEPFORCEENCRYPT=false
  RECOVERYMODE=false
}

run_migrations() { return; }
