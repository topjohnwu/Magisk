#!/sbin/sh

#################
# Initialization
#################

umask 022

# echo before loading util_functions
ui_print() { echo "$1"; }

require_new_magisk() {
  ui_print "*******************************"
  ui_print " Please install Magisk v19.0+! "
  ui_print "*******************************"
  exit 1
}

#########################
# Load util_functions.sh
#########################

OUTFD=$2
ZIPFILE=$3

mount /data 2>/dev/null

[ -f /data/adb/magisk/util_functions.sh ] || require_new_magisk
. /data/adb/magisk/util_functions.sh
[ $MAGISK_VER_CODE -lt 19000 ] && require_new_magisk

if [ $MAGISK_VER_CODE -ge 20400 ]; then
  # New Magisk have complete installation logic within util_functions.sh
  install_module
  exit 0
fi

#################
# Legacy Support
#################

TMPDIR=/dev/tmp
PERSISTDIR=/sbin/.magisk/mirror/persist

is_legacy_script() {
  unzip -l "$ZIPFILE" install.sh | grep -q install.sh
  return $?
}

print_modname() {
  local len
  len=`echo -n $MODNAME | wc -c`
  len=$((len + 2))
  local pounds=`printf "%${len}s" | tr ' ' '*'`
  ui_print "$pounds"
  ui_print " $MODNAME "
  ui_print "$pounds"
  ui_print "*******************"
  ui_print " Powered by Magisk "
  ui_print "*******************"
}

# Override abort as old scripts have some issues
abort() {
  ui_print "$1"
  $BOOTMODE || recovery_cleanup
  [ -n $MODPATH ] && rm -rf $MODPATH
  rm -rf $TMPDIR
  exit 1
}

rm -rf $TMPDIR 2>/dev/null
mkdir -p $TMPDIR

# Preperation for flashable zips
setup_flashable

# Mount partitions
mount_partitions

# Detect version and architecture
api_level_arch_detect

# Setup busybox and binaries
$BOOTMODE && boot_actions || recovery_actions

##############
# Preparation
##############

# Extract prop file
unzip -o "$ZIPFILE" module.prop -d $TMPDIR >&2
[ ! -f $TMPDIR/module.prop ] && abort "! Unable to extract zip file!"

$BOOTMODE && MODDIRNAME=modules_update || MODDIRNAME=modules
MODULEROOT=$NVBASE/$MODDIRNAME
MODID=`grep_prop id $TMPDIR/module.prop`
MODPATH=$MODULEROOT/$MODID
MODNAME=`grep_prop name $TMPDIR/module.prop`

# Create mod paths
rm -rf $MODPATH 2>/dev/null
mkdir -p $MODPATH

##########
# Install
##########

if is_legacy_script; then
  unzip -oj "$ZIPFILE" module.prop install.sh uninstall.sh 'common/*' -d $TMPDIR >&2

  # Load install script
  . $TMPDIR/install.sh

  # Callbacks
  print_modname
  on_install

  # Custom uninstaller
  [ -f $TMPDIR/uninstall.sh ] && cp -af $TMPDIR/uninstall.sh $MODPATH/uninstall.sh

  # Skip mount
  $SKIPMOUNT && touch $MODPATH/skip_mount

  # prop file
  $PROPFILE && cp -af $TMPDIR/system.prop $MODPATH/system.prop

  # Module info
  cp -af $TMPDIR/module.prop $MODPATH/module.prop

  # post-fs-data scripts
  $POSTFSDATA && cp -af $TMPDIR/post-fs-data.sh $MODPATH/post-fs-data.sh

  # service scripts
  $LATESTARTSERVICE && cp -af $TMPDIR/service.sh $MODPATH/service.sh

  ui_print "- Setting permissions"
  set_permissions
else
  print_modname

  unzip -o "$ZIPFILE" customize.sh -d $MODPATH >&2

  if ! grep -q '^SKIPUNZIP=1$' $MODPATH/customize.sh 2>/dev/null; then
    ui_print "- Extracting module files"
    unzip -o "$ZIPFILE" -x 'META-INF/*' -d $MODPATH >&2

    # Default permissions
    set_perm_recursive $MODPATH 0 0 0755 0644
  fi

  # Load customization script
  [ -f $MODPATH/customize.sh ] && . $MODPATH/customize.sh
fi

# Handle replace folders
for TARGET in $REPLACE; do
  ui_print "- Replace target: $TARGET"
  mktouch $MODPATH$TARGET/.replace
done

if $BOOTMODE; then
  # Update info for Magisk Manager
  mktouch $NVBASE/modules/$MODID/update
  cp -af $MODPATH/module.prop $NVBASE/modules/$MODID/module.prop
fi

# Copy over custom sepolicy rules
if [ -f $MODPATH/sepolicy.rule -a -e $PERSISTDIR ]; then
  ui_print "- Installing custom sepolicy patch"
  PERSISTMOD=$PERSISTDIR/magisk/$MODID
  mkdir -p $PERSISTMOD
  cp -af $MODPATH/sepolicy.rule $PERSISTMOD/sepolicy.rule
fi

# Remove stuffs that don't belong to modules
rm -rf \
$MODPATH/system/placeholder $MODPATH/customize.sh \
$MODPATH/README.md $MODPATH/.git* 2>/dev/null

#############
# Finalizing
#############

cd /
$BOOTMODE || recovery_cleanup
rm -rf $TMPDIR

ui_print "- Done"
exit 0
