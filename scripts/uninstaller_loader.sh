#!/sbin/sh
##########################################################################################
#
# Magisk Uninstaller (used in recovery)
# by topjohnwu
# 
# This script will load the real uninstaller in a flashable zip
#
##########################################################################################

INSTALLER=/tmp/uninstall

# Default permissions
umask 022

##########################################################################################
# Flashable update-binary preparation
##########################################################################################

OUTFD=$2
ZIP=$3

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

mkdir -p $INSTALLER
cd $INSTALLER
unzip -o "$ZIP"

##########################################################################################
# Functions
##########################################################################################

ui_print() {
  echo -n -e "ui_print $1\n" >> /proc/self/fd/$OUTFD
  echo -n -e "ui_print\n" >> /proc/self/fd/$OUTFD
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
    FILES='/system/build.prop'
  fi
  cat $FILES 2>/dev/null | sed -n $REGEX | head -n 1
}

##########################################################################################
# Main
##########################################################################################

ui_print "*****************************"
ui_print "      Magisk Uninstaller     "
ui_print "*****************************"

if [ ! -d "$INSTALLER/arm" ]; then
  ui_print "! Failed: Unable to extract zip file!"
  exit 1
fi

ui_print "- Mounting /system(ro), /cache, /data"
mount -o ro /system 2>/dev/null
mount /cache 2>/dev/null
mount /data 2>/dev/null

if [ ! -f '/system/build.prop' ]; then
  ui_print "! Failed: /system could not be mounted!"
  exit 1
fi

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

ui_print "- Device platform: $ARCH"
CHROMEDIR=$INSTALLER/chromeos
BINDIR=$INSTALLER/$ARCH

##########################################################################################
# Detection all done, start installing
##########################################################################################

if (is_mounted /data); then
  # Copy the binaries to /data/magisk, in case they do not exist
  rm -rf /data/magisk 2>/dev/null
  mkdir -p /data/magisk 2>/dev/null
  cp -af $BINDIR/. $CHROMEDIR /data/magisk
  . $INSTALLER/magisk_uninstaller.sh
else
  ui_print "! Data unavailable"
  ui_print "! Placing uninstall script to /cache"
  ui_print "! The device might reboot multiple times"
  cp -af $INSTALLER/magisk_uninstaller.sh /cache/magisk_uninstaller.sh
  umount /system
  ui_print "- Rebooting....."
  sleep 5
  reboot
fi

umount /system
ui_print "- Done"
exit 0
