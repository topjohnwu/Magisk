#!/system/bin/sh

LOGFILE=/cache/magisk.log
IMG=/data/magisk.img

MOUNTPOINT=/magisk

COREDIR=$MOUNTPOINT/.core

TMPDIR=/dev/magisk
DUMMDIR=$TMPDIR/dummy
MIRRDIR=$TMPDIR/mirror
MOUNTINFO=$TMPDIR/mnt

# Use the included busybox for maximum compatibility and reliable results
# e.g. we rely on the option "-c" for cp (reserve contexts), and -exec for find
TOOLPATH=/data/busybox
BINPATH=/data/magisk
OLDPATH=$PATH
PATH=$TOOLPATH:$OLDPATH

# Default permissions
umask 022


log_print() {
  echo "$1"
  echo "$1" >> $LOGFILE
  log -p i -t Magisk "$1"
}

mktouch() {
  mkdir -p ${1%/*} 2>/dev/null
  if [ -z "$2" ]; then
    touch "$1" 2>/dev/null
  else
    echo "$2" > "$1" 2>/dev/null
  fi
}

unblock() {
  touch /dev/.magisk.unblock
  exit
}

run_scripts() {
  PATH=$OLDPATH
  BASE=$MOUNTPOINT
  for MOD in $BASE/* ; do
    if [ ! -f "$MOD/disable" ]; then
      if [ -f "$MOD/$1.sh" ]; then
        chmod 755 $MOD/$1.sh
        chcon "u:object_r:system_file:s0" "$MOD/$1.sh"
        log_print "$1: $MOD/$1.sh"
        sh $MOD/$1.sh
      fi
    fi
  done
  PATH=$TOOLPATH:$OLDPATH
}

loopsetup() {
  LOOPDEVICE=
  for DEV in $(ls /dev/block/loop*); do
    if [ `losetup $DEV $1 >/dev/null 2>&1; echo $?` -eq 0 ]; then
      LOOPDEVICE=$DEV
      break
    fi
  done
}

target_size_check() {
  e2fsck -p -f $1
  curBlocks=`e2fsck -n $1 2>/dev/null | cut -d, -f3 | cut -d\  -f2`;
  curUsedM=$((`echo "$curBlocks" | cut -d/ -f1` * 4 / 1024));
  curSizeM=$((`echo "$curBlocks" | cut -d/ -f2` * 4 / 1024));
  curFreeM=$((curSizeM - curUsedM));
}

travel() {
  cd "$1/$2"
  if [ -f ".replace" ]; then
    rm -rf "$MOUNTINFO/$2"
    mktouch "$MOUNTINFO/$2" "$1"
  else
    for ITEM in * ; do
      if [ ! -e "/$2/$ITEM" ]; then
        # New item found
        if [ "$2" = "system" ]; then
          # We cannot add new items to /system root, delete it
          rm -rf "$ITEM"
        else
          # If we are in a higher level, delete the lower levels
          rm -rf "$MOUNTINFO/dummy/$2"
          # Mount the dummy parent
          mktouch "$MOUNTINFO/dummy/$2"

          if [ -d "$ITEM" ]; then
            # Create new dummy directory and mount it
            mkdir -p "$DUMMDIR/$2/$ITEM"
            mktouch "$MOUNTINFO/$2/$ITEM" "$1"
          elif [ -L "$ITEM" ]; then
            # Symlinks are small, copy them
            mkdir -p "$DUMMDIR/$2" 2>/dev/null
            cp -afc "$ITEM" "$DUMMDIR/$2/$ITEM"
          else
            # Create new dummy file and mount it
            mktouch "$DUMMDIR/$2/$ITEM"
            mktouch "$MOUNTINFO/$2/$ITEM" "$1"
          fi
        fi
      else
        if [ -d "$ITEM" ]; then
          # It's an directory, travel deeper
          (travel "$1" "$2/$ITEM")
        elif [ ! -L "$ITEM" ]; then
          # Mount this file
          mktouch "$MOUNTINFO/$2/$ITEM" "$1"
        fi
      fi
    done
  fi
}

clone_dummy() {
  for ITEM in "$1/"* ; do
    if [ -d "$DUMMDIR$ITEM" ]; then
      (clone_dummy "$ITEM")
    elif [ ! -e "$DUMMDIR$ITEM" ]; then
      if [ -d "$ITEM" ]; then
        # Create dummy directory
        mkdir -p "$DUMMDIR$ITEM"
      elif [ -L "$ITEM" ]; then
        # Symlinks are small, copy them
        cp -afc "$ITEM" "$DUMMDIR$ITEM"
      else
        # Create dummy file
        mktouch "$DUMMDIR$ITEM"
      fi
      chcon -f "u:object_r:system_file:s0" "$DUMMDIR$ITEM"
    fi
  done
}

bind_mount() {
  if [ -e "$1" -a -e "$2" ]; then
    mount -o bind $1 $2
    if [ "$?" -eq "0" ]; then 
      log_print "Mount: $1"
    else 
      log_print "Mount Fail: $1"
    fi 
  fi
}

merge_image() {
  if [ -f "$1" ]; then
    log_print "$1 found"
    if [ -f "$IMG" ]; then
      log_print "$IMG found, attempt to merge"

      # Handle large images
      target_size_check $1
      MERGEUSED=$curUsedM
      target_size_check $IMG
      if [ "$MERGEUSED" -gt "$curFreeM" ]; then
        NEWDATASIZE=$((((MERGEUSED + curUsedM) / 32 + 2) * 32))
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

      if [ ! -z "$LOOPDATA" ]; then
        if [ ! -z "$LOOPMERGE" ]; then
          # if loop devices have been setup, mount images
          OK=true

          if [ `mount -t ext4 -o rw,noatime $LOOPDATA /cache/data_img >/dev/null 2>&1; echo $?` -ne 0 ]; then
            OK=false
          fi

          if [ `mount -t ext4 -o rw,noatime $LOOPMERGE /cache/merge_img >/dev/null 2>&1; echo $?` -ne 0 ]; then
            OK=false
          fi

          if ($OK); then
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

    # Cleanup previous version stuffs...
    rm -rf /cache/magisk /cache/magisk_merge /cache/magiskhide.log

    if [ -d "/cache/magisk_mount" ]; then
      log_print "Mounting cache files"
      find /cache/magisk_mount -type f 2>/dev/null | while read ITEM ; do
        chmod 644 "$ITEM"
        chcon "u:object_r:system_file:s0" "$ITEM"
        TARGET="${ITEM#/cache/magisk_mount}"
        bind_mount "$ITEM" "$TARGET"
      done
    fi

    unblock
    ;;

  post-fs-data )
    if [ `mount | grep " /data " >/dev/null 2>&1; echo $?` -ne 0 ]; then
      # /data not mounted yet, we will be called again later
      unblock
    fi

    if [ `mount | grep " /data " | grep "tmpfs" >/dev/null 2>&1; echo $?` -eq 0 ]; then
      # /data not mounted yet, we will be called again later
      unblock
    fi

    # Don't run twice
    if [ "`getprop magisk.restart_pfsd`" != "1" ]; then

      log_print "** Magisk post-fs-data mode running..."

      # Live patch sepolicy
      $BINPATH/sepolicy-inject --live -s su

      # Multirom functions should go here, not available right now
      MULTIROM=false

      # Cache support
      if [ -d "/cache/data_bin" ]; then
        rm -rf $BINPATH $TOOLPATH
        mkdir -p $TOOLPATH
        mv /cache/data_bin $BINPATH
        chmod -R 755 $BINPATH $TOOLPATH
        $BINPATH/busybox --install -s $TOOLPATH
        ln -s $BINPATH/busybox $TOOLPATH/busybox
        # Prevent issues
        rm -f $TOOLPATH/su $TOOLPATH/sh
      fi

      mv /cache/stock_boot.img /data 2>/dev/null

      find $BINPATH -exec chcon -h "u:object_r:system_file:s0" {} \;
      find $TOOLPATH -exec chcon -h "u:object_r:system_file:s0" {} \;
      chmod -R 755 $BINPATH $TOOLPATH

      # Image merging
      chmod 644 $IMG /cache/magisk.img /data/magisk_merge.img 2>/dev/null
      merge_image /cache/magisk.img
      merge_image /data/magisk_merge.img

      # Mount magisk.img
      [ ! -d "$MOUNTPOINT" ] && mkdir -p $MOUNTPOINT
      if [ `cat /proc/mounts | grep $MOUNTPOINT >/dev/null 2>&1; echo $?` -ne 0 ]; then
        loopsetup $IMG
        if [ ! -z "$LOOPDEVICE" ]; then
          mount -t ext4 -o rw,noatime $LOOPDEVICE $MOUNTPOINT
        fi
      fi

      if [ `cat /proc/mounts | grep $MOUNTPOINT >/dev/null 2>&1; echo $?` -ne 0 ]; then
        log_print "magisk.img mount failed, nothing to do :("
        unblock
      fi

      # Remove empty directories, legacy paths and symlinks
      rm -rf $COREDIR/bin $COREDIR/dummy $COREDIR/mirror
      find $MOUNTPOINT -type d -depth ! -path "*core*" -exec rmdir {} \; 2>/dev/null

      # Remove modules
      for MOD in $MOUNTPOINT/* ; do
        if [ -f "$MOD/remove" ] || [ "$MOD" = "zzsupersu" ]; then
          log_print "Remove module: $MOD"
          rm -rf $MOD
        fi
      done

      # Unmount, shrink, remount
      if [ `umount $MOUNTPOINT >/dev/null 2>&1; echo $?` -eq 0 ]; then
        losetup -d $LOOPDEVICE
        target_size_check $IMG
        NEWDATASIZE=$(((curUsedM / 32 + 2) * 32))
        if [ "$curSizeM" -gt "$NEWDATASIZE" ]; then
          log_print "Shrinking $IMG to ${NEWDATASIZE}M..."
          resize2fs $IMG ${NEWDATASIZE}M
        fi
        loopsetup $IMG
        if [ ! -z "$LOOPDEVICE" ]; then
          mount -t ext4 -o rw,noatime $LOOPDEVICE $MOUNTPOINT
        fi
        if [ `cat /proc/mounts | grep $MOUNTPOINT >/dev/null 2>&1; echo $?` -ne 0 ]; then
          log_print "magisk.img mount failed, nothing to do :("
          unblock
        fi
      fi

      log_print "Preparing modules"

      mkdir -p $DUMMDIR
      mkdir -p $MIRRDIR/system

      # Travel through all mods, all magic happens here
      for MOD in $MOUNTPOINT/* ; do
        if [ -f "$MOD/auto_mount" -a -d "$MOD/system" -a ! -f "$MOD/disable" ]; then
          (travel $MOD system)
        fi
      done

      # Proper permissions for generated items
      find $TMPDIR -exec chcon -h "u:object_r:system_file:s0" {} \;

      # linker(64), t*box required
      if [ -f "$MOUNTINFO/dummy/system/bin" ]; then
        cd /system/bin
        cp -afc linker* t*box $DUMMDIR/system/bin/
      fi

      # Some libraries are required
      LIBS="libc++.so libc.so libcutils.so libm.so libstdc++.so libcrypto.so liblog.so libpcre.so libselinux.so libpackagelistparser.so"
      if [ -f "$MOUNTINFO/dummy/system/lib" ]; then
        cd /system/lib
        cp -afc $LIBS $DUMMDIR/system/lib
      fi
      if [ -f "$MOUNTINFO/dummy/system/lib64" ]; then
        cd /system/lib64
        cp -afc $LIBS $DUMMDIR/system/lib64
      fi

      # vendor libraries are device dependent, had no choice but copy them all if needed....
      if [ -f "$MOUNTINFO/dummy/system/vendor" ]; then
        cp -afc /system/vendor/lib/. $DUMMDIR/system/vendor/lib
        [ -d "/system/vendor/lib64" ] && cp -afc /system/vendor/lib64/. $DUMMDIR/system/vendor/lib64
      fi
      if [ -f "$MOUNTINFO/dummy/system/vendor/lib" ]; then
        cp -afc /system/vendor/lib/. $DUMMDIR/system/vendor/lib
      fi
      if [ -f "$MOUNTINFO/dummy/system/vendor/lib64" ]; then
        cp -afc /system/vendor/lib64/. $DUMMDIR/system/vendor/lib64
      fi

      # Remove crap folder
      rm -rf $MOUNTPOINT/lost+found
      
      # Start doing tasks
      
      # Stage 1
      log_print "Bind mount dummy system"
      find $MOUNTINFO/dummy -type f 2>/dev/null | while read ITEM ; do
        TARGET=${ITEM#$MOUNTINFO/dummy}
        ORIG="$DUMMDIR$TARGET"
        clone_dummy "$TARGET"
        bind_mount "$ORIG" "$TARGET"
      done

      # Stage 2
      log_print "Bind mount module items"
      find $MOUNTINFO/system -type f 2>/dev/null | while read ITEM ; do
        TARGET=${ITEM#$MOUNTINFO}
        ORIG=`cat $ITEM`$TARGET
        bind_mount $ORIG $TARGET
        rm -f $DUMMDIR${TARGET%/*}/.dummy 2>/dev/null
      done

      # Run scripts
      run_scripts post-fs-data

      # Bind hosts for Adblock apps
      if [ -f "$COREDIR/hosts" ]; then
        log_print "Enabling systemless hosts file support"
        bind_mount $COREDIR/hosts /system/etc/hosts
      fi

      # Expose busybox
      if [ -f "$COREDIR/busybox/enable" ]; then
        log_print "Enabling BusyBox"
        cp -afc /data/busybox/. $COREDIR/busybox
        cp -afc /system/xbin/. $COREDIR/busybox
        chmod -R 755 $COREDIR/busybox
        chcon -hR "u:object_r:system_file:s0" $COREDIR/busybox
        bind_mount $COREDIR/busybox /system/xbin
      fi

      # Stage 3
      log_print "Bind mount system mirror"
      bind_mount /system $MIRRDIR/system

      # Stage 4
      log_print "Bind mount mirror items"
      # Find all empty directores and dummy files, they should be mounted by original files in /system
      TOOLPATH=/data/busybox find $DUMMDIR -type d \
      -exec sh -c '[ -z "`ls -A $1`" ] && echo $1' -- {} \; \
      -o \( -type f -size 0 -print \) | \
      while read ITEM ; do
        ORIG=${ITEM/dummy/mirror}
        TARGET=${ITEM#$DUMMDIR}
        bind_mount $ORIG $TARGET
      done

      # Restart post-fs-data if necessary (multirom)
      ($MULTIROM) && setprop magisk.restart_pfsd 1

    fi
    unblock
    ;;

  service )
    # Version info
    MAGISK_VERSION_STUB
    log_print "** Magisk late_start service mode running..."
    run_scripts service

    # Magisk Hide
    if [ -f "$COREDIR/magiskhide/enable" ]; then
      log_print "** Removing tampered read-only system props"

      VERIFYBOOT=`getprop ro.boot.verifiedbootstate`
      FLASHLOCKED=`getprop ro.boot.flash.locked`
      VERITYMODE=`getprop ro.boot.veritymode`

      [ ! -z "$VERIFYBOOT" -a "$VERIFYBOOT" != "green" ] && log_print "`$BINPATH/resetprop -v -n ro.boot.verifiedbootstate green`"
      [ ! -z "$FLASHLOCKED" -a "$FLASHLOCKED" != "1" ] && log_print "`$BINPATH/resetprop -v -n ro.boot.flash.locked 1`"
      [ ! -z "$VERITYMODE" -a "$VERITYMODE" != "enforcing" ] && log_print "`$BINPATH/resetprop -v -n ro.boot.veritymode enforcing`"

      mktouch $COREDIR/magiskhide/hidelist
      chmod -R 755 $COREDIR/magiskhide
      # Add Safety Net preset
      $COREDIR/magiskhide/add com.google.android.gms.unstable
      log_print "** Starting Magisk Hide"
      /data/magisk/magiskhide
    fi
    ;;

esac
