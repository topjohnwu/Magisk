#!/system/bin/sh

LOGFILE=/cache/magisk.log

log_print() {
  echo $1
  echo "phh: $1" >> $LOGFILE
  log -p i -t phh "$1"
}

launch_daemonsu() {
  export PATH=$OLDPATH
  # Switch contexts
  echo "u:r:su_daemon:s0" > /proc/self/attr/current
  # Start daemon
  exec /magisk/phh/bin/su --daemon
}

# Disable the other root
[ -d "/magisk/zzsupersu" ] && touch /magisk/zzsupersu/disable

log_print "Live patching sepolicy"
/magisk/phh/bin/sepolicy-inject --live

# Expose the root path
log_print "Linking supath"
rm -rf /magisk/.core/bin
ln -s /magisk/phh/bin /magisk/.core/bin

# Run su.d
for script in /magisk/phh/su.d/* ; do
  if [ -f "$script" ]; then
    chmod 755 $script
    log_print "su.d: $script"
    $script
  fi
done

log_print "Starting su daemon"
(launch_daemonsu &)
