#!/sbin/sh

TMPDIR=/dev/tmp
MOUNTPATH=/dev/magisk_img

# Default permissions
umask 022

# Initial cleanup
rm -rf $TMPDIR 2>/dev/null
mkdir -p $TMPDIR

# echo before loading util_functions
ui_print() { echo "$1"; }

require_new_magisk() {
  ui_print "***********************************"
  ui_print " Please install the latest Magisk! "
  ui_print "***********************************"
  exit 1
}

imageless_magisk() {
  [ $MAGISK_VER_CODE -gt 18100 ]
  return $?
}

##########################################################################################
# Environment
##########################################################################################

OUTFD=$2
ZIPFILE=$3

mount /data 2>/dev/null

# Load utility functions
if [ -f /data/adb/magisk/util_functions.sh ]; then
  . /data/adb/magisk/util_functions.sh
  NVBASE=/data/adb
else
  require_new_magisk
fi

# Preperation for flashable zips
setup_flashable

# Mount partitions
mount_partitions

# Detect version and architecture
api_level_arch_detect

# Setup busybox and binaries
$BOOTMODE && boot_actions || recovery_actions

##########################################################################################
# Preparation
##########################################################################################

# Extract common files
unzip -oj "$ZIPFILE" module.prop install.sh uninstall.sh 'common/*' -d $TMPDIR >&2

[ ! -f $TMPDIR/install.sh ] && abort "! Unable to extract zip file!"
# Load install script
. $TMPDIR/install.sh

if imageless_magisk; then
  $BOOTMODE && MODDIRNAME=modules_update || MODDIRNAME=modules
  MODULEROOT=$NVBASE/$MODDIRNAME
else
  $BOOTMODE && IMGNAME=magisk_merge.img || IMGNAME=magisk.img
  IMG=$NVBASE/$IMGNAME
  request_zip_size_check "$ZIPFILE"
  mount_magisk_img
  MODULEROOT=$MOUNTPATH
fi

MODID=`grep_prop id $TMPDIR/module.prop`
MODPATH=$MODULEROOT/$MODID

print_modname

ui_print "******************************"
ui_print "Powered by Magisk (@topjohnwu)"
ui_print "******************************"

##########################################################################################
# Install
##########################################################################################

# Create mod paths
rm -rf $MODPATH 2>/dev/null
mkdir -p $MODPATH

on_install

# Remove placeholder
rm -f $MODPATH/system/placeholder 2>/dev/null

# Custom uninstaller
[ -f $TMPDIR/uninstall.sh ] && cp -af $TMPDIR/uninstall.sh $MODPATH/uninstall.sh

# Auto Mount
if imageless_magisk; then
  $SKIPMOUNT && touch $MODPATH/skip_mount
else
  $SKIPMOUNT || touch $MODPATH/auto_mount
fi

# prop files
$PROPFILE && cp -af $TMPDIR/system.prop $MODPATH/system.prop

# Module info
cp -af $TMPDIR/module.prop $MODPATH/module.prop
if $BOOTMODE; then
  # Update info for Magisk Manager
  if imageless_magisk; then
    mktouch $NVBASE/modules/$MODID/update
    cp -af $TMPDIR/module.prop $NVBASE/modules/$MODID/module.prop
  else
    mktouch /sbin/.magisk/img/$MODID/update
    cp -af $TMPDIR/module.prop /sbin/.magisk/img/$MODID/module.prop
  fi
fi

# post-fs-data mode scripts
$POSTFSDATA && cp -af $TMPDIR/post-fs-data.sh $MODPATH/post-fs-data.sh

# service mode scripts
$LATESTARTSERVICE && cp -af $TMPDIR/service.sh $MODPATH/service.sh

# Handle replace folders
for TARGET in $REPLACE; do
  mktouch $MODPATH$TARGET/.replace
done

ui_print "- Setting permissions"
set_permissions

##########################################################################################
# Finalizing
##########################################################################################

cd /
imageless_magisk || unmount_magisk_img
$BOOTMODE || recovery_cleanup
rm -rf $TMPDIR $MOUNTPATH

ui_print "- Done"
exit 0
