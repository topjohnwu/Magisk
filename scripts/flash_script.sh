#MAGISK
############################################
#
# Magisk Flash Script (updater-script)
# by topjohnwu
#
############################################

############################################
# Preparation
############################################

COMMONDIR=$INSTALLER/common
APK=$COMMONDIR/magisk.apk
CHROMEDIR=$INSTALLER/chromeos

# Default permissions
umask 022

OUTFD=$2
ZIP=$3

if [ ! -f $COMMONDIR/util_functions.sh ]; then
  echo "! Unable to extract zip file!"
  exit 1
fi

# Load utility fuctions
. $COMMONDIR/util_functions.sh

setup_flashable "$@"

############################################
# Detection
############################################

PRETTY_VER=$MAGISK_VER
echo $PRETTY_VER | grep -q '\.' && PRETTY_VER=v$PRETTY_VER
print_title "Magisk $PRETTY_VER Installer"

is_mounted /data || mount /data || is_mounted /cache || mount /cache
mount_partitions
check_data
get_flags
find_boot_image

[ -z $BOOTIMAGE ] && abort "! Unable to detect target image"
ui_print "- Target image: $BOOTIMAGE"

# Detect version and architecture
api_level_arch_detect

[ $API -lt 17 ] && abort "! Magisk only support Android 4.2 and above"

ui_print "- Device platform: $ARCH"

BINDIR=$INSTALLER/$ARCH32
chmod -R 755 $CHROMEDIR $BINDIR

# Check if system root is installed and remove
remove_system_su

############################################
# Environment
############################################

ui_print "- Constructing environment"

# Copy required files
rm -rf $MAGISKBIN/* 2>/dev/null
mkdir -p $MAGISKBIN 2>/dev/null
cp -af $BINDIR/. $COMMONDIR/. $CHROMEDIR $BBBIN $MAGISKBIN
chmod -R 755 $MAGISKBIN

# addon.d
if [ -d /system/addon.d ]; then
  ui_print "- Adding addon.d survival script"
  mount -o rw,remount /system
  ADDOND=/system/addon.d/99-magisk.sh
  cp -af $COMMONDIR/addon.d.sh $ADDOND
  chmod 755 $ADDOND
fi

$BOOTMODE || recovery_actions

############################################
# Boot/DTBO Patching
############################################

install_magisk

# Cleanups
$BOOTMODE || recovery_cleanup
rm -rf $TMPDIR

ui_print "- Done"
exit 0
