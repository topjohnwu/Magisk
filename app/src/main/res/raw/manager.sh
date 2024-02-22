##################################
# Magisk app internal scripts
##################################

run_delay() {
  (sleep $1; $2)&
}

env_check() {
  for file in busybox magiskboot magiskinit util_functions.sh boot_patch.sh; do
    [ -f "$MAGISKBIN/$file" ] || return 1
  done
  if [ "$2" -ge 25000 ]; then
    [ -f "$MAGISKBIN/magiskpolicy" ] || return 1
  fi
  if [ "$2" -ge 25210 ] && [ -f "$MAGISKTMP/.magisk/config" ]; then
    [ -b "$MAGISKTMP/.magisk/block/preinit" ] || return 2
  fi
  grep -xqF "MAGISK_VER='$1'" "$MAGISKBIN/util_functions.sh" || return 3
  grep -xqF "MAGISK_VER_CODE=$2" "$MAGISKBIN/util_functions.sh" || return 3
  return 0
}

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

fix_env() {
  # Cleanup and make dirs
  rm -rf $MAGISKBIN/*
  mkdir -p $MAGISKBIN 2>/dev/null
  chmod 700 $NVBASE
  cp_readlink $1 $MAGISKBIN
  rm -rf $1
  chown -R 0:0 $MAGISKBIN
}

install_addond(){
    local installDir="$MAGISKBIN"
    local AppApkPath="$1"
    local SYSTEM_INSTALL="$2"
    [ -z "$SYSTEM_INSTALL" ] && SYSTEM_INSTALL=false
    addond=/system/addon.d
    test ! -d $addond && return
    ui_print "- Adding addon.d survival script"
    BLOCKNAME="/dev/block/system_block.$(random_str 5 20)"
    rm -rf "$BLOCKNAME"
    if is_rootfs; then
        mkblknode "$BLOCKNAME" /system
    else
        mkblknode "$BLOCKNAME"  /
    fi
    blockdev --setrw "$BLOCKNAME"
    rm -rf "$BLOCKNAME"
    mount -o rw,remount /
    mount -o rw,remount /system
    rm -rf $addond/99-magisk.sh 2>/dev/null
    rm -rf $addond/magisk 2>/dev/null
    if [ "$SYSTEM_INSTALL" == "true" ]; then
        cp -prLf "$installDir"/. /system/etc/init/magisk || { ui_print "! Failed to install addon.d"; return; }
        mv "$installDir/addon.d.sh" $addond/99-magisk.sh
        cp "$AppApkPath" /system/etc/init/magisk/magisk.apk
        chmod 755 /system/etc/init/magisk/*
        sed -i "s/^SYSTEMINSTALL=.*/SYSTEMINSTALL=true/g" $addond/99-magisk.sh
    else
        mkdir -p $addond/magisk
        cp -prLf "$installDir"/. $addond/magisk || { ui_print "! Failed to install addon.d"; return; }
        mv $addond/magisk/boot_patch.sh $addond/magisk/boot_patch.sh.in
        mv $addond/magisk/addon.d.sh $addond/99-magisk.sh
        cp "$AppApkPath" $addond/magisk/magisk.apk
    fi
    mount -o ro,remount /
    mount -o ro,remount /system
}

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
  copy_preinit_files
  install_addond "$3"
  return 0
}

run_uninstaller() {
  rm -rf /dev/tmp
  mkdir -p /dev/tmp/install
  unzip -o "$1" "assets/*" "lib/*" -d /dev/tmp/install
  INSTALLER=/dev/tmp/install sh /dev/tmp/install/assets/uninstaller.sh dummy 1 "$1"
}

restore_imgs() {
  [ -z $SHA1 ] && return 1
  local BACKUPDIR=/data/magisk_backup_$SHA1
  [ -d $BACKUPDIR ] || return 1

  get_flags
  find_boot_image

  for name in dtb dtbo; do
    [ -f $BACKUPDIR/${name}.img.gz ] || continue
    local IMAGE=$(find_block $name$SLOT)
    [ -z $IMAGE ] && continue
    flash_image $BACKUPDIR/${name}.img.gz $IMAGE
  done
  [ -f $BACKUPDIR/boot.img.gz ] || return 1
  flash_image $BACKUPDIR/boot.img.gz $BOOTIMAGE
}

post_ota() {
  cd $NVBASE
  cp -f $1 bootctl
  rm -f $1
  chmod 755 bootctl
  ./bootctl hal-info || return
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

add_hosts_module() {
  # Do not touch existing hosts module
  [ -d $NVBASE/modules/hosts ] && return
  cd $NVBASE/modules
  mkdir -p hosts/system/etc
  cat << EOF > hosts/module.prop
id=hosts
name=Systemless Hosts
version=1.0
versionCode=1
author=Magisk
description=Magisk app built-in systemless hosts module
EOF
  magisk --clone /system/etc/hosts hosts/system/etc/hosts
  touch hosts/update
  cd /
}

adb_pm_install() {
  local tmp=/data/local/tmp/temp.apk
  cp -f "$1" $tmp
  chmod 644 $tmp
  su 2000 -c pm install -g $tmp || pm install -g $tmp || su 1000 -c pm install -g $tmp
  local res=$?
  rm -f $tmp
  if [ $res = 0 ]; then
    ( magisk magiskhide sulist && magisk magiskhide add "$2" ) &
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
}

run_migrations() { return; }

grep_prop() { return; }

get_sulist_status(){
    SULISTMODE=false
    if magisk magiskhide sulist; then
        SULISTMODE=true
    fi
}

##############################
# Magisk Delta Custom install script
##############################

# define
MAGISKSYSTEMDIR="/system/etc/init/magisk"

random_str(){
local FROM
local TO
FROM="$1"; TO="$2"
tr -dc A-Za-z0-9 </dev/urandom | head -c $(($FROM+$(($RANDOM%$(($TO-$FROM+1))))))
}

magiskrc(){
local MAGISKTMP="$1"

# use "magisk --auto-selinux" to automatically switching selinux state

cat <<EOF
on post-fs-data
    start logd
    exec u:r:su:s0 root root -- $MAGISKSYSTEMDIR/magiskpolicy --live --magisk
    exec u:r:magisk:s0 root root -- $MAGISKSYSTEMDIR/magiskpolicy --live --magisk
    exec u:r:update_engine:s0 root root -- $MAGISKSYSTEMDIR/magiskpolicy --live --magisk
    exec u:r:su:s0 root root -- $MAGISKSYSTEMDIR/$magisk_name --auto-selinux --setup-sbin $MAGISKSYSTEMDIR $MAGISKTMP
    exec u:r:su:s0 root root -- $MAGISKTMP/magisk --auto-selinux --post-fs-data
on nonencrypted
    exec u:r:su:s0 root root -- $MAGISKTMP/magisk --auto-selinux --service
on property:vold.decrypt=trigger_restart_framework
    exec u:r:su:s0 root root -- $MAGISKTMP/magisk --auto-selinux --service
on property:sys.boot_completed=1
    mkdir /data/adb/magisk 755
    exec u:r:su:s0 root root -- $MAGISKTMP/magisk --auto-selinux --boot-complete
   
on property:init.svc.zygote=restarting
    exec u:r:su:s0 root root -- $MAGISKTMP/magisk --auto-selinux --zygote-restart
   
on property:init.svc.zygote=stopped
    exec u:r:su:s0 root root -- $MAGISKTMP/magisk --auto-selinux --zygote-restart
EOF
}

remount_check(){
    local mode="$1"
    local part="$(realpath "$2")"
    local ignore_not_exist="$3"
    local i
    if ! grep -q " $part " /proc/mounts && [ ! -z "$ignore_not_exist" ]; then
        return "$ignore_not_exist"
    fi
    mount -o "$mode,remount" "$part"
    local IFS=$'\t\n ,'
    for i in $(cat /proc/mounts | grep " $part " | awk '{ print $4 }'); do
        test "$i" == "$mode" && return 0
    done
    return 1
}

backup_restore(){
    # if gz is not found and orig file is found, backup to gz
    if [ ! -f "${1}.gz" ] && [ -f "$1" ]; then
        gzip -k "$1" && return 0
    elif [ -f "${1}.gz" ]; then
    # if gz found, restore from gz
        rm -rf "$1" && gzip -kdf "${1}.gz" && return 0
    fi
    return 1
}

restore_from_bak(){
    backup_restore "$1" && rm -rf "${1}.gz"
}

cleanup_system_installation(){
    rm -rf "$MIRRORDIR${MAGISKSYSTEMDIR}"
    rm -rf "$MIRRORDIR${MAGISKSYSTEMDIR}.rc"
    backup_restore "$MIRRORDIR/system/etc/init/bootanim.rc" \
    && rm -rf "$MIRRORDIR/system/etc/init/bootanim.rc.gz"
    if [ -e "$MIRRORDIR${MAGISKSYSTEMDIR}" ] || [ -e "$MIRRORDIR${MAGISKSYSTEMDIR}.rc" ]; then
        return 1
    fi
}

installer_cleanup(){
    if $BOOTMODE; then
        umount -l "/proc/$$/attr"
    else
        recovery_cleanup
    fi
    mount -o ro,remount /
}

direct_install_system(){
    print_title "Progisk (System Mode)" "by electrondefuser, HuskyDG & Dr TSNG"
    print_title "Powered by Magisk"
    api_level_arch_detect
    local INSTALLDIR="$1"

    ui_print "- Remount system partition as read-write"
    # Use kernel trick to clean up mirrors automatically when installer completed
    local MIRRORDIR="/proc/$$/attr" ROOTDIR SYSTEMDIR VENDORDIR

    ROOTDIR="$MIRRORDIR/system_root"
    SYSTEMDIR="$MIRRORDIR/system"
    VENDORDIR="$MIRRORDIR/vendor"
    ODM_DIR="$MIRRORDIR/odm"

    local MAGISKTMP_TO_INSTALL=/sbin

    if $BOOTMODE; then
        umount -l "/proc/$$/attr"
        # setup mirrors to get the original content
        mount -t tmpfs -o 'mode=0755' tmpfs "$MIRRORDIR" || return 1
        if is_rootfs; then
            ROOTDIR=/
            mkdir "$SYSTEMDIR"
            force_bind_mount "/system" "$SYSTEMDIR" || return 1
        else
            mkdir "$ROOTDIR"
            force_bind_mount "/" "$ROOTDIR" || return 1
            if mountpoint -q /system; then
                mkdir "$SYSTEMDIR"
                force_bind_mount "/system" "$SYSTEMDIR" || return 1
            else
                ln -fs ./system_root/system "$SYSTEMDIR"
            fi

            # we are modifying system directly so we need to create /sbin if it does not exist
            if [ ! -d "$ROOTDIR"/sbin ]; then
                rm -rf "$ROOTDIR"/sbin
                mkdir "$ROOTDIR"/sbin
                if [ ! -d "$ROOTDIR"/sbin ]; then
                    ui_print "! Can't create tmpfs path /sbin"
                    return 1;
                fi
            fi

        fi

        # check if /vendor is seperated fs
        if mountpoint -q /vendor; then
            mkdir "$VENDORDIR"
            force_bind_mount "/vendor" "$VENDORDIR" || return 1
         else
            ln -fs ./system/vendor "$VENDORDIR"
        fi

        # check if /odm is seperated fs
        if mountpoint -q /odm; then
            mkdir "$ODM_DIR"
            force_bind_mount "/odm" "$ODM_DIR" || return 1
         else
            ln -fs ./system_root/odm "$ODM_DIR"
        fi
    else
        local MIRRORDIR="/" ROOTDIR SYSTEMDIR VENDORDIR
        ROOTDIR="$MIRRORDIR/system_root"
        SYSTEMDIR="$MIRRORDIR/system"
        VENDORDIR="$MIRRORDIR/vendor"
        ODM_DIR="$MIRRORDIR/odm"
        ui_print "- Mount system partitions as read-write..."
        remount_check rw "$ROOTDIR" 0 || { warn_system_ro; return 1; }
        remount_check rw "$SYSTEMDIR" 0 || { warn_system_ro; return 1; }
        remount_check rw "$VENDORDIR" 0 || { warn_system_ro; return 1; }
        remount_check rw "$ODM_DIR" 0 || { warn_system_ro; return 1; }

        # we are modifying system directly so we need to create /sbin if it does not exist
        if [ -d "$ROOTDIR" ] && [ ! -d "$ROOTDIR"/sbin ]; then
            rm -rf "$ROOTDIR"/sbin
            mkdir "$ROOTDIR"/sbin
            if [ ! -d "$ROOTDIR"/sbin ]; then
                ui_print "! Can't create tmpfs path /sbin"
                return 1;
            fi
        fi

    fi


    ui_print "- Cleaning up enviroment..."
    {
        local checkfile="$MIRRORDIR/system/.check_$(random_str 10 20)"
        # test write, need atleast 20mb
        dd if=/dev/zero of="$checkfile" bs=1024 count=20000 || \
            { rm -rf "$checkfile"; ui_print "! Insufficient free space or system write protection"; return 1; }
        rm -rf "$checkfile"
    }
    cleanup_system_installation || return 1

    local magisk_applet=magisk32 magisk_name=magisk32
    if [ "$IS64BIT" == true ]; then
        magisk_name=magisk64
        magisk_applet="magisk32 magisk64"
    fi

    ui_print "- Copy files to system partition"
    mkdir -p "$MIRRORDIR$MAGISKSYSTEMDIR" || return 1
    for magisk in $magisk_applet magiskpolicy magiskinit stub.apk; do
        cat "$INSTALLDIR/$magisk" >"$MIRRORDIR$MAGISKSYSTEMDIR/$magisk" || { ui_print "! Unable to write Magisk binaries to system"; return 1; }
    done
    echo -e "SYSTEMMODE=true\nRECOVERYMODE=false" >"$MIRRORDIR$MAGISKSYSTEMDIR/config"
    chcon -R u:object_r:system_file:s0 "$MIRRORDIR$MAGISKSYSTEMDIR"
    chmod -R 700 "$MIRRORDIR$MAGISKSYSTEMDIR"

    if [ "$API" -gt 24 ]; then

        # test live patch
        {
            if $BOOTMODE; then
                ui_print "- Check if kernel supports dynamic SELinux Policy patch"
                if [ -d /sys/fs/selinux ] && ! "$INSTALLDIR/magiskpolicy" --live "permissive su" &>/dev/null; then
                    ui_print "! Kernel does not support dynamic SELinux Policy patch"
                    return 1
                fi
            else
                ui_print "W: It's impossible to check kernel compatible in recovery mode"
                ui_print "W: Please make sure your kernel can dynamic patch SELinux Policy"
            fi
            if ! is_rootfs; then
              {
                ui_print "- Patch sepolicy file"
                local sepol file
                for file in /vendor/etc/selinux/precompiled_sepolicy /odm/etc/selinux/precompiled_sepolicy /system/etc/selinux/precompiled_sepolicy /system_root/sepolicy /system_root/sepolicy_debug /system_root/sepolicy.unlocked; do
                    if [ -f "$MIRRORDIR$file" ]; then
                        sepol="$file"
                        break
                    fi
                done
                if [ -z "$sepol" ]; then
                    ui_print "! Cannot find sepolicy file"
                    return 1
                else
                    ui_print "- Target sepolicy is $sepol"
                    backup_restore "$MIRRORDIR$sepol" || { ui_print "! Backup failed"; return 1; }
                    # copy file to cache
                    cp -af "$MIRRORDIR$sepol" "$INSTALLDIR/sepol.in"
                    if ! "$INSTALLDIR/magiskinit" --patch-sepol "$INSTALLDIR/sepol.in" "$INSTALLDIR/sepol.out" || ! cp -af "$INSTALLDIR/sepol.out" "$MIRRORDIR$sepol"; then
                        ui_print "! Unable to patch sepolicy file"
                        restore_from_bak "$MIRRORDIR$sepol"
                        return 1
                    fi
                    ui_print "- Patching sepolicy file success!"
                fi
              }
            fi
        }
        ui_print "- Add init boot script"
        {
            hijackrc="$MIRRORDIR/system/etc/init/magisk.rc" 
            if [ -f "$MIRRORDIR/system/etc/init/bootanim.rc" ]; then
                backup_restore "$MIRRORDIR/system/etc/init/bootanim.rc" && hijackrc="$MIRRORDIR/system/etc/init/bootanim.rc"
            fi
        }
        echo "$(magiskrc "$MAGISKTMP_TO_INSTALL")" >>"$hijackrc" || return 1
    fi

    ui_print "[*] Reflash your ROM if your ROM is unable to start"
    ui_print "    and do not use this method to install Magisk" 

    $BOOTMODE && installer_cleanup
    true
    return 0
}



xdirect_install_system() {
  direct_install_system "$@" || { cleanup_system_installation; installer_cleanup; return 1; }
  fix_env "$1"
  install_addond "$3" "true"
  run_migrations
  return 0
}



#############
# Initialize
#############

app_init() {
  mount_partitions
  RAMDISKEXIST=false
  check_boot_ramdisk && RAMDISKEXIST=true
  get_flags
  run_migrations
  SHA1=$(grep_prop SHA1 $MAGISKTMP/.magisk/config)
  check_encryption
  get_sulist_status
  BOOTIMAGE_PATCHED=false
  [ ! -z "$SHA1" ] && BOOTIMAGE_PATCHED=true
}

export BOOTMODE=true
