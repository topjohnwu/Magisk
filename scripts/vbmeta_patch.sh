#!/system/bin/sh
#######################################################################################
# Magisk Vbmeta Image Patcher
#######################################################################################
#
# Usage: vbmeta_patch.sh <vbmetaimage>
#
# This script should be placed in a directory with the following files:
#
# File name          Type      Description
#
# vbmeta_patch.sh    script    A script to patch vbmeta image for Magisk.
#                  (this file) The script will use files in its same
#                              directory to complete the patching process
# boot_patch.sh      script    A script to patch boot image for Magisk.
#                              The script will use files in its same
#                              directory to complete the patching process
# util_functions.sh  script    A script which hosts all functions required
#                              for this script to work properly
# magiskinit         binary    The binary to replace /init
# magisk(32/64)      binary    The magisk binaries
# magiskboot         binary    A tool to manipulate boot images
# magiskvbmeta       binary    A tool to manipulate vbmeta images
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

VBMETAIMAGE="$1"
[ -e "$VBMETAIMAGE" ] || ui_print "- vbmeta image $VBMETAIMAGE does not exist!"

chmod -R 755 .

########
# Check
########

# Test patch status
ui_print "- Checking vbmeta status"
./magiskvbmeta test "$VBMETAIMAGE"
case $? in
  0 )  # Stock vbmeta
    ui_print "- Stock vbmeta image detected"
    SIZE=$(./magiskvbmeta size "$VBMETAIMAGE" 2>/dev/null)
    dd if=$VBMETAIMAGE of=stock_vbmeta.img bs=1 count=$SIZE 2>/dev/null
    ;;
  1 )  # Magisk patched
    ui_print "! Patched vbmeta image detected"
    rm -f stock_vbmeta.img
    abort "! Please restore back to stock vbmeta image"
    ;;
  2 )  # Other patched
    ui_print "! Unexpected vbmeta image flags detected"
    abort "! Please restore back to stock vbmeta image"
    ;;
  4 )  # Invalid
    abort "! Invalid vbmeta image detected"
    ;;
esac

########
# Patch
########

ui_print "- Patching vbmeta"
./magiskvbmeta patch "$VBMETAIMAGE"

########
# Flash
########

# Reset any error code
true
