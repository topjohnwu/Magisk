#MAGISK
##########################################################################################
#
# Magisk Uninstaller
# by topjohnwu
#
##########################################################################################

##########################################################################################
# Preparation
##########################################################################################

# This path should work in any cases
TMPDIR=/dev/tmp

INSTALLER=$TMPDIR/install
CHROMEDIR=$INSTALLER/chromeos

# Default permissions
umask 022

OUTFD=$2
ZIP=$3

if [ ! -f $INSTALLER/util_functions.sh ]; then
  echo "! Unable to extract zip file!"
  exit 1
fi

# Load utility functions
. $INSTALLER/util_functions.sh

setup_flashable

ui_print "************************"
ui_print "   Magisk Uninstaller   "
ui_print "************************"

is_mounted /data || mount /data || abort "! Unable to mount partitions"
is_mounted /cache || mount /cache 2>/dev/null
mount_partitions

api_level_arch_detect

ui_print "- Device platform: $ARCH"
MAGISKBIN=$INSTALLER/$ARCH32
mv $CHROMEDIR $MAGISKBIN
chmod -R 755 $MAGISKBIN

check_data
$DATA_DE || abort "! Cannot access /data, please uninstall with Magisk Manager"
$BOOTMODE || recovery_actions

##########################################################################################
# Uninstall
##########################################################################################

find_boot_image
find_dtbo_image

[ -e $BOOTIMAGE ] || abort "! Unable to detect boot image"
ui_print "- Found target image: $BOOTIMAGE"
[ -z $DTBOIMAGE ] || ui_print "- Found dtbo image: $DTBOIMAGE"

cd $MAGISKBIN

CHROMEOS=false

ui_print "- Unpacking boot image"
./magiskboot --unpack "$BOOTIMAGE"

case $? in
  1 )
    abort "! Unable to unpack boot image"
    ;;
  2 )
    ui_print "- ChromeOS boot image detected"
    CHROMEOS=true
    ;;
  3 )
    ui_print "! Sony ELF32 format detected"
    abort "! Please use BootBridge from @AdrianDC"
    ;;
  4 )
    ui_print "! Sony ELF64 format detected"
    abort "! Stock kernel cannot be patched, please use a custom kernel"
esac

# Detect boot image state
ui_print "- Checking ramdisk status"
./magiskboot --cpio ramdisk.cpio test
case $? in
  0 )  # Stock boot
    ui_print "- Stock boot image detected"
    ;;
  1 )  # Magisk patched
    ui_print "- Magisk patched image detected"
    # Find SHA1 of stock boot image
    [ -z $SHA1 ] && SHA1=`./magiskboot --cpio ramdisk.cpio sha1 2>/dev/null`
    STOCKBOOT=/data/stock_boot_${SHA1}.img.gz
    STOCKDTBO=/data/stock_dtbo.img.gz
    if [ -f $STOCKBOOT ]; then
      ui_print "- Restoring stock boot image"
      flash_image $STOCKBOOT $BOOTIMAGE
      if [ -f $STOCKDTBO -a -b "$DTBOIMAGE" ]; then
        ui_print "- Restoring stock dtbo image"
        flash_image $STOCKDTBO $DTBOIMAGE
      fi
    else
      ui_print "! Boot image backup unavailable"
      ui_print "- Restoring ramdisk with internal backup"
      ./magiskboot --cpio ramdisk.cpio restore
      ./magiskboot --repack $BOOTIMAGE
      # Sign chromeos boot
      $CHROMEOS && sign_chromeos
      ui_print "- Flashing restored boot image"
      flash_image new-boot.img $BOOTIMAGE || abort "! Insufficient partition size"
    fi
    ;;
  2 ) # Other patched
    ui_print "! Boot image patched by other programs"
    abort "! Cannot uninstall"
    ;;
esac

ui_print "- Removing Magisk files"
rm -rf  /cache/*magisk* /cache/unblock /data/*magisk* /data/cache/*magisk* /data/property/*magisk* \
        /data/Magisk.apk /data/busybox /data/custom_ramdisk_patch.sh /data/adb/*magisk* \
        /data/adb/post-fs-data.d /data/adb/service.d /data/adb/modules* 2>/dev/null

if [ -f /system/addon.d/99-magisk.sh ]; then
  mount -o rw,remount /system
  rm -f /system/addon.d/99-magisk.sh
fi

cd /

if $BOOTMODE; then
  ui_print "**********************************************"
  ui_print "* Magisk Manager will uninstall itself, and  *"
  ui_print "* the device will reboot after a few seconds *"
  ui_print "**********************************************"
  (sleep 8; /system/bin/reboot)&
else
  rm -rf /data/user*/*/*magisk* /data/app/*magisk*
  recovery_cleanup
  ui_print "- Done"
fi

rm -rf $TMPDIR
exit 0
