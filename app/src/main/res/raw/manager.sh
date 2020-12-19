##################################
# Magisk Manager internal scripts
##################################

run_delay() {
  (sleep $1; $2)&
}

env_check() {
  for file in busybox magisk magiskboot magiskinit util_functions.sh boot_patch.sh; do
    [ -f $MAGISKBIN/$file ] || return 1
  done
  return 0
}

fix_env() {
  cd $MAGISKBIN
  PATH=/system/bin /system/bin/sh update-binary -x
  ./busybox rm -f update-binary magisk.apk
  ./busybox chmod -R 755 .
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
  case $? in
    1)
      echo "! Insufficient partition size"
      return 1
      ;;
    2)
      echo "! $2 is read only"
      return 2
      ;;
  esac
  rm -rf $1
  return 0
}

restore_imgs() {
  [ -z $SHA1 ] && return 1
  local BACKUPDIR=/data/magisk_backup_$SHA1
  [ -d $BACKUPDIR ] || return 1

  get_flags
  find_boot_image

  for name in dtb dtbo; do
    [ -f $BACKUPDIR/${name}.img.gz ] || continue
    local IMAGE=$(find_block $name$SLOT)
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
  [ $(./bootctl get-current-slot) -eq 0 ] && SLOT_NUM=1 || SLOT_NUM=0
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
  [ -d $MAGISKTMP/modules/hosts ] && return
  cd $MAGISKTMP/modules
  mkdir -p hosts/system/etc
  cat << EOF > hosts/module.prop
id=hosts
name=Systemless Hosts
version=1.0
versionCode=1
author=Magisk Manager
description=Magisk Manager built-in systemless hosts module
EOF
  magisk --clone /system/etc/hosts hosts/system/etc/hosts
  touch hosts/update
  cd /
}

adb_pm_install() {
  local tmp=/data/local/tmp/temp.apk
  cp -f "$1" $tmp
  chmod 644 $tmp
  su 2000 -c pm install $tmp || pm install $tmp
  local res=$?
  rm -f $tmp
  return $res
}

check_boot_ramdisk() {
  # Create boolean ISAB
  [ -z $SLOT ] && ISAB=false || ISAB=true

  # If we are running as recovery mode, then we do not have ramdisk
  [ "$RECOVERYMODE" = "true" ] && return 1

  # If we are A/B, then we must have ramdisk
  $ISAB && return 0

  # If we are using legacy SAR, but not A/B, assume we do not have ramdisk
  if grep ' / ' /proc/mounts | grep -q '/dev/root'; then
    # Override recovery mode to true if not set
    [ -z $RECOVERYMODE ] && RECOVERYMODE=true
    return 1
  fi

  return 0
}

check_encryption() {
  if $ISENCRYPTED; then
    if [ $SDK_INT -lt 24 ]; then
      CRYPTOTYPE="block"
    else
      # First see what the system tells us
      CRYPTOTYPE=$(getprop ro.crypto.type)
      if [ -z $CRYPTOTYPE ]; then
        # If not mounting through device mapper, we are FBE
        if grep ' /data ' /proc/mounts | grep -qv 'dm-'; then
          CRYPTOTYPE="file"
        else
          # We are either FDE or metadata encryption (which is also FBE)
          grep -q ' /metadata ' /proc/mounts && CRYPTOTYPE="file" || CRYPTOTYPE="block"
        fi
      fi
    fi
  else
    CRYPTOTYPE="N/A"
  fi
}

##########################
# Non-root util_functions
##########################

mount_partitions() {
  [ "$(getprop ro.build.ab_update)" = "true" ] && SLOT=$(getprop ro.boot.slot_suffix)
  # Check whether non rootfs root dir exists
  grep ' / ' /proc/mounts | grep -qv 'rootfs' && SYSTEM_ROOT=true || SYSTEM_ROOT=false
}

get_flags() {
  KEEPVERITY=$SYSTEM_ROOT
  [ "$(getprop ro.crypto.state)" = "encrypted" ] && ISENCRYPTED=true || ISENCRYPTED=false
  KEEPFORCEENCRYPT=$ISENCRYPTED
  # Do NOT preset RECOVERYMODE here
}

run_migrations() { return; }

grep_prop() { return; }

#############
# Initialize
#############

mm_init() {
  export BOOTMODE=true
  mount_partitions
  get_flags
  run_migrations
  SHA1=$(grep_prop SHA1 $MAGISKTMP/config)
  check_boot_ramdisk && RAMDISKEXIST=true || RAMDISKEXIST=false
  check_encryption
  # Make sure RECOVERYMODE has value
  [ -z $RECOVERYMODE ] && RECOVERYMODE=false
}
