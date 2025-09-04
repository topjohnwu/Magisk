#!/system/bin/sh
#######################################################################################
# Magisk Boot Image Patcher
#######################################################################################
#
# Usage: boot_patch.sh <bootimage>
#
# The following environment variables can configure the installation:
# KEEPVERITY, KEEPFORCEENCRYPT, PATCHVBMETAFLAG, RECOVERYMODE, LEGACYSAR
#
# This script should be placed in a directory with the following files:
#
# File name          Type      Description
#
# boot_patch.sh      script    A script to patch boot image for Magisk.
#                  (this file) The script will use files in its same
#                              directory to complete the patching process.
# util_functions.sh  script    A script which hosts all functions required
#                              for this script to work properly.
# magiskinit         binary    The binary to replace /init.
# magisk             binary    The magisk binary.
# magiskboot         binary    A tool to manipulate boot images.
# init-ld            binary    The library that will be LD_PRELOAD of /init
# stub.apk           binary    The stub Magisk app to embed into ramdisk.
# chromeos           folder    This folder includes the utility and keys to sign
#                  (optional)  chromeos boot images. Only used for Pixel C.
#
#######################################################################################

############
# Functions
############

# Pure bash dirname implementation
getdir() {
  case "$1" in
    */*)
      dir=${1%/*}
      if [ -z $dir ]; then
        echo "/"
      else
        echo $dir
      fi
    ;;
    *) echo "." ;;
  esac
}

#################
# Initialization
#################

if [ -z $SOURCEDMODE ]; then
  # Switch to the location of the script file
  cd "$(getdir "${BASH_SOURCE:-$0}")"
  # Load utility functions
  . ./util_functions.sh
  # Check if 64-bit
  api_level_arch_detect
fi

BOOTIMAGE="$1"
[ -e "$BOOTIMAGE" ] || abort "$BOOTIMAGE does not exist!"

# Dump image for MTD/NAND character device boot partitions
if [ -c "$BOOTIMAGE" ]; then
  nanddump -f boot.img "$BOOTIMAGE"
  BOOTNAND="$BOOTIMAGE"
  BOOTIMAGE=boot.img
fi

# Flags
[ -z $KEEPVERITY ] && KEEPVERITY=false
[ -z $KEEPFORCEENCRYPT ] && KEEPFORCEENCRYPT=false
[ -z $PATCHVBMETAFLAG ] && PATCHVBMETAFLAG=false
[ -z $RECOVERYMODE ] && RECOVERYMODE=false
[ -z $LEGACYSAR ] && LEGACYSAR=false
export KEEPVERITY
export KEEPFORCEENCRYPT
export PATCHVBMETAFLAG

chmod -R 755 .

#########
# Unpack
#########

CHROMEOS=false

ui_print "- Unpacking boot image"
./magiskboot unpack "$BOOTIMAGE"

case $? in
  0 ) ;;
  1 )
    abort "! Unsupported/Unknown image format"
    ;;
  2 )
    ui_print "- ChromeOS boot image detected"
    CHROMEOS=true
    ;;
  * )
    abort "! Unable to unpack boot image"
    ;;
esac

###################
# Ramdisk Restores
###################

# Test patch status and do restore
ui_print "- Checking ramdisk status"
for path in "./ramdisk.cpio" "./vendor_ramdisk/init_boot.cpio" "./vendor_ramdisk/ramdisk.cpio"; do
  if [ -e "$path" ]; then
    CPIO_PATH="${path#./}"
    ./magiskboot cpio "$CPIO_PATH" test
    STATUS=$?
    SKIP_BACKUP=""
    break
  fi
done

if [ -z "$CPIO_PATH" ]; then
  # Stock A only legacy SAR, or some Android 13 GKIs
  CPIO_PATH="ramdisk.cpio"
  STATUS=0
  SKIP_BACKUP="#"
fi
case $STATUS in
  0 )
    # Stock boot
    ui_print "- Stock boot image detected"
    SHA1=$(./magiskboot sha1 "$BOOTIMAGE" 2>/dev/null)
    cat $BOOTIMAGE > stock_boot.img
    cp -af "$CPIO_PATH" "$CPIO_PATH.orig" 2>/dev/null
    ;;
  1 )
    # Magisk patched
    ui_print "- Magisk patched boot image detected"
    ./magiskboot cpio $CPIO_PATH \
    "extract .backup/.magisk config.orig" \
    "restore"
    cp -af "$CPIO_PATH" "$CPIO_PATH.orig"
    rm -f stock_boot.img
    ;;
  2 )
    # Unsupported
    ui_print "! Boot image patched by unsupported programs"
    abort "! Please restore back to stock boot image"
    ;;
esac

if [ -f config.orig ]; then
  # Read existing configs
  chmod 0644 config.orig
  SHA1=$(grep_prop SHA1 config.orig)
  if ! $BOOTMODE; then
    # Do not inherit config if not in recovery
    PREINITDEVICE=$(grep_prop PREINITDEVICE config.orig)
  fi
  rm config.orig
fi

##################
# Ramdisk Patches
##################

ui_print "- Patching ramdisk"

$BOOTMODE && [ -z "$PREINITDEVICE" ] && PREINITDEVICE=$(./magisk --preinit-device)

# Compress to save precious ramdisk space
./magiskboot compress=xz magisk magisk.xz
./magiskboot compress=xz stub.apk stub.xz
./magiskboot compress=xz init-ld init-ld.xz

echo "KEEPVERITY=$KEEPVERITY" > config
echo "KEEPFORCEENCRYPT=$KEEPFORCEENCRYPT" >> config
echo "RECOVERYMODE=$RECOVERYMODE" >> config
if [ -n "$PREINITDEVICE" ]; then
  ui_print "- Pre-init storage partition: $PREINITDEVICE"
  echo "PREINITDEVICE=$PREINITDEVICE" >> config
fi
[ -n "$SHA1" ] && echo "SHA1=$SHA1" >> config

./magiskboot cpio "$CPIO_PATH" \
"add 0750 init magiskinit" \
"mkdir 0750 overlay.d" \
"mkdir 0750 overlay.d/sbin" \
"add 0644 overlay.d/sbin/magisk.xz magisk.xz" \
"add 0644 overlay.d/sbin/stub.xz stub.xz" \
"add 0644 overlay.d/sbin/init-ld.xz init-ld.xz" \
"patch" \
"$SKIP_BACKUP backup $CPIO_PATH.orig" \
"mkdir 000 .backup" \
"add 000 .backup/.magisk config" \
|| abort "! Unable to patch ramdisk"

rm -f $CPIO_PATH.orig config *.xz

#################
# Binary Patches
#################

for dt in dtb kernel_dtb extra; do
  if [ -f $dt ]; then
    if ! ./magiskboot dtb $dt test; then
      ui_print "! Boot image $dt was patched by old (unsupported) Magisk"
      abort "! Please try again with *unpatched* boot image"
    fi
    if ./magiskboot dtb $dt patch; then
      ui_print "- Patch fstab in boot image $dt"
    fi
  fi
done

if [ -f kernel ]; then
  PATCHEDKERNEL=false
  # Remove Samsung RKP
  ./magiskboot hexpatch kernel \
  49010054011440B93FA00F71E9000054010840B93FA00F7189000054001840B91FA00F7188010054 \
  A1020054011440B93FA00F7140020054010840B93FA00F71E0010054001840B91FA00F7181010054 \
  && PATCHEDKERNEL=true

  # Remove Samsung defex
  # Before: [mov w2, #-221]   (-__NR_execve)
  # After:  [mov w2, #-32768]
  ./magiskboot hexpatch kernel 821B8012 E2FF8F12 && PATCHEDKERNEL=true

  # Disable Samsung PROCA
  # proca_config -> proca_magisk
  ./magiskboot hexpatch kernel \
  70726F63615F636F6E66696700 \
  70726F63615F6D616769736B00 \
  && PATCHEDKERNEL=true

  # Force kernel to load rootfs for legacy SAR devices
  # skip_initramfs -> want_initramfs
  $LEGACYSAR && ./magiskboot hexpatch kernel \
  736B69705F696E697472616D667300 \
  77616E745F696E697472616D667300 \
  && PATCHEDKERNEL=true

  # If the kernel doesn't need to be patched at all,
  # keep raw kernel to avoid bootloops on some weird devices
  $PATCHEDKERNEL || rm -f kernel
fi

#################
# Repack & Flash
#################

ui_print "- Repacking boot image"
./magiskboot repack "$BOOTIMAGE" || abort "! Unable to repack boot image"

# Sign chromeos boot
$CHROMEOS && sign_chromeos

# Restore the original boot partition path
[ -e "$BOOTNAND" ] && BOOTIMAGE="$BOOTNAND"

# Reset any error code
true
