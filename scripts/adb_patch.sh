#####################################################################
#   ADB Magisk Boot Image Patcher
#####################################################################
#
# With a device accessible via ADB, usage:
# ./build.py patch path/to/boot.img
#
#####################################################################

if [ ! -f /system/build.prop ]; then
  # Running on PC
  echo 'Please run `./build.py patch` instead of directly executing the script!'
  exit 1
fi

cd /data/local/tmp
chmod 755 busybox magisk*

if [ -z "$FIRST_STAGE" ]; then
  export FIRST_STAGE=1
  export ASH_STANDALONE=1
  # Re-exec script with busybox
  exec ./busybox sh $0 "$@"
fi

TARGET_FILE="$1"
OUTPUT_FILE="$1.magisk"

# Fetch env flags using non-root functions in app_functions
. ./app_functions.sh
mount_partitions
get_flags

# Patch the boot image to the target path
sh boot_patch.sh $TARGET_FILE
mv new-boot.img $OUTPUT_FILE
./magiskboot cleanup
