sudb_clean() {
  local USERID=$1
  local DIR="/sbin/.core/db-${USERID}"
  umount -l /data/user*/*/*/databases/su.db $DIR $DIR/*
  rm -rf $DIR
}

sudb_setup() {
  local USER=$1
  local GLOBAL_DB=$2
  local USERID=$(($USER / 100000))
  local DIR=/sbin/.core/db-${USERID}
  touch $GLOBAL_DB
  mkdir -p $DIR
  touch $DIR/magisk.db
  mount -o bind $GLOBAL_DB $DIR/magisk.db
  chcon u:object_r:su_file:s0 $DIR $DIR/*
  chmod 700 $DIR
  chown $USER.$USER $DIR
  chmod 666 $DIR/*
}
