##################################
# Magisk app internal scripts
##################################

# $1 = delay
# $2 = command
run_delay() {
  (sleep $1; $2)&
}

# $1 = version string
# $2 = version code
env_check() {
  for file in busybox magiskboot magiskinit util_functions.sh boot_patch.sh; do
    [ -f "$MAGISKBIN/$file" ] || return 1
  done
  if [ "$2" -ge 25000 ]; then
    [ -f "$MAGISKBIN/magiskpolicy" ] || return 1
  fi
  if [ "$2" -ge 25210 ]; then
    [ -b "$MAGISKTMP/.magisk/device/preinit" ] || [ -b "$MAGISKTMP/.magisk/block/preinit" ] || return 2
  fi
  grep -xqF "MAGISK_VER='$1'" "$MAGISKBIN/util_functions.sh" || return 3
  grep -xqF "MAGISK_VER_CODE=$2" "$MAGISKBIN/util_functions.sh" || return 3
  return 0
}

# $1 = dir to copy
# $2 = destination (optional)
cp_readlink() {
  if [ -z $2 ]; then
    cd $1
  else
    cp -af $1/. $2
    cd $2
  fi
  for file in *; do
    if [ -L $file ]; then
      local full=$(readlink -f $file)
      rm $file
      cp -af $full $file
    fi
  done
  chmod -R 755 .
  cd /
}

# $1 = install dir
fix_env() {
  # Cleanup and make dirs
  rm -rf $MAGISKBIN/*
  mkdir -p $MAGISKBIN 2>/dev/null
  chmod 700 /data/adb
  cp_readlink $1 $MAGISKBIN
  rm -rf $1
  chown -R 0:0 $MAGISKBIN
}

# $1 = install dir
# $2 = boot partition
direct_install() {
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

  rm -f $1/new-boot.img
  fix_env $1
  run_migrations

  return 0
}

# $1 = uninstaller zip
run_uninstaller() {
  rm -rf /dev/tmp
  mkdir -p /dev/tmp/install
  unzip -o "$1" "assets/*" "lib/*" -d /dev/tmp/install
  INSTALLER=/dev/tmp/install sh /dev/tmp/install/assets/uninstaller.sh dummy 1 "$1"
}

# $1 = boot partition
restore_imgs() {
  local SHA1=$(grep_prop SHA1 $MAGISKTMP/.magisk/config)
  local BACKUPDIR=/data/magisk_backup_$SHA1
  [ -d $BACKUPDIR ] || return 1
  [ -f $BACKUPDIR/boot.img.gz ] || return 1
  flash_image $BACKUPDIR/boot.img.gz $1
}

# $1 = path to bootctl executable
post_ota() {
  cd /data/adb
  cp -f $1 bootctl
  rm -f $1
  chmod 755 bootctl
  if ! ./bootctl hal-info; then
    rm -f bootctl
    return
  fi
  SLOT_NUM=0
  [ $(./bootctl get-current-slot) -eq 0 ] && SLOT_NUM=1
  ./bootctl set-active-boot-slot $SLOT_NUM
  cat << EOF > post-fs-data.d/post_ota.sh
/data/adb/bootctl mark-boot-successful
rm -f /data/adb/bootctl
rm -f /data/adb/post-fs-data.d/post_ota.sh
EOF
  chmod 755 post-fs-data.d/post_ota.sh
  cd /
}

add_hosts_module() {
  # Do not touch existing hosts module
  [ -d /data/adb/modules/hosts ] && return
  cd /data/adb/modules
  mkdir -p hosts/system/etc
  cat << EOF > hosts/module.prop
id=hosts
name=Systemless Hosts
version=1.0
versionCode=1
author=Magisk
description=Magisk app built-in systemless hosts module
EOF
  magisk --clone /system/etc/hosts hosts/system/etc/hosts
  touch hosts/update
  cd /
}

# $1 = APK
# $2 = package name
adb_pm_install() {
  local tmp=/data/local/tmp/temp.apk
  cp -f "$1" $tmp
  chmod 644 $tmp
  su 2000 -c pm install -g $tmp || pm install -g $tmp || su 1000 -c pm install -g $tmp
  local res=$?
  rm -f $tmp
  if [ $res = 0 ]; then
    appops set "$2" REQUEST_INSTALL_PACKAGES allow
  fi
  return $res
}

check_boot_ramdisk() {
  # Create boolean ISAB
  ISAB=true
  [ -z $SLOT ] && ISAB=false

  # If we are A/B, then we must have ramdisk
  $ISAB && return 0

  # If we are using legacy SAR, but not A/B, assume we do not have ramdisk
  if $LEGACYSAR; then
    # Override recovery mode to true
    RECOVERYMODE=true
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
          CRYPTOTYPE="block"
          grep -q ' /metadata ' /proc/mounts && CRYPTOTYPE="file"
        fi
      fi
    fi
  else
    CRYPTOTYPE="N/A"
  fi
}

printvar() {
  eval echo $1=\$$1
}

run_action() {
  local MODID="$1"
  cd "/data/adb/modules/$MODID"
  sh ./action.sh
  local RES=$?
  cd /
  return $RES
}

##########################
# Non-root util_functions
##########################

mount_partitions() {
  [ "$(getprop ro.build.ab_update)" = "true" ] && SLOT=$(getprop ro.boot.slot_suffix)
  # Check whether non rootfs root dir exists
  SYSTEM_AS_ROOT=false
  grep ' / ' /proc/mounts | grep -qv 'rootfs' && SYSTEM_AS_ROOT=true

  LEGACYSAR=false
  grep ' / ' /proc/mounts | grep -q '/dev/root' && LEGACYSAR=true
}

get_flags() {
  KEEPVERITY=$SYSTEM_AS_ROOT
  ISENCRYPTED=false
  [ "$(getprop ro.crypto.state)" = "encrypted" ] && ISENCRYPTED=true
  KEEPFORCEENCRYPT=$ISENCRYPTED
  if [ -n "$(getprop ro.boot.vbmeta.device)" -o -n "$(getprop ro.boot.vbmeta.size)" ]; then
    PATCHVBMETAFLAG=false
  elif getprop ro.product.ab_ota_partitions | grep -wq vbmeta; then
    PATCHVBMETAFLAG=false
  else
    PATCHVBMETAFLAG=true
  fi
  [ -z $RECOVERYMODE ] && RECOVERYMODE=false
}

run_migrations() { return; }

grep_prop() { return; }

#############
# Initialize
#############

app_init() {
  mount_partitions >/dev/null
  RAMDISKEXIST=false
  check_boot_ramdisk && RAMDISKEXIST=true
  get_flags >/dev/null
  run_migrations >/dev/null
  check_encryption

  # Dump variables
  printvar SLOT
  printvar SYSTEM_AS_ROOT
  printvar RAMDISKEXIST
  printvar ISAB
  printvar CRYPTOTYPE
  printvar PATCHVBMETAFLAG
  printvar LEGACYSAR
  printvar RECOVERYMODE
  printvar KEEPVERITY
  printvar KEEPFORCEENCRYPT
}

export BOOTMODE=true
