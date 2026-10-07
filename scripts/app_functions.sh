##################################
# Magisk app internal scripts
##################################

# $1 = delay
# $2 = command
run_delay() {
  (sleep $1; $2)&
}

# $1 = version string
# $2 = version code
env_check() {
  for file in busybox magiskboot magiskinit util_functions.sh boot_patch.sh; do
    [ -f "$MAGISKBIN/$file" ] || return 1
  done
  if [ "$2" -ge 25000 ]; then
    [ -f "$MAGISKBIN/magiskpolicy" ] || return 1
  fi
  if [ "$2" -ge 25210 ]; then
    [ -b "$MAGISKTMP/.magisk/device/preinit" ] || [ -b "$MAGISKTMP/.magisk/block/preinit" ] || return 2
  fi
  grep -xqF "MAGISK_VER='$1'" "$MAGISKBIN/util_functions.sh" || return 3
  grep -xqF "MAGISK_VER_CODE=$2" "$MAGISKBIN/util_functions.sh" || return 3
  return 0
}

# $1 = dir to copy
# $2 = destination (optional)
cp_readlink() {
  if [ -z $2 ]; then
    cd $1
  else
    cp -af $1/. $2
    cd $2
  fi
  for file in *; do
    if [ -L $file ]; then
      local full=$(readlink -f $file)
      rm $file
      cp -af $full $file
    fi
  done
  chmod -R 755 .
  cd /
}

# $1 = install dir
fix_env() {
  # Cleanup and make dirs
  rm -rf $MAGISKBIN/*
  mkdir -p $MAGISKBIN 2>/dev/null
  chmod 700 /data/adb
  cp_readlink $1 $MAGISKBIN
  rm -rf $1
  chown -R 0:0 $MAGISKBIN
}

# $1 = install dir
# $2 = boot partition
direct_install() {
  echo "- Flashing new boot image"
  flash_image $1/new-boot.img $2
  case $? in
    1)
      echo "! Insufficient partition size"
      return 1
      ;;
    2)
      echo "! $2 is read only"
      return 2
      ;;
  esac

  rm -f $1/new-boot.img
  fix_env $1
  run_migrations

  return 0
}

# $1 = uninstaller zip
run_uninstaller() {
  rm -rf /dev/tmp
  mkdir -p /dev/tmp/install
  unzip -o "$1" "assets/*" "lib/*" -d /dev/tmp/install
  INSTALLER=/dev/tmp/install sh /dev/tmp/install/assets/uninstaller.sh dummy 1 "$1"
}

# $1 = boot partition
restore_imgs() {
  local SHA1=$(grep_prop SHA1 $MAGISKTMP/.magisk/config)
  local BACKUPDIR=/data/magisk_backup_$SHA1
  [ -d $BACKUPDIR ] || return 1
  [ -f $BACKUPDIR/boot.img.gz ] || return 1
  flash_image $BACKUPDIR/boot.img.gz $1
}

post_ota() {
  cd /data/adb
  cp -f $MAGISKBIN/bootctl bootctl
  rm -f $MAGISKBIN/bootctl
  chmod 755 bootctl
  if ! ./bootctl hal-info; then
    rm -f bootctl
    return
  fi
  SLOT_NUM=0
  [ $(./bootctl get-current-slot) -eq 0 ] && SLOT_NUM=1
  ./bootctl set-active-boot-slot $SLOT_NUM
  cat << EOF > post-fs-data.d/post_ota.sh
/data/adb/bootctl mark-boot-successful
rm -f /data/adb/bootctl
rm -f /data/adb/post-fs-data.d/post_ota.sh
EOF
  chmod 755 post-fs-data.d/post_ota.sh
  cd /
}

# $1 = APK
# $2 = package name
adb_pm_install() {
  local tmp=/data/local/tmp/temp.apk
  cp -f "$1" $tmp
  chmod 644 $tmp
  su 2000 -c pm install -g $tmp || pm install -g $tmp || su 1000 -c pm install -g $tmp
  local res=$?
  rm -f $tmp
  if [ $res = 0 ]; then
    appops set "$2" REQUEST_INSTALL_PACKAGES allow
  fi
  return $res
}

check_boot_ramdisk() {
  # Create boolean ISAB
  ISAB=true
  [ -z $SLOT ] && ISAB=false

  # If we are A/B, then we must have ramdisk
  $ISAB && return 0

  # If we are using legacy SAR, but not A/B, assume we do not have ramdisk
  if $LEGACYSAR; then
    # Override recovery mode to true
    RECOVERYMODE=true
    return 1
  fi

  return 0
}

check_encryption() {
  if $ISENCRYPTED; then
    if [ $SDK_INT -lt 24 ]; then
      CRYPTOTYPE="block"
    else
      # First see what the system tells us
      CRYPTOTYPE=$(getprop ro.crypto.type)
      if [ -z $CRYPTOTYPE ]; then
        # If not mounting through device mapper, we are FBE
        if grep ' /data ' /proc/mounts | grep -qv 'dm-'; then
          CRYPTOTYPE="file"
        else
          # We are either FDE or metadata encryption (which is also FBE)
          CRYPTOTYPE="block"
          grep -q ' /metadata ' /proc/mounts && CRYPTOTYPE="file"
        fi
      fi
    fi
  else
    CRYPTOTYPE="N/A"
  fi
}

printvar() {
  eval echo $1=\$$1
}

run_action() {
  local MODID="$1"
  cd "/data/adb/modules/$MODID"
  sh ./action.sh
  local RES=$?
  cd /
  return $RES
}

# Patch the vendor_boot image of Aluminium OS recovery images to
# disable data wipe and bypass AVB hash verification of boot partitions.
# All temporary files are always removed regardless of the result.
# $1 = install dir
# $2 = vendor_boot image (absolute path), patched in-place
patch_al_vendor_boot() {
  cd "$1" || return 1

  # Run in a subshell so errors can exit early without skipping cleanup
  (
  IMG=$2
  CPIO=vendor_ramdisk/ramdisk.cpio
  PS=system/bin/partition-script.sh
  CLI=system/bin/avb_verify_cli

  # Return value 3 indicates a vendor_boot image
  ./magiskboot unpack "$IMG"
  if [ $? -ne 3 ]; then
    echo "! Invalid vendor_boot image"
    exit 1
  fi

  if [ ! -f $CPIO ]; then
    echo "! Unable to find vendor ramdisk"
    exit 1
  fi

  if ./magiskboot cpio $CPIO "exists $CLI.real"; then
    echo "! vendor_boot image is already patched"
    exit 1
  fi

  if ! ./magiskboot cpio $CPIO "exists $PS" || ! ./magiskboot cpio $CPIO "exists $CLI"; then
    echo "! Unsupported vendor ramdisk"
    exit 1
  fi

  # Disable userdata wipe and TPM clear in partition-script.sh
  echo "- Patching $PS"
  ./magiskboot cpio $CPIO "extract $PS partition-script.sh"
  sed -e 's/^\([[:space:]]*\)write_base_table()[[:space:]]*{/\1orig_write_base_table() {/' \
  -e 's/^\([[:space:]]*\)load_base_vars()[[:space:]]*{/\1orig_load_base_vars() {/' \
  partition-script.sh > partition-script.sh.new
  if ! grep -q 'orig_write_base_table() {' partition-script.sh.new || ! grep -q 'orig_load_base_vars() {' partition-script.sh.new; then
    echo "! Unable to patch $PS"
    exit 1
  fi
  cat << 'EOF' >> partition-script.sh.new
# ==============================================================================
# Custom partition-script.sh Overrides (Disable Data Wipe)
# ==============================================================================

write_base_table() {
  orig_write_base_table "$@"
}

load_base_vars() {
  orig_load_base_vars

  # recovery_media aborts if PARTITION_NUM_USERDATA or PARTITION_NUM_METADATA
  # are unset, and normally formats metadata (mke2fs) and zeroes the first 2 MiB
  # of userdata. Redirect both variables to the unused ota_recovery_b partition
  # so those wipes hit ota_recovery_b, leaving the real userdata and metadata
  # (which stores the FBE encryption key) untouched.
  PARTITION_NUM_USERDATA="${PARTITION_NUM_OTA_RECOVERY_B}"
  PARTITION_NUM_METADATA="${PARTITION_NUM_OTA_RECOVERY_B}"

  # Unset PARTITION_NUM_PERSIST so its partition number in INSTALLABLE_PARTITIONS
  # is not resolved by name_map and is skipped instead of being overwritten with
  # factory persist.img.
  unset PARTITION_NUM_PERSIST

  # Unset security partition numbers so the post-install security reset phase
  # skips zeroing desktop_security_persist and desktop_security_storage.
  unset PARTITION_NUM_DESKTOP_SECURITY_PERSIST
  unset PARTITION_NUM_DESKTOP_SECURITY_STORAGE

  # Hold /dev/tpm0 open in the background. The Linux TPM character driver
  # enforces single-open exclusivity (-EBUSY), preventing recovery_media from
  # clearing the TPM or resetting Trusty MACs (treated as a non-fatal warning)
  # and preserving hardware-backed FBE encryption keys.
  sleep 86400 </dev/tpm0 >/dev/null 2>&1 &
}
EOF

  # Collect all cpio commands and run them in one go
  set -- "add 0755 $PS partition-script.sh.new"

  # Remove pinned recovery media public keys
  for KEY in system vendor; do
    KEY=$KEY/etc/security/avb/recovery_media_public_key.bin
    if ./magiskboot cpio $CPIO "exists $KEY"; then
      echo "- Removing $KEY"
      set -- "$@" "rm $KEY"
    fi
  done

  # Wrap avb_verify_cli to drop the trusted public key argument and
  # strip all Hash descriptors from its output, so boot partitions are
  # flashed without verification and vbmeta is left untouched
  echo "- Wrapping $CLI"
  cat << 'EOF' > avb_verify_cli
#!/system/bin/sh
out="$(/system/bin/avb_verify_cli.real "$1" "$2")" || exit $?
printf '%s\n' "$out" | sed -E 's/\{"Hash":\{[^}]*\}\},?//g; s/,\]/]/g'
EOF

  set -- "$@" "mv $CLI $CLI.real" "add 0755 $CLI avb_verify_cli"
  if ! ./magiskboot cpio $CPIO "$@"; then
    echo "! Unable to patch vendor ramdisk"
    exit 1
  fi

  echo "- Repacking vendor_boot image"
  if ! ./magiskboot repack "$IMG" new-vendor_boot.img; then
    echo "! Unable to repack vendor_boot image"
    exit 1
  fi
  cat new-vendor_boot.img > "$IMG"
  )
  local RES=$?

  ./magiskboot cleanup
  rm -f new-vendor_boot.img partition-script.sh partition-script.sh.new avb_verify_cli
  cd /
  return $RES
}

##########################
# Non-root util_functions
##########################

mount_partitions() {
  [ "$(getprop ro.build.ab_update)" = "true" ] && SLOT=$(getprop ro.boot.slot_suffix)
  # Check whether non rootfs root dir exists
  SYSTEM_AS_ROOT=false
  grep ' / ' /proc/mounts | grep -qv 'rootfs' && SYSTEM_AS_ROOT=true

  LEGACYSAR=false
  grep ' / ' /proc/mounts | grep -q '/dev/root' && LEGACYSAR=true
}

get_flags() {
  KEEPVERITY=$SYSTEM_AS_ROOT
  ISENCRYPTED=false
  [ "$(getprop ro.crypto.state)" = "encrypted" ] && ISENCRYPTED=true
  KEEPFORCEENCRYPT=$ISENCRYPTED
  if [ -n "$(getprop ro.boot.vbmeta.device)" -o -n "$(getprop ro.boot.vbmeta.size)" ]; then
    PATCHVBMETAFLAG=false
  elif getprop ro.product.ab_ota_partitions | grep -wq vbmeta; then
    PATCHVBMETAFLAG=false
  else
    PATCHVBMETAFLAG=true
  fi
  [ -z $RECOVERYMODE ] && RECOVERYMODE=false
  [ -z $VENDORBOOT ] && VENDORBOOT=false
}

run_migrations() { return; }

grep_prop() { return; }

#############
# Initialize
#############

app_init() {
  mount_partitions >/dev/null
  RAMDISKEXIST=false
  check_boot_ramdisk && RAMDISKEXIST=true
  get_flags >/dev/null
  run_migrations >/dev/null
  check_encryption

  # Dump variables
  printvar SLOT
  printvar SYSTEM_AS_ROOT
  printvar RAMDISKEXIST
  printvar ISAB
  printvar CRYPTOTYPE
  printvar PATCHVBMETAFLAG
  printvar LEGACYSAR
  printvar RECOVERYMODE
  printvar KEEPVERITY
  printvar KEEPFORCEENCRYPT
  printvar VENDORBOOT
}

export BOOTMODE=true
