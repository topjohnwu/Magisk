#!/system/bin/sh
##########################################################################################
#
# Magisk Boot Image Patcher
# by topjohnwu
# 
# This script should be placed in a directory with at least the following files:
# 
# File name       type      Description
# 
# boot_patch.sh   script    A script to patch boot. Expect path to boot image as parameter.
#               (this file) The script will use binaries and files in its same directory
#                           to complete the patching process
# magisk          binary    The main binary for all Magisk operations.
#                           It is also used to patch the sepolicy in the ramdisk.
# magiskboot      binary    A tool to unpack boot image, decompress ramdisk, extract ramdisk
#                           and patch common patches such as forceencrypt, remove dm-verity.
# init.magisk.rc  script    A new line will be added to init.rc to import this script.
#                           All magisk entrypoints are defined here
# 
# If the script is not running as root, then the input boot image should be a stock image
# or have a backup included in ramdisk internally, since we cannot access the stock boot
# image placed under /data we've created when previously installing
#
##########################################################################################

CWD=`pwd`
cd `dirname $1`
BOOTIMAGE="`pwd`/`basename $1`"
cd "$CWD"

if [ -z $BOOTIMAGE ]; then
  ui_print_wrap "This script requires a boot image as a parameter"
  exit 1
fi

# Presets
[ -z $KEEPVERITY ] && KEEPVERITY=false
[ -z $KEEPFORCEENCRYPT ] && KEEPFORCEENCRYPT=false

# Detect whether running as root
[ `id -u` -eq 0 ] && ROOT=true || ROOT=false

# Call ui_print_wrap if exists, or else simply use echo
# Useful when wrapped in flashable zip
ui_print_wrap() {
  type ui_print >/dev/null 2>&1 && ui_print "$1" || echo "$1"
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
  LD_LIBRARY_PATH=$SYSTEMLIB ./magiskboot --cpio-add ramdisk.cpio $1 $2 $3
}

# --cpio-extract <incpio> <entry> <outfile>
cpio_extract() {
  LD_LIBRARY_PATH=$SYSTEMLIB ./magiskboot --cpio-extract ramdisk.cpio $1 $2
}

# --cpio-mkdir <incpio> <mode> <entry>
cpio_mkdir() {
  LD_LIBRARY_PATH=$SYSTEMLIB ./magiskboot --cpio-mkdir ramdisk.cpio $1 $2
}

##########################################################################################
# Prework
##########################################################################################

# Switch to the location of the script file
[ -z $SOURCEDMODE ] && cd "`dirname "${BASH_SOURCE:-$0}"`"
chmod +x ./*

# Detect ARCH
[ -d /system/lib64 ] && SYSTEMLIB=/system/lib64 || SYSTEMLIB=/system/lib

ui_print_wrap "- Unpacking boot image"
LD_LIBRARY_PATH=$SYSTEMLIB ./magiskboot --unpack $BOOTIMAGE

case $? in
  1 )
    ui_print_wrap "! Unable to unpack boot image"
    exit 1
    ;;
  2 )
    ui_print_wrap "! Sony ELF32 format detected"
    ui_print_wrap "! Please use BootBridge from @AdrianDC to flash Magisk"
    exit 1
    ;;
  3 )
    ui_print_wrap "! Sony ELF64 format detected"
    ui_print_wrap "! Stock kernel cannot be patched, please use a custom kernel"
    exit 1
esac

##########################################################################################
# Ramdisk restores
##########################################################################################

# Test patch status and do restore, after this section, ramdisk.cpio.orig is guaranteed to exist
ui_print_wrap "- Checking ramdisk status"
LD_LIBRARY_PATH=$SYSTEMLIB ./magiskboot --cpio-test ramdisk.cpio
case $? in
  0 )  # Stock boot
    ui_print_wrap "- Stock boot image detected!"
    ui_print_wrap "- Backing up stock boot image"
    SHA1=`LD_LIBRARY_PATH=$SYSTEMLIB ./magiskboot --sha1 $BOOTIMAGE | tail -n 1`
    STOCKDUMP=stock_boot_${SHA1}.img
    dd if=$BOOTIMAGE of=$STOCKDUMP
    LD_LIBRARY_PATH=$SYSTEMLIB ./magiskboot --compress $STOCKDUMP
    cp -af ramdisk.cpio ramdisk.cpio.orig
    ;;
  1 )  # Magisk patched
    ui_print_wrap "- Magisk patched image detected!"
    # Find SHA1 of stock boot image
    if [ -z $SHA1 ]; then
      LD_LIBRARY_PATH=$SYSTEMLIB ./magiskboot --cpio-extract ramdisk.cpio init.magisk.rc init.magisk.rc.old
      SHA1=`grep_prop "# STOCKSHA1" init.magisk.rc.old`
      rm -f init.magisk.rc.old
    fi

    OK=false
    LD_LIBRARY_PATH=$SYSTEMLIB ./magiskboot --cpio-restore ramdisk.cpio
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
          LD_LIBRARY_PATH=$SYSTEMLIB ./magiskboot --decompress ${STOCKDUMP}.gz stock_boot.img
          LD_LIBRARY_PATH=$SYSTEMLIB ./magiskboot --unpack stock_boot.img
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
    ui_print_wrap "! Please restore stock boot image"
    exit 1
    ;;
esac

##########################################################################################
# Ramdisk patches
##########################################################################################

ui_print_wrap "- Patching ramdisk"

# The common patches
$KEEPVERITY || LD_LIBRARY_PATH=$SYSTEMLIB ./magiskboot --cpio-patch-dmverity ramdisk.cpio
$KEEPFORCEENCRYPT || LD_LIBRARY_PATH=$SYSTEMLIB ./magiskboot --cpio-patch-forceencrypt ramdisk.cpio

# Add magisk entrypoint
cpio_extract init.rc init.rc
grep "import /init.magisk.rc" init.rc >/dev/null || sed -i '1,/.*import.*/s/.*import.*/import \/init.magisk.rc\n&/' init.rc
sed -i "/selinux.reload_policy/d" init.rc
cpio_add 750 init.rc init.rc
rm -f init.rc

# sepolicy patches
cpio_extract sepolicy sepolicy
LD_LIBRARY_PATH=$SYSTEMLIB ./magisk magiskpolicy --load sepolicy --save sepolicy --minimal
cpio_add 644 sepolicy sepolicy
rm -f sepolicy

# Add new items
if [ ! -z $SHA1 ]; then
  cp init.magisk.rc init.magisk.rc.bak
  echo "# STOCKSHA1=$SHA1" >> init.magisk.rc
fi
cpio_add 750 init.magisk.rc init.magisk.rc
[ -f init.magisk.rc.bak ] && mv init.magisk.rc.bak init.magisk.rc
cpio_add 755 sbin/magisk magisk

# Create ramdisk backups
LD_LIBRARY_PATH=$SYSTEMLIB ./magiskboot --cpio-backup ramdisk.cpio ramdisk.cpio.orig

rm -f ramdisk.cpio.orig

##########################################################################################
# Repack and flash
##########################################################################################

# Hexpatches

# Remove Samsung RKP in stock kernel
LD_LIBRARY_PATH=$SYSTEMLIB ./magiskboot --hexpatch kernel \
49010054011440B93FA00F71E9000054010840B93FA00F7189000054001840B91FA00F7188010054 \
A1020054011440B93FA00F7140020054010840B93FA00F71E0010054001840B91FA00F7181010054

ui_print_wrap "- Repacking boot image"
LD_LIBRARY_PATH=$SYSTEMLIB ./magiskboot --repack $BOOTIMAGE

if [ $? -ne 0 ]; then
  ui_print_wrap "! Unable to repack boot image!"
  exit 1
fi

LD_LIBRARY_PATH=$SYSTEMLIB ./magiskboot --cleanup
