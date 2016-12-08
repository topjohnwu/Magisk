#!/system/bin/sh

LOGFILE=/cache/magisk.log
MODDIR=${0%/*}

log_print() {
  echo $1
  echo "phh: $1" >> $LOGFILE
  log -p i -t phh "$1"
}

launch_daemonsu() {
  # Revert to original path, legacy issue
  case $PATH in
    *busybox* )
      export PATH=$OLDPATH
      ;;
  esac
  # Switch contexts
  echo "u:r:su_daemon:s0" > /proc/self/attr/current
  # Start daemon
  exec /sbin/su --daemon
}

# Disable the other root
[ -d "/magisk/zzsupersu" ] && touch /magisk/zzsupersu/disable

log_print "Live patching sepolicy"
$MODDIR/bin/sepolicy-inject --live

# Expose the root path
log_print "Mounting supath"
rm -rf /magisk/.core/bin $MODDIR/sbin_bind
mkdir -p $MODDIR/sbin_bind 
/data/busybox/cp -afc /sbin/. $MODDIR/sbin_bind
chmod 755 $MODDIR/sbin_bind
ln -s $MODDIR/bin/* $MODDIR/sbin_bind
mount -o bind $MODDIR/sbin_bind /sbin

# Run su.d
for script in $MODDIR/su.d/* ; do
  if [ -f "$script" ]; then
    chmod 755 $script
    log_print "su.d: $script"
    sh $script
  fi
done

log_print "Starting su daemon"
(launch_daemonsu &)
