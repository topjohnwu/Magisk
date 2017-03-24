#!/system/bin/sh

LOGFILE=/cache/magisk.log
DISABLEFILE=/cache/.disable_magisk
UNINSTALLER=/cache/magisk_uninstaller.sh
IMG=/data/magisk.img
WHITELIST="/system/bin"

MOUNTPOINT=/magisk

COREDIR=$MOUNTPOINT/.core

TMPDIR=/dev/magisk
DUMMDIR=$TMPDIR/dummy
MIRRDIR=$TMPDIR/mirror
MOUNTINFO=$TMPDIR/mnt

# Use the included busybox for maximum compatibility and reliable results
# e.g. we rely on the option "-c" for cp (reserve contexts), and -exec for find
TOOLPATH=/dev/busybox
BINPATH=/data/magisk
OLDPATH=$PATH
export PATH=$TOOLPATH:$OLDPATH

APPDIR=/data/data/com.topjohnwu.magisk/files

# Default permissions
umask 022

log_print() {
  echo "$1"
  echo "$1" >> $LOGFILE
  log -p i -t Magisk "$1"
}

mktouch() {
  mkdir -p "${1%/*}" 2>/dev/null
  if [ -z "$2" ]; then
    touch "$1" 2>/dev/null
  else
    echo "$2" > "$1" 2>/dev/null
  fi
}

in_list() {
  for i in $2; do
    [ "$1" = "$i" ] && return 0
  done
  return 1
}

unblock() {
  touch /dev/.magisk.unblock
  chcon u:object_r:device:s0 /dev/.magisk.unblock
  exit
}

bind_mount() {
  if [ -e "$1" -a -e "$2" ]; then
    $BINPATH/busybox mount -o bind "$1" "$2" || log_print "Mount Fail: $1 -> $2"
  fi
}

loopsetup() {
  LOOPDEVICE=
  for DEV in `ls /dev/block/loop*`; do
    if losetup $DEV $1; then
      LOOPDEVICE=$DEV
      break
    fi
  done
}

image_size_check() {
  e2fsck -yf $1
  curBlocks=`e2fsck -n $1 2>/dev/null | grep $1 | cut -d, -f3 | cut -d\  -f2`;
  curUsedM=`echo "$curBlocks" | cut -d/ -f1`
  curSizeM=`echo "$curBlocks" | cut -d/ -f1`
  curFreeM=$(((curSizeM - curUsedM) * 4 / 1024))
  curUsedM=$((curUsedM * 4 / 1024 + 1))
  curSizeM=$((curSizeM * 4 / 1024))
}

run_scripts() {
  BASE=$MOUNTPOINT
  for MOD in $BASE/* ; do
    if [ ! -f $MOD/disable ]; then
      if [ -f $MOD/$1.sh ]; then
        chmod 755 $MOD/$1.sh
        chcon u:object_r:system_file:s0 $MOD/$1.sh
        log_print "$1: $MOD/$1.sh"
        sh $MOD/$1.sh
      fi
    fi
  done
  for SCRIPT in $COREDIR/${1}.d/* ; do
    if [ -f "$SCRIPT" ]; then
      chmod 755 $SCRIPT
      chcon u:object_r:system_file:s0 $SCRIPT
      log_print "${1}.d: $SCRIPT"
      sh $SCRIPT
    fi
  done
}

travel() {
  cd "$TRAVEL_ROOT/$1"
  if [ -f .replace ]; then
    log_print "Replace: /$1"
    rm -rf "$MOUNTINFO/$1"
    mktouch "$MOUNTINFO/$1" "$TRAVEL_ROOT"
  else
    for ITEM in * ; do
      # This means it's an empty folder (shouldn't happen, but better to be safe)
      [ "$ITEM" = "*" ] && return
      # Ignore /system/vendor since we will handle it differently
      [ "$1" = "system" -a "$ITEM" = "vendor" ] && continue

      # Target not found or target/file is a symlink
      if [ ! -e "/$1/$ITEM" -o -L "/$1/$ITEM" -o -L "$ITEM" ]; then
        # If we are in a higher level, delete the lower levels
        rm -rf "$MOUNTINFO/dummy/$1" 2>/dev/null
        # Mount the dummy parent
        log_print "Replace with dummy: /$1"
        mktouch "$MOUNTINFO/dummy/$1"

        if [ -L "$ITEM" ]; then
          # Copy symlinks
          log_print "Symlink: /$1/$ITEM"
          mkdir -p "$DUMMDIR/$1" 2>/dev/null
          cp -afc "$ITEM" "$DUMMDIR/$1/$ITEM"
        elif [ -d "$ITEM" ]; then
          # Create new dummy directory and mount it
          log_print "New directory: /$1/$ITEM"
          mkdir -p "$DUMMDIR/$1/$ITEM"
          mktouch "$MOUNTINFO/$1/$ITEM" "$TRAVEL_ROOT"
        else
          # Create new dummy file and mount it
          log_print "New file: /$1/$ITEM"
          mktouch "$DUMMDIR/$1/$ITEM"
          mktouch "$MOUNTINFO/$1/$ITEM" "$TRAVEL_ROOT"
        fi
      else
        if [ -d "$ITEM" ]; then
          # It's an directory, travel deeper
          (travel "$1/$ITEM")
        elif [ ! -L "$ITEM" ]; then
          # Mount this file
          log_print "Replace: /$1/$ITEM"
          mktouch "$MOUNTINFO/$1/$ITEM" "$TRAVEL_ROOT"
        fi
      fi
    done
  fi
}

clone_dummy() {
  LINK=false
  in_list "$1" "$WHITELIST" && LINK=true

  for ITEM in $MIRRDIR$1/* ; do
    REAL="${ITEM#$MIRRDIR}"
    if [ -d "$MOUNTINFO$REAL" ]; then
      # Need to clone deeper
      mkdir -p "$DUMMDIR$REAL"
      (clone_dummy "$REAL")
    elif [ ! -f "$DUMMDIR$REAL" ]; then
      # It's not the file to be added/replaced, clone it
      if [ -L "$ITEM" ]; then
        # Copy original symlink
        cp -afc "$ITEM" "$DUMMDIR$REAL"
      else
        if $LINK && [ ! -e "$MOUNTINFO$REAL" ]; then
          ln -sf "$MIRRDIR$REAL" "$DUMMDIR$REAL"
        else
          if [ -d "$ITEM" ]; then
            mkdir -p "$DUMMDIR$REAL"
          else
            mktouch "$DUMMDIR$REAL"
          fi
          if [ ! -e "$MOUNTINFO$REAL" ]; then
            log_print "Clone skeleton: $REAL"
            mktouch "$MOUNTINFO/mirror$REAL"
          fi
        fi
      fi
    fi
  done
}

merge_image() {
  if [ -f $1 ]; then
    log_print "$1 found"
    if [ -f $IMG ]; then
      log_print "$IMG found, attempt to merge"

      # Handle large images
      image_size_check $1
      mergeUsedM=$curUsedM
      image_size_check $IMG
      if [ "$mergeUsedM" -gt "$curFreeM" ]; then
        NEWDATASIZE=$(((mergeUsedM + curUsedM) / 32 * 32 + 32))
        log_print "Expanding $IMG to ${NEWDATASIZE}M..."
        resize2fs $IMG ${NEWDATASIZE}M
      fi

      # Start merging
      mkdir /cache/data_img
      mkdir /cache/merge_img

      # setup loop devices
      loopsetup $IMG
      LOOPDATA=$LOOPDEVICE
      log_print "$LOOPDATA $IMG"

      loopsetup $1
      LOOPMERGE=$LOOPDEVICE
      log_print "$LOOPMERGE $1"

      if [ ! -z $LOOPDATA -a ! -z $LOOPMERGE ]; then
        # if loop devices have been setup, mount images
        OK=false
        mount -t ext4 -o rw,noatime $LOOPDATA /cache/data_img && \
        mount -t ext4 -o rw,noatime $LOOPMERGE /cache/merge_img && \
        OK=true

        if $OK; then
          # Merge (will reserve selinux contexts)
          cd /cache/merge_img
          for MOD in *; do
            if [ "$MOD" != "lost+found" ]; then
              log_print "Merging: $MOD"
              rm -rf /cache/data_img/$MOD
            fi
          done
          cp -afc . /cache/data_img
          log_print "Merge complete"
          cd /
        fi

        umount /cache/data_img
        umount /cache/merge_img
      fi

      losetup -d $LOOPDATA
      losetup -d $LOOPMERGE

      rmdir /cache/data_img
      rmdir /cache/merge_img
    else 
      log_print "Moving $1 to $IMG "
      mv $1 $IMG
    fi
    rm -f $1
  fi
}

case $1 in
  post-fs )
    mv $LOGFILE /cache/last_magisk.log
    touch $LOGFILE
    chmod 644 $LOGFILE

    # No more cache mods!
    # Only for multirom!

    log_print "** Magisk post-fs mode running..."

    # Cleanup legacy stuffs...
    rm -rf /cache/magisk /cache/magisk_merge /cache/magiskhide.log

    [ -f $DISABLEFILE -o -f $UNINSTALLER ] && unblock

    if [ -d /cache/magisk_mount ]; then
      log_print "* Mounting cache files"
      find /cache/magisk_mount -type f 2>/dev/null | while read ITEM ; do
        chmod 644 "$ITEM"
        chcon u:object_r:system_file:s0 "$ITEM"
        TARGET="${ITEM#/cache/magisk_mount}"
        bind_mount "$ITEM" "$TARGET"
      done
    fi

    unblock
    ;;

  post-fs-data )
    # /data not mounted yet
    ! mount | grep " /data " >/dev/null && unblock
    mount | grep " /data " | grep "tmpfs" >/dev/null && unblock

    # Don't run twice
    if [ "`getprop magisk.restart_pfsd`" != "1" ]; then

      log_print "** Magisk post-fs-data mode running..."

      # Cache support
      mv /cache/stock_boot* /data 2>/dev/null
      if [ -d /cache/data_bin ]; then
        rm -rf $BINPATH
        mv /cache/data_bin $BINPATH
      fi

      chmod -R 755 $BINPATH
      chown -R 0.0 $BINPATH

      # Live patch sepolicy
      $BINPATH/magiskpolicy --live

      if [ -f $UNINSTALLER ]; then
        touch /dev/.magisk.unblock
        chcon u:object_r:device:s0 /dev/.magisk.unblock
        BOOTMODE=true sh $UNINSTALLER
        exit
      fi

      # Set up environment
      mkdir -p $TOOLPATH
      $BINPATH/busybox --install -s $TOOLPATH
      ln -sf $BINPATH/busybox $TOOLPATH/busybox
      # Prevent issues
      rm -f $TOOLPATH/su $TOOLPATH/sh $TOOLPATH/reboot
      chmod -R 755 $TOOLPATH
      chown -R 0.0 $TOOLPATH
      find $BINPATH $TOOLPATH -exec chcon -h u:object_r:system_file:s0 {} \;

      log_print "* Linking binaries to /sbin"
      mount -o rw,remount rootfs /
      chmod 755 /sbin
      ln -sf $BINPATH/magiskpolicy /sbin/magiskpolicy
      ln -sf $BINPATH/magiskpolicy /sbin/sepolicy-inject
      ln -sf $BINPATH/resetprop /sbin/resetprop
      if [ ! -f /sbin/launch_daemonsu.sh ]; then
        log_print "* Starting MagiskSU"
        export PATH=$OLDPATH
        ln -sf $BINPATH/su /sbin/su
        ln -sf $BINPATH/magiskpolicy /sbin/supolicy
        /sbin/su --daemon
        export PATH=$TOOLPATH:$OLDPATH
      fi
      mount -o ro,remount rootfs /

      [ -f $DISABLEFILE ] && unblock

      # Multirom functions should go here, not available right now
      MULTIROM=false

      # Image merging
      chmod 644 $IMG /cache/magisk.img /data/magisk_merge.img 2>/dev/null
      merge_image /cache/magisk.img
      merge_image /data/magisk_merge.img

      # Mount magisk.img
      [ ! -d $MOUNTPOINT ] && mkdir -p $MOUNTPOINT
      if ! mount | grep $MOUNTPOINT; then
        loopsetup $IMG
        [ ! -z $LOOPDEVICE ] && mount -t ext4 -o rw,noatime $LOOPDEVICE $MOUNTPOINT
        if [ $? -ne 0 ]; then
          log_print "magisk.img mount failed, nothing to do :("
          unblock
        fi
      fi

      # Remove empty directories, legacy paths, symlinks, old temporary images
      find $MOUNTPOINT -type d -depth ! -path "*core*" -exec rmdir {} \; 2>/dev/null
      rm -rf $MOUNTPOINT/zzsupersu $MOUNTPOINT/phh $COREDIR/bin $COREDIR/dummy $COREDIR/mirror \
             $COREDIR/busybox $COREDIR/su /data/magisk/*.img /data/busybox 2>/dev/null

      # Remove modules that are labeled to be removed
      for MOD in $MOUNTPOINT/* ; do
        rm -f $MOD/system/placeholder 2>/dev/null
        if [ -f $MOD/remove ]; then
          log_print "Remove module: $MOD"
          rm -rf $MOD
        fi
      done

      # Unmount, shrink, remount
      if umount $MOUNTPOINT; then
        losetup -d $LOOPDEVICE 2>/dev/null
        image_size_check $IMG
        NEWDATASIZE=$((curUsedM / 32 * 32 + 32))
        if [ "$curSizeM" -gt "$NEWDATASIZE" ]; then
          log_print "Shrinking $IMG to ${NEWDATASIZE}M..."
          resize2fs $IMG ${NEWDATASIZE}M
        fi
        loopsetup $IMG
        [ ! -z $LOOPDEVICE ] && mount -t ext4 -o rw,noatime $LOOPDEVICE $MOUNTPOINT
        if [ $? -ne 0 ]; then
          log_print "magisk.img mount failed, nothing to do :("
          unblock
        fi
      fi

      log_print "* Preparing modules"

      # Remove crap folder
      rm -rf $MOUNTPOINT/lost+found

      # Link vendor if not exist
      if [ ! -e /vendor ]; then
        mount -o rw,remount rootfs /
        ln -sf /system/vendor /vendor
        mount -o ro,remount rootfs /
      fi

      for MOD in $MOUNTPOINT/* ; do
        if [ ! -f $MOD/disable ]; then
          # Travel through all mods
          if [ -f $MOD/auto_mount -a -d $MOD/system ]; then
            log_print "Analyzing module: $MOD"
            TRAVEL_ROOT=$MOD
            (travel system)
            rm -f $MOD/vendor 2>/dev/null
            if [ -d $MOD/system/vendor ]; then
              ln -sf $MOD/system/vendor $MOD/vendor
              (travel vendor)
            fi
          fi
          # Read in defined system props
          if [ -f $MOD/system.prop ]; then
            log_print "* Reading props from $MOD/system.prop"
            $BINPATH/resetprop --file $MOD/system.prop
          fi
        fi
      done

      # Proper permissions for generated items
      find $TMPDIR -exec chcon -h u:object_r:system_file:s0 {} \;

      # linker(64), t*box required for bin
      if [ -f $MOUNTINFO/dummy/system/bin ]; then
        cp -afc /system/bin/linker* /system/bin/t*box $DUMMDIR/system/bin/
      fi

      # Start doing tasks

      # Stage 1
      log_print "* Stage 1: Mount system and vendor mirrors"
      SYSTEMBLOCK=`mount | grep " /system " | awk '{print $1}'`
      mkdir -p $MIRRDIR/system
      mount -o ro $SYSTEMBLOCK $MIRRDIR/system
      if [ `mount | grep -c " /vendor "` -ne 0 ]; then
        VENDORBLOCK=`mount | grep " /vendor " | awk '{print $1}'`
        mkdir -p $MIRRDIR/vendor
        mount -o ro $VENDORBLOCK $MIRRDIR/vendor
      else
        ln -sf $MIRRDIR/system/vendor $MIRRDIR/vendor
      fi

      # Since mirrors always exist, we load libraries and binaries from mirrors
      export LD_LIBRARY_PATH=$MIRRDIR/system/lib:$MIRRDIR/vendor/lib
      [ -d $MIRRDIR/system/lib64 ] && export LD_LIBRARY_PATH=$MIRRDIR/system/lib64:$MIRRDIR/vendor/lib64

      # Stage 2
      log_print "* Stage 2: Mount dummy skeletons"
      # Move /system/vendor to /vendor for consistency
      mv -f $MOUNTINFO/dummy/system/vendor $MOUNTINFO/dummy/vendor 2>/dev/null
      mv -f $DUMMDIR/system/vendor $DUMMDIR/vendor 2>/dev/null
      find $MOUNTINFO/dummy -type f 2>/dev/null | while read ITEM ; do
        TARGET="${ITEM#$MOUNTINFO/dummy}"
        ORIG="$DUMMDIR$TARGET"
        (clone_dummy "$TARGET")
        bind_mount "$ORIG" "$TARGET"
      done

      # Check if the dummy /system/bin is empty, it shouldn't
      [ -e $DUMMDIR/system/bin -a ! -e $DUMMDIR/system/bin/sh ] && clone_dummy /system/bin

      # Stage 3
      log_print "* Stage 3: Mount module items"
      find $MOUNTINFO/system $MOUNTINFO/vendor -type f 2>/dev/null | while read ITEM ; do
        TARGET="${ITEM#$MOUNTINFO}"
        ORIG="`cat "$ITEM"`$TARGET"
        bind_mount "$ORIG" "$TARGET"
      done

      # Stage 4
      log_print "* Stage 4: Execute scripts"
      run_scripts post-fs-data

      # Stage 5
      log_print "* Stage 5: Mount mirrored items back to dummy"
      find $MOUNTINFO/mirror -type f 2>/dev/null | while read ITEM ; do
        TARGET="${ITEM#$MOUNTINFO/mirror}"
        ORIG="$MIRRDIR$TARGET"
        bind_mount "$ORIG" "$TARGET"
      done

      # Bind hosts for Adblock apps
      if [ -f $COREDIR/hosts ]; then
        log_print "* Enabling systemless hosts file support"
        bind_mount $COREDIR/hosts /system/etc/hosts
      fi

      if [ -f $BINPATH/magisk.apk ]; then
        if ! ls /data/app | grep com.topjohnwu.magisk; then
          mkdir /data/app/com.topjohnwu.magisk-1
          cp $BINPATH/magisk.apk /data/app/com.topjohnwu.magisk-1/base.apk
          chown 1000.1000 /data/app/com.topjohnwu.magisk-1
          chown 1000.1000 /data/app/com.topjohnwu.magisk-1/base.apk
          chmod 755 /data/app/com.topjohnwu.magisk-1
          chmod 644 /data/app/com.topjohnwu.magisk-1/base.apk
          chcon u:object_r:apk_data_file:s0 /data/app/com.topjohnwu.magisk-1
          chcon u:object_r:apk_data_file:s0 /data/app/com.topjohnwu.magisk-1/base.apk
        fi
        rm -f $BINPATH/magisk.apk 2>/dev/null
      fi

      # Expose busybox
      [ "`getprop persist.magisk.busybox`" = "1" ] && sh /sbin/magic_mask.sh mount_busybox

      # Restart post-fs-data if necessary (multirom)
      $MULTIROM && setprop magisk.restart_pfsd 1

    fi
    unblock
    ;;

  mount_busybox )
    log_print "* Enabling BusyBox"
    cp -afc /system/xbin/. $TOOLPATH
    umount /system/xbin 2>/dev/null
    bind_mount $TOOLPATH /system/xbin
    ;;

  service )
    # Version info
    MAGISK_VERSION_STUB
    log_print "** Magisk late_start service mode running..."
    if [ -f $DISABLEFILE ]; then
      setprop ro.magisk.disable 1
      exit
    fi
    run_scripts service
    
    # Start MagiskHide
    [ "`getprop persist.magisk.hide`" = "1" ] && sh $COREDIR/magiskhide/enable
    ;;

esac
