#!/system/bin/sh
##########################################################################################
#
# Magisk Boot Image Patcher
# by topjohnwu
#
# This script should be placed in a directory with the following files:
#
# File name       type      Description
#
# boot_patch.sh   script    A script to patch boot. Expect path to boot image as parameter.
#               (this file) The script will use binaries and files in its same directory
#                           to complete the patching process
# monogisk        binary    The monolithic binary to replace /init
# magiskboot      binary    A tool to unpack boot image, decompress ramdisk, extract ramdisk
#                           , and patch the ramdisk for Magisk support
# chromeos        folder    This folder should store all the utilities and keys to sign
#               (optional)  a chromeos device, used in the tablet Pixel C
#
# If the script is not running as root, then the input boot image should be a stock image
# or have a backup included in ramdisk internally, since we cannot access the stock boot
# image placed under /data we've created when previously installed
#
##########################################################################################
##########################################################################################
# Functions
##########################################################################################

# Pure bash dirname implementation
dirname_wrap() {
  case "$1" in
    */*)
      dir=${1%/*}
      [ -z $dir ] && echo "/" || echo $dir
      ;;
    *)
      echo "."
      ;;
  esac
}

# Pure bash basename implementation
basename_wrap() {
  echo ${1##*/}
}

##########################################################################################
# Initialization
##########################################################################################

if [ -z $SOURCEDMODE ]; then
  # Switch to the location of the script file
  cd "`dirname_wrap "${BASH_SOURCE:-$0}"`"
  # Load utility functions
  . ./util_functions.sh
fi

BOOTIMAGE="$1"

[ -e "$BOOTIMAGE" ] || abort "$BOOTIMAGE does not exist!"

# Presets
[ -z $KEEPVERITY ] && KEEPVERITY=false
[ -z $HIGHCOMP ] && HIGHCOMP=false

if [ -z $KEEPFORCEENCRYPT ]; then
  if [ "`getprop ro.crypto.state`" = "encrypted" ]; then
    KEEPFORCEENCRYPT=true
    ui_print "- Encrypted data detected, keep forceencrypt"
  else
    KEEPFORCEENCRYPT=false
  fi
fi

chmod -R 755 .

# Extract magisk if doesn't exist
[ -e magisk ] || ./magiskinit -x magisk magisk

##########################################################################################
# Unpack
##########################################################################################

CHROMEOS=false

ui_print "- Unpacking boot image"
./magiskboot --unpack "$BOOTIMAGE"

case $? in
  1 )
    abort "! Unable to unpack boot image"
    ;;
  2 )
    ui_print "! Insufficient boot partition size detected"
    HIGHCOMP=true
    ui_print "- Enable high compression mode"
    ;;
  3 )
    ui_print "- ChromeOS boot image detected"
    CHROMEOS=true
    ;;
  4 )
    ui_print "! Sony ELF32 format detected"
    abort "! Please use BootBridge from @AdrianDC to flash Magisk"
    ;;
  5 )
    ui_print "! Sony ELF64 format detected"
    abort "! Stock kernel cannot be patched, please use a custom kernel"
esac

##########################################################################################
# Ramdisk restores
##########################################################################################

# Test patch status and do restore, after this section, ramdisk.cpio.orig is guaranteed to exist
ui_print "- Checking ramdisk status"
./magiskboot --cpio ramdisk.cpio test
case $? in
  0 )  # Stock boot
    ui_print "- Stock boot image detected"
    ui_print "- Backing up stock boot image"
    SHA1=`./magiskboot --sha1 "$BOOTIMAGE" 2>/dev/null`
    STOCKDUMP=stock_boot_${SHA1}.img.gz
    ./magiskboot --compress "$BOOTIMAGE" $STOCKDUMP
    cp -af ramdisk.cpio ramdisk.cpio.orig
    ;;
  1 )  # Magisk patched
    ui_print "- Magisk patched image detected"
    # Find SHA1 of stock boot image
    [ -z $SHA1 ] && SHA1=`./magiskboot --cpio ramdisk.cpio sha1 2>/dev/null`
    ./magiskboot --cpio ramdisk.cpio restore
    cp -af ramdisk.cpio ramdisk.cpio.orig
    ;;
  2 ) # Other patched
    ui_print "! Boot image patched by other programs"
    abort "! Please restore stock boot image"
    ;;
esac

##########################################################################################
# Ramdisk patches
##########################################################################################

ui_print "- Patching ramdisk"

./magiskboot --cpio ramdisk.cpio \
'add 750 init magiskinit' \
"magisk ramdisk.cpio.orig $HIGHCOMP $KEEPVERITY $KEEPFORCEENCRYPT $SHA1"

rm -f ramdisk.cpio.orig

##########################################################################################
# Binary patches
##########################################################################################

if ! $KEEPVERITY && [ -f dtb ]; then
  ./magiskboot --dtb-patch dtb && ui_print "- Patching fstab in dtb to remove dm-verity"
fi

if [ -f kernel ]; then
  # Remove Samsung RKP in stock kernel
  ./magiskboot --hexpatch kernel \
  49010054011440B93FA00F71E9000054010840B93FA00F7189000054001840B91FA00F7188010054 \
  A1020054011440B93FA00F7140020054010840B93FA00F71E0010054001840B91FA00F7181010054

  # skip_initramfs -> want_initramfs
  ./magiskboot --hexpatch kernel \
  736B69705F696E697472616D6673 \
  77616E745F696E697472616D6673
fi

##########################################################################################
# Repack and flash
##########################################################################################

ui_print "- Repacking boot image"
./magiskboot --repack "$BOOTIMAGE" || abort "! Unable to repack boot image!"

# Sign chromeos boot
$CHROMEOS && sign_chromeos

./magiskboot --cleanup
