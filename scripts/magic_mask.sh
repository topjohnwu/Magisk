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

export OLDPATH=$PATH
export PATH=$TOOLPATH:$OLDPATH

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
  cd "$TRAVEL_ROOT/$1"
  if [ -f ".replace" ]; then
    rm -rf "$MOUNTINFO/$1"
    mktouch "$MOUNTINFO/$1" "$TRAVEL_ROOT"
  else
    for ITEM in * ; do
      if [ ! -e "/$1/$ITEM" ]; then
        # New item found
        if [ "$1" = "system" ]; then
          # We cannot add new items to /system root, delete it
          rm -rf "$ITEM"
        else
          # If we are in a higher level, delete the lower levels
          rm -rf "$MOUNTINFO/dummy/$1"
          # Mount the dummy parent
          mktouch "$MOUNTINFO/dummy/$1"

          if [ -d "$ITEM" ]; then
            # Create new dummy directory and mount it
            mkdir -p "$DUMMDIR/$1/$ITEM"
            mktouch "$MOUNTINFO/$1/$ITEM" "$TRAVEL_ROOT"
          elif [ -L "$ITEM" ]; then
            # Symlinks are small, copy them
            mkdir -p "$DUMMDIR/$1" 2>/dev/null
            cp -afc "$ITEM" "$DUMMDIR/$1/$ITEM"
          else
            # Create new dummy file and mount it
            mktouch "$DUMMDIR/$1/$ITEM"
            mktouch "$MOUNTINFO/$1/$ITEM" "$TRAVEL_ROOT"
          fi
        fi
      else
        if [ -d "$ITEM" ]; then
          # It's an directory, travel deeper
          (travel "$1/$ITEM")
        elif [ ! -L "$ITEM" ]; then
          # Mount this file
          mktouch "$MOUNTINFO/$1/$ITEM" "$TRAVEL_ROOT"
        fi
      fi
    done
  fi
}

clone_dummy() {
  for ITEM in $1/* ; do
    if [ ! -e "$TRAVEL_ROOT$ITEM" ]; then
      if [ ! -d "$MOUNTINFO$ITEM" ]; then
        if [ -L "$ITEM" ]; then
          # Copy original symlink
          cp -afc "$ITEM" "$TRAVEL_ROOT$ITEM"
        else
          # Link to mirror item
          ln -s "$MIRRDIR$ITEM" "$TRAVEL_ROOT$ITEM"
        fi
      else
        # Need to clone a skeleton
        (clone_dummy "$ITEM")
      fi
    elif [ -d "$TRAVEL_ROOT$ITEM" ]; then
      # Need to clone a skeleton
      (clone_dummy "$ITEM")
    fi
  done
}

make_copy_image() {
  TARGETSIZE=$2
  if [ -z $TARGETSIZE ]; then
    TARGETSIZE=`du -s $1 | awk '{print $1}'`
    TARGETSIZE=$((($TARGETSIZE / 10240 + 2) * 10240))
  fi
  TARGETIMG=/data/magisk/${1//\//_}.img
  make_ext4fs -l ${TARGETSIZE}K -a $1 $TARGETIMG
  loopsetup $TARGETIMG
  mkdir -p $MIRRDIR/copy$1
  mount -t ext4 -o rw,noatime $LOOPDEVICE $MIRRDIR/copy$1
  return $?
}

mount_copy_image() {
  TARGETIMG=/data/magisk/${1//\//_}.img
  umount $MIRRDIR/copy$1
  rm -rf $MIRRDIR/copy
  losetup -d $LOOPDEVICE 2>/dev/null
  loopsetup $TARGETIMG
  mount -t ext4 -o rw,noatime $LOOPDEVICE $1
  return $?
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
      log_print "* Mounting cache files"
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

      # Live patch sepolicy
      $BINPATH/sepolicy-inject --live -s su

      # Multirom functions should go here, not available right now
      MULTIROM=false

      # Image merging
      chmod 644 $IMG /cache/magisk.img /data/magisk_merge.img 2>/dev/null
      merge_image /cache/magisk.img
      merge_image /data/magisk_merge.img

      # Mount magisk.img
      [ ! -d $MOUNTPOINT ] && mkdir -p $MOUNTPOINT
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

      # Remove empty directories, legacy paths, symlinks, old temporary images
      find $MOUNTPOINT -type d -depth ! -path "*core*" -exec rmdir {} \; 2>/dev/null
      rm -rf $COREDIR/bin $COREDIR/dummy $COREDIR/mirror /data/magisk/*.img

      # Remove modules that is labeled to be removed
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

      log_print "* Preparing modules"

      mkdir -p $DUMMDIR
      mkdir -p $MIRRDIR/system

      # Travel through all mods
      for MOD in $MOUNTPOINT/* ; do
        if [ -f "$MOD/auto_mount" -a -d "$MOD/system" -a ! -f "$MOD/disable" ]; then
          TRAVEL_ROOT=$MOD
          (travel system)
        fi
      done

      # Proper permissions for generated items
      find $TMPDIR -exec chcon -h "u:object_r:system_file:s0" {} \;

      # linker(64), t*box required for bin
      if [ -f "$MOUNTINFO/dummy/system/bin" ]; then
        cp -afc /system/bin/linker* /system/bin/t*box $DUMMDIR/system/bin/
      fi

      BACKUPLIBS=false

      # Libraries are full of issues, copy a full clone to data

      # lib
      if [ -f "$MOUNTINFO/dummy/system/lib" ]; then
        BACKUPLIBS=true
        make_copy_image /system/lib
        cp -afc /system/lib/. $MIRRDIR/copy/system/lib
        cp -afc $DUMMDIR/system/lib/. $MIRRDIR/copy/system/lib
        mount_copy_image /system/lib
        rm -f $MOUNTINFO/dummy/system/lib
      fi

      # lib64
      if [ -f "$MOUNTINFO/dummy/system/lib64" ]; then
        BACKUPLIBS=true
        make_copy_image /system/lib64
        cp -afc /system/lib64/. $MIRRDIR/copy/system/lib64
        cp -afc $DUMMDIR/system/lib64/. $MIRRDIR/copy/system/lib64
        mount_copy_image /system/lib64
        rm -f $MOUNTINFO/dummy/system/lib64
      fi

      # Whole vendor
      if [ -f "$MOUNTINFO/dummy/system/vendor" ]; then
        BACKUPLIBS=true
        LIBSIZE=`du -s /vendor/lib | awk '{print $1}'`
        if [ -d /vendor/lib64 ]; then
          LIB64SIZE=`du -s /vendor/lib64 | awk '{print $1}'`
          VENDORLIBSIZE=$(((($LIBSIZE + $LIB64SIZE) / 10240 + 2) * 10240))
        else
          VENDORLIBSIZE=$((($LIBSIZE / 10240 + 2) * 10240))
        fi
        make_copy_image /vendor $VENDORLIBSIZE

        # Copy lib/lib64
        mkdir -p $MIRRDIR/copy/vendor/lib
        cp -afc /vendor/lib/. $MIRRDIR/copy/vendor/lib
        cp -afc $DUMMDIR/system/vendor/lib/. $MIRRDIR/copy/vendor/lib 2>/dev/null
        if [ -d /vendor/lib64 ]; then
          mkdir -p $MIRRDIR/copy/vendor/lib64
          cp -afc /vendor/lib64/. $MIRRDIR/copy/vendor/lib64
          cp -afc $DUMMDIR/system/vendor/lib64/. $MIRRDIR/copy/vendor/lib64 2>/dev/null
        fi

        cp -afc $DUMMDIR/system/vendor/. $MIRRDIR/copy/vendor

        TRAVEL_ROOT=$MIRRDIR/copy
        (clone_dummy /vendor)
        # Create vendor mirror
        if [ `mount | grep -c "on /vendor type"` -ne 0 ]; then
          VENDORBLOCK=`mount | grep "on /vendor type" | awk '{print $1}'`
          mkdir -p $MIRRDIR/vendor
          mount -o ro $VENDORBLOCK $MIRRDIR/vendor
        else
          ln -s $MIRRDIR/system/vendor $MIRRDIR/vendor
        fi
        mount_copy_image /vendor
        rm -f $MOUNTINFO/dummy/system/vendor
      fi

      # vendor lib
      if [ -f "$MOUNTINFO/dummy/system/vendor/lib" ]; then
        BACKUPLIBS=true
        make_copy_image /system/vendor/lib
        cp -afc /system/vendor/lib/. $MIRRDIR/copy/system/vendor/lib
        cp -afc $DUMMDIR/system/vendor/lib/. $MIRRDIR/copy/system/vendor/lib
        mount_copy_image /system/vendor/lib
        rm -f $MOUNTINFO/dummy/system/vendor/lib
      fi

      # vendor lib64
      if [ -f "$MOUNTINFO/dummy/system/vendor/lib64" ]; then
        BACKUPLIBS=true
        make_copy_image /system/vendor/lib64
        cp -afc /system/vendor/lib64/. $MIRRDIR/copy/system/vendor/lib64
        cp -afc $DUMMDIR/system/vendor/lib64/. $MIRRDIR/copy/system/vendor/lib64
        mount_copy_image /system/vendor/lib64
        rm -f $MOUNTINFO/dummy/system/vendor/lib64
      fi

      # Crash prevention!!
      $BACKUPLIBS && rm -f $COREDIR/magiskhide/enable 2>/dev/null

      # Remove crap folder
      rm -rf $MOUNTPOINT/lost+found
      
      # Start doing tasks
      
      # Stage 1
      TRAVEL_ROOT=$DUMMDIR
      log_print "* Bind mount dummy system"
      find $MOUNTINFO/dummy -type f 2>/dev/null | while read ITEM ; do
        TARGET=${ITEM#$MOUNTINFO/dummy}
        ORIG="$DUMMDIR$TARGET"
        (clone_dummy "$TARGET")
        bind_mount "$ORIG" "$TARGET"
      done

      # Stage 2
      log_print "* Bind mount module items"
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
        log_print "* Enabling systemless hosts file support"
        bind_mount $COREDIR/hosts /system/etc/hosts
      fi

      # Expose busybox
      if [ -f "$COREDIR/busybox/enable" ]; then
        log_print "* Enabling BusyBox"
        cp -afc /data/busybox/. $COREDIR/busybox
        cp -afc /system/xbin/. $COREDIR/busybox
        chmod -R 755 $COREDIR/busybox
        chcon -hR "u:object_r:system_file:s0" $COREDIR/busybox
        bind_mount $COREDIR/busybox /system/xbin
      fi

      # Stage 3
      log_print "* Bind mount system mirror"
      bind_mount /system $MIRRDIR/system

      # Restart post-fs-data if necessary (multirom)
      $MULTIROM && setprop magisk.restart_pfsd 1

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
      log_print "* Removing tampered read-only system props"

      VERIFYBOOT=`getprop ro.boot.verifiedbootstate`
      FLASHLOCKED=`getprop ro.boot.flash.locked`
      VERITYMODE=`getprop ro.boot.veritymode`

      [ ! -z "$VERIFYBOOT" -a "$VERIFYBOOT" != "green" ] && \
      log_print "`$BINPATH/resetprop -v -n ro.boot.verifiedbootstate green`"
      [ ! -z "$FLASHLOCKED" -a "$FLASHLOCKED" != "1" ] && \
      log_print "`$BINPATH/resetprop -v -n ro.boot.flash.locked 1`"
      [ ! -z "$VERITYMODE" -a "$VERITYMODE" != "enforcing" ] && \
      log_print "`$BINPATH/resetprop -v -n ro.boot.veritymode enforcing`"

      mktouch $COREDIR/magiskhide/hidelist
      chmod -R 755 $COREDIR/magiskhide
      # Add Safety Net preset
      $COREDIR/magiskhide/add com.google.android.gms.unstable
      log_print "* Starting Magisk Hide"
      /data/magisk/magiskhide
    fi
    ;;

esac
