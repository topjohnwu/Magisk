env_check() {
  for file in busybox magisk magiskboot magiskinit util_functions.sh boot_patch.sh; do
    [ -f /data/adb/magisk/$file ] || return 1
  done
  return 0
}

fix_env() {
  cd /data/adb/magisk
  sh update-binary extract
  rm -f update-binary magisk.apk
  cd /
  rm -rf /sbin/.magisk/busybox/*
  /sbin/.magisk/mirror/bin/busybox --install -s /sbin/.magisk/busybox
}

direct_install() {
  rm -rf /data/adb/magisk/* 2>/dev/null
  mkdir -p /data/adb/magisk 2>/dev/null
  chmod 700 /data/adb
  cp -rf $1/* /data/adb/magisk
  rm -rf /data/adb/magisk/new-boot.img
  echo "- Flashing new boot image"
  flash_image $1/new-boot.img $2
  if [ $? -ne 0 ]; then
    echo "! Insufficient partition size"
    return 1
  fi
  rm -rf $1
  return 0
}

mm_patch_dtbo() {
  if $KEEPVERITY; then
    return 1
  else
    find_dtbo_image
    patch_dtbo_image
  fi
}

restore_imgs() {
  local SHA1=`grep_prop SHA1 /sbin/.magisk/config`
  [ -z $SHA1 ] && local SHA1=`cat /.backup/.sha1`
  [ -z $SHA1 ] && return 1
  local STOCKBOOT=/data/stock_boot_${SHA1}.img.gz
  local STOCKDTBO=/data/stock_dtbo.img.gz
  [ -f $STOCKBOOT ] || return 1

  find_boot_image
  find_dtbo_image

  if [ -f $STOCKDTBO -a -b "$DTBOIMAGE" ]; then
    flash_image $STOCKDTBO $DTBOIMAGE
  fi
  if [ -f $STOCKBOOT -a -b "$BOOTIMAGE" ]; then
    flash_image $STOCKBOOT $BOOTIMAGE
    return 0
  fi
  return 1
}

post_ota() {
  cd $1
  chmod 755 bootctl
  ./bootctl hal-info || return
  [ `./bootctl get-current-slot` -eq 0 ] && SLOT_NUM=1 || SLOT_NUM=0
  ./bootctl set-active-boot-slot $SLOT_NUM
  echo '${0%/*}/../bootctl mark-boot-successful;rm -f ${0%/*}/../bootctl $0' > post-fs-data.d/post_ota.sh
  chmod 755 post-fs-data.d/post_ota.sh
  cd /
}

add_hosts_module() {
  # Do not touch existing hosts module
  [ -d /sbin/.magisk/img/hosts ] && return
  cd /sbin/.magisk/img
  mkdir -p hosts/system/etc
  cat << EOF > hosts/module.prop
id=hosts
name=Systemless Hosts
version=1.0
versionCode=1
author=Magisk Manager
description=Magisk Manager built-in systemless hosts module
minMagisk=17000
EOF
  if [ -f .core/hosts ]; then
    # Migrate old hosts file to new module
    mv -f .core/hosts hosts/system/etc/hosts
  else
    cp -f /system/etc/hosts hosts/system/etc/hosts
  fi
  magisk --clone-attr /system/etc/hosts hosts/system/etc/hosts
  touch hosts/update
  touch hosts/auto_mount
  cd /
}

rm_launch() {
  db_clean $1
  pm uninstall $2
  monkey -p $3 1
  exit
}
