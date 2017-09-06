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
# magisk          binary    The main binary for all Magisk operations.
#                           It is also used to patch the sepolicy in the ramdisk.
# magiskboot      binary    A tool to unpack boot image, decompress ramdisk, extract ramdisk
#                           , and patch the ramdisk for Magisk support
# init.magisk.rc  script    A new line will be added to init.rc to import this script.
#                           All magisk entrypoints are defined here
# chromeos        folder    This folder should store all the utilities and keys to sign
#               (optional)  a chromeos device, used in the tablet Pixel C
#
# If the script is not running as root, then the input boot image should be a stock image
# or have a backup included in ramdisk internally, since we cannot access the stock boot
# image placed under /data we've created when previously installing
#
##########################################################################################
##########################################################################################
# Functions
##########################################################################################

# Call ui_print_wrap if exists, or else simply use echo
# Useful when wrapped in flashable zip
ui_print_wrap() {
  type ui_print >/dev/null 2>&1 && ui_print "$1" || echo "$1"
}

# Call abort if exists, or else show error message and exit
# Essential when wrapped in flashable zip
abort_wrap() {
  type abort >/dev/null 2>&1
  if [ $? -ne 0 ]; then
    ui_print_wrap "$1"
    exit 1
  else
    abort "$1"
  fi
}

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

grep_prop() {
  REGEX="s/^$1=//p"
  shift
  FILES=$@
  if [ -z "$FILES" ]; then
    FILES='/system/build.prop'
  fi
  cat $FILES 2>/dev/null | sed -n "$REGEX" | head -n 1
}

# --cpio-add <incpio> <mode> <entry> <infile>
cpio_add() {
  ./magiskboot --cpio-add ramdisk.cpio $1 $2 $3
}

# --cpio-extract <incpio> <entry> <outfile>
cpio_extract() {
  ./magiskboot --cpio-extract ramdisk.cpio $1 $2
}

# --cpio-mkdir <incpio> <mode> <entry>
cpio_mkdir() {
  ./magiskboot --cpio-mkdir ramdisk.cpio $1 $2
}

##########################################################################################
# Initialization
##########################################################################################

[ -z $1 ] && abort_wrap "This script requires a boot image as a parameter"

cwd=`pwd`
cd "`dirname_wrap $1`"
BOOTIMAGE="`pwd`/`basename_wrap $1`"
cd $cwd

[ -e "$BOOTIMAGE" ] || abort_wrap "$BOOTIMAGE does not exist!"

# Presets
[ -z $KEEPVERITY ] && KEEPVERITY=false
[ -z $KEEPFORCEENCRYPT ] && KEEPFORCEENCRYPT=false

# Detect whether running as root
id | grep "uid=0" >/dev/null 2>&1 && ROOT=true || ROOT=false

# Switch to the location of the script file
[ -z $SOURCEDMODE ] && cd "`dirname_wrap "${BASH_SOURCE:-$0}"`"
chmod -R 755 .

##########################################################################################
# Unpack
##########################################################################################

migrate_boot_backup

CHROMEOS=false

ui_print_wrap "- Unpacking boot image"
./magiskboot --unpack "$BOOTIMAGE"

case $? in
  1 )
    abort_wrap "! Unable to unpack boot image"
    ;;
  2 )
    CHROMEOS=true
    ;;
  3 )
    ui_print_wrap "! Sony ELF32 format detected"
    abort_wrap "! Please use BootBridge from @AdrianDC to flash Magisk"
    ;;
  4 )
    ui_print_wrap "! Sony ELF64 format detected"
    abort_wrap "! Stock kernel cannot be patched, please use a custom kernel"
esac

##########################################################################################
# Ramdisk restores
##########################################################################################

# Test patch status and do restore, after this section, ramdisk.cpio.orig is guaranteed to exist
ui_print_wrap "- Checking ramdisk status"
./magiskboot --cpio-test ramdisk.cpio
case $? in
  0 )  # Stock boot
    ui_print_wrap "- Stock boot image detected!"
    ui_print_wrap "- Backing up stock boot image"
    SHA1=`./magiskboot --sha1 "$BOOTIMAGE" 2>/dev/null`
    STOCKDUMP=stock_boot_${SHA1}.img
    dd if="$BOOTIMAGE" of=$STOCKDUMP
    ./magiskboot --compress $STOCKDUMP
    cp -af ramdisk.cpio ramdisk.cpio.orig
    ;;
  1 )  # Magisk patched
    ui_print_wrap "- Magisk patched image detected!"
    # Find SHA1 of stock boot image
    [ -z $SHA1 ] && SHA1=`./magiskboot --cpio-stocksha1 ramdisk.cpio 2>/dev/null`
    OK=false
    ./magiskboot --cpio-restore ramdisk.cpio
    if [ $? -eq 0 ]; then
      ui_print_wrap "- Ramdisk restored from internal backup"
      OK=true
    else
      # Restore failed
      ui_print_wrap "! Cannot restore from internal backup"
      # If we are root and SHA1 known, we try to find the stock backup
      if $ROOT && [ ! -z $SHA1 ]; then
        STOCKDUMP=/data/stock_boot_${SHA1}.img
        if [ -f ${STOCKDUMP}.gz ]; then
          ui_print_wrap "- Stock boot image backup found"
          ./magiskboot --decompress ${STOCKDUMP}.gz stock_boot.img
          ./magiskboot --unpack stock_boot.img
          rm -f stock_boot.img
          OK=true
        fi
      fi
    fi
    if ! $OK; then
      ui_print_wrap "! Ramdisk restoration incomplete"
      ui_print_wrap "! Will still try to continue installation"
    fi
    cp -af ramdisk.cpio ramdisk.cpio.orig
    ;;
  2 ) # Other patched
    ui_print_wrap "! Boot image patched by other programs!"
    abort_wrap "! Please restore stock boot image"
    ;;
esac

##########################################################################################
# Ramdisk patches
##########################################################################################

ui_print_wrap "- Patching ramdisk"

# Add magisk entrypoint
./magiskboot --cpio-patch ramdisk.cpio $KEEPVERITY $KEEPFORCEENCRYPT

# sepolicy patches
cpio_extract sepolicy sepolicy
./magisk magiskpolicy --load sepolicy --save sepolicy --minimal
cpio_add 644 sepolicy sepolicy
rm -f sepolicy

# Add new items
if [ ! -z $SHA1 ]; then
  cp init.magisk.rc init.magisk.rc.bak
  echo "# STOCKSHA1=$SHA1" >> init.magisk.rc
fi
cpio_add 750 init.magisk.rc init.magisk.rc
mv init.magisk.rc.bak init.magisk.rc 2>/dev/null
cpio_add 755 sbin/magisk magisk

# Create ramdisk backups
./magiskboot --cpio-backup ramdisk.cpio ramdisk.cpio.orig

rm -f ramdisk.cpio.orig

##########################################################################################
# Repack and flash
##########################################################################################

# Hexpatches

# Remove Samsung RKP in stock kernel
./magiskboot --hexpatch kernel \
49010054011440B93FA00F71E9000054010840B93FA00F7189000054001840B91FA00F7188010054 \
A1020054011440B93FA00F7140020054010840B93FA00F71E0010054001840B91FA00F7181010054

ui_print_wrap "- Repacking boot image"
./magiskboot --repack "$BOOTIMAGE" || abort_wrap "! Unable to repack boot image!"

# Sign chromeos boot
$CHROMEOS && sign_chromeos

./magiskboot --cleanup
