#!/system/bin/sh

BINDIR=/magisk/.core/bin

# Find where's su binary
for DIR in /magisk /data/magisk /cache/magisk /su/bin; do
  [ -f "$DIR/su" ] && break
done

case $1 in
  phhsu )
    if [ ! -d "/dev/su" ]; then
      $DIR/sepolicy-inject --live --auto -s su
      exec $DIR/su --daemon
    fi
    ;;
  post-fs )
    # If su call fails, temporary switch to permissive (workaround)
    # This workaround will not always work (e.g. Samsung stock boot images)
    if [ `$DIR/su -c "/sbin/magic_mask.sh post-fs" >/dev/null 2>&1; echo $?` -ne 0 ]; then
      echo 0 > /sys/fs/selinux/enforce
      /sbin/magic_mask.sh post-fs
      echo 1 > /sys/fs/selinux/enforce
    fi
    ;;
  post-fs-data )
    # su call shall always work
    if [ `$DIR/su -c "/sbin/magic_mask.sh post-fs-data" >/dev/null 2>&1; echo $?` -ne 0 ]; then
      /sbin/magic_mask.sh post-fs-data
    fi
    ;;
  service )
    # su call shall always work
    if [ `$DIR/su -c "/sbin/magic_mask.sh service" >/dev/null 2>&1; echo $?` -ne 0 ]; then
      /sbin/magic_mask.sh service
    fi
    ;;
  root )
    # This will only be used in phh root
    $DIR/su -c "/sbin/magic_mask.sh root"
    ;;
esac