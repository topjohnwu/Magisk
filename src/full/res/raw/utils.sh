db_sepatch() {
  magiskpolicy --live 'create magisk_file' 'attradd magisk_file mlstrustedobject' \
  'allow * magisk_file file *' 'allow * magisk_file dir *' \
  'allow magisk_file * filesystem associate'
}

db_clean() {
  local USERID=$1
  local DIR="/sbin/.core/db-${USERID}"
  umount -l /data/user*/*/*/databases/su.db $DIR $DIR/*
  rm -rf $DIR
  [ "$USERID" = "*" ] && rm -fv /data/adb/magisk.db*
}

db_init() {
  # Temporary let the folder rw by anyone
  chcon u:object_r:magisk_file:s0 /data/adb
  chmod 777 /data/adb
}

db_restore() {
  chmod 700 /data/adb
  magisk --restorecon
}

db_setup() {
  local USER=$1
  local USERID=$(($USER / 100000))
  local DIR=/sbin/.core/db-${USERID}
  mkdir -p $DIR
  touch $DIR/magisk.db
  mount -o bind /data/adb/magisk.db $DIR/magisk.db
  rm -f /data/adb/magisk.db-*
  chcon u:object_r:magisk_file:s0 $DIR $DIR/*
  chmod 700 $DIR
  chown $USER.$USER $DIR
  chmod 666 $DIR/*
}

env_check() {
  for file in busybox magisk magiskboot magiskinit util_functions.sh boot_patch.sh; do
    [ -f /data/adb/magisk/$file ] || return 1
  done
  return 0
}

mm_patch_dtbo() {
  if $KEEPVERITY; then
    echo false
  else
    find_dtbo_image
    patch_dtbo_image >/dev/null 2>&1 && echo true || echo false
  fi
}

restore_imgs() {
  SHA1=`cat /.backup/.sha1`
  [ -z $SHA1 ] && SHA1=`grep_prop #STOCKSHA1`
  [ -z $SHA1 ] && return 1
  STOCKBOOT=/data/stock_boot_${SHA1}.img.gz
  STOCKDTBO=/data/stock_dtbo.img.gz
  [ -f $STOCKBOOT ] || return 1

  find_boot_image
  find_dtbo_image

  magisk --unlock-blocks 2>/dev/null
  if [ -b "$DTBOIMAGE" -a -f $STOCKDTBO ]; then
    gzip -d < $STOCKDTBO > $DTBOIMAGE
  fi
  if [ -b "$BOOTIMAGE" -a -f $STOCKBOOT ]; then
    gzip -d < $STOCKBOOT | cat - /dev/zero > $BOOTIMAGE 2>/dev/null
    return 0
  fi
  return 1
}
