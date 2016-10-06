#!/system/bin/sh

LOGFILE=/cache/magisk.log
IMG=/data/magisk.img
MOUNTLIST=/dev/mountlist

MOUNTPOINT=/magisk

COREDIR=$MOUNTPOINT/.core

DUMMDIR=$COREDIR/dummy
MIRRDIR=$COREDIR/mirror

TMPDIR=/dev/tmp

# Use the included busybox to do everything for maximum compatibility
# We also do so because we rely on the option "-c" for cp (reserve contexts)

# Reserve the original PATH
export OLDPATH=$PATH
export PATH="/data/busybox:$PATH"

log_print() {
  echo $1
  echo $1 >> $LOGFILE
  log -p i -t Magisk "$1"
}

mktouch() {
  mkdir -p ${1%/*} 2>/dev/null
  if [ -z "$2" ]; then
    touch $1 2>/dev/null
  else
    echo $2 > $1 2>/dev/null
  fi
}

unblock() {
  touch /dev/.magisk.unblock
  exit
}

run_scripts() {
  BASE=$MOUNTPOINT
  if [ "$1" = "post-fs" ]; then
    BASE=/cache/magisk
  fi

  for MOD in $BASE/* ; do
    if [ ! -f "$MOD/disable" ]; then
      if [ -f "$MOD/$1.sh" ]; then
        chmod 755 $MOD/$1.sh
        chcon 'u:object_r:system_file:s0' $MOD/$1.sh
        log_print "$1: $MOD/$1.sh"
        $MOD/$1.sh
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
  cd $1/$2
  if [ -f ".replace" ]; then
    rm -rf $TMPDIR/$2
    mktouch $TMPDIR/$2 $1
  else
    for ITEM in * ; do
      if [ ! -e "/$2/$ITEM" ]; then
        # New item found
        if [ $2 = "system" ]; then
          # We cannot add new items to /system root, delete it
          rm -rf $ITEM
        else
          if [ -d "$TMPDIR/dummy/$2" ]; then
            # We are in a higher level, delete the lower levels
            rm -rf $TMPDIR/dummy/$2
          fi
          # Mount the dummy parent
          mktouch $TMPDIR/dummy/$2

          mkdir -p $DUMMDIR/$2 2>/dev/null
          if [ -d "$ITEM" ]; then
            # Create new dummy directory
            mkdir -p $DUMMDIR/$2/$ITEM
          elif [ -L "$ITEM" ]; then
            # Symlinks are small, copy them
            cp -afc $ITEM $DUMMDIR/$2/$ITEM
          else
            # Create new dummy file
            mktouch $DUMMDIR/$2/$ITEM
          fi

          # Clone the original /system structure (depth 1)
          if [ -e "/$2" ]; then
            for DUMMY in /$2/* ; do
              if [ -d "$DUMMY" ]; then
                # Create dummy directory
                mkdir -p $DUMMDIR$DUMMY
              elif [ -L "$DUMMY" ]; then
                # Symlinks are small, copy them
                cp -afc $DUMMY $DUMMDIR$DUMMY
              else
                # Create dummy file
                mktouch $DUMMDIR$DUMMY
              fi
            done
          fi
        fi
      fi

      if [ -d "$ITEM" ]; then
        # It's an directory, travel deeper
        (travel $1 $2/$ITEM)
      elif [ ! -L "$ITEM" ]; then
        # Mount this file
        mktouch $TMPDIR/$2/$ITEM $1
      fi
    done
  fi
}

bind_mount() {
  if [ -e "$1" -a -e "$2" ]; then
    mount -o bind $1 $2
    if [ "$?" -eq "0" ]; then 
      log_print "Mount: $1"
      echo $2 >> $MOUNTLIST
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
              rm -rf /cache/data_img/$MOD
              cp -afc $MOD /cache/data_img/
            done
            log_print "Merge complete"
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
    log_print "Magisk post-fs mode running..."

    if [ -d "/cache/magisk_merge" ]; then
      cd /cache/magisk_merge
      for MOD in *; do
        rm -rf /cache/magisk/$MOD
        cp -afc $MOD /cache/magisk/
      done 
      rm -rf /cache/magisk_merge
    fi

    for MOD in /cache/magisk/* ; do
      if [ -f "$MOD/remove" ]; then
        log_print "Remove module: $MOD"
        rm -rf $MOD
      elif [ -f "$MOD/auto_mount" -a ! -f "$MOD/disable" ]; then
        find $MOD/system -type f 2>/dev/null | while read ITEM ; do
          TARGET=${ITEM#$MOD}
          bind_mount $ITEM $TARGET
        done
      fi
    done

    run_scripts post-fs
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

    log_print "Magisk post-fs-data mode running..."

    # Live patch sepolicy
    /data/magisk/sepolicy-inject --live -s su

    [ ! -d "$MOUNTPOINT" ] && mkdir -p $MOUNTPOINT

    # Cache support
    if [ -d "/cache/data_bin" ]; then
      rm -rf /data/busybox /data/magisk
      mkdir -p /data/busybox
      mv /cache/data_bin /data/magisk
      chmod 755 /data/busybox /data/magisk /data/magisk/*
      chcon 'u:object_r:system_file:s0' /data/busybox /data/magisk /data/magisk/*
      /data/magisk/busybox --install -s /data/busybox
      # Prevent issues
      rm -f /data/busybox/su /data/busybox/sh
    fi
    mv /cache/stock_boot.img /data 2>/dev/null
    
    chmod 644 $IMG /cache/magisk.img /data/magisk_merge.img 2>/dev/null

    # Handle image merging
    merge_image /cache/magisk.img
    merge_image /data/magisk_merge.img

    # Mount magisk.img
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

    log_print "Preparing modules"
    # First do cleanups
    rm -rf $DUMMDIR
    rmdir $(find $MOUNTPOINT -type d -depth ! -path "*core*" ) 2>/dev/null
    rm -rf $COREDIR/bin

    mkdir -p $DUMMDIR
    mkdir -p $MIRRDIR/system

    # Travel through all mods
    for MOD in $MOUNTPOINT/* ; do
      if [ -f "$MOD/remove" ]; then
        log_print "Remove module: $MOD"
        rm -rf $MOD
      elif [ -f "$MOD/auto_mount" -a -d "$MOD/system" -a ! -f "$MOD/disable" ]; then
        (travel $MOD system)
      fi
    done

    # Proper permissions for generated items
    chmod 755 $(find $COREDIR -type d)
    chmod 644 $(find $COREDIR -type f)
    find $COREDIR -type d -exec chmod 755 {} \;
    find $COREDIR -type f -exec chmod 644 {} \;

    # linker(64), t*box, and app_process* are required if we need to dummy mount bin folder
    if [ -f "$TMPDIR/dummy/system/bin" ]; then
      rm -f $DUMMDIR/system/bin/linker* $DUMMDIR/system/bin/t*box $DUMMDIR/system/bin/app_process*
      cd /system/bin
      cp -afc linker* t*box app_process* $DUMMDIR/system/bin/
    fi

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
    fi

    if [ `cat /proc/mounts | grep $MOUNTPOINT >/dev/null 2>&1; echo $?` -ne 0 ]; then
      log_print "magisk.img mount failed, nothing to do :("
      unblock
    fi

    # Remove crap folder
    rm -rf $MOUNTPOINT/lost+found
    
    # Start doing tasks
    
    # Stage 1
    log_print "Bind mount dummy system"
    find $TMPDIR/dummy -type f 2>/dev/null | while read ITEM ; do
      TARGET=${ITEM#$TMPDIR/dummy}
      ORIG=$DUMMDIR$TARGET
      bind_mount $ORIG $TARGET
    done

    # Stage 2
    log_print "Bind mount module items"
    find $TMPDIR/system -type f 2>/dev/null | while read ITEM ; do
      TARGET=${ITEM#$TMPDIR}
      ORIG=`cat $ITEM`$TARGET
      bind_mount $ORIG $TARGET
      rm -f $DUMMDIR${TARGET%/*}/.dummy 2>/dev/null
    done

    # Run scripts
    run_scripts post-fs-data

    # Bind hosts for Adblock apps
    [ ! -f "$COREDIR/hosts" ] && cp -afc /system/etc/hosts $COREDIR/hosts
    log_print "Enabling systemless hosts file support"
    bind_mount $COREDIR/hosts /system/etc/hosts

    # Stage 3
    log_print "Bind mount system mirror"
    bind_mount /system $MIRRDIR/system

    # Stage 4
    log_print "Bind mount mirror items"
    # Find all empty directores and dummy files, they should be mounted by original files in /system
    find $DUMMDIR -type d -exec sh -c '[ -z "$(ls -A $1)" ] && echo $1' -- {} \; -o \( -type f -size 0 -print \) | while read ITEM ; do
      ORIG=${ITEM/dummy/mirror}
      TARGET=${ITEM#$DUMMDIR}
      bind_mount $ORIG $TARGET
    done

    # All done
    rm -rf $TMPDIR

    unblock
    ;;

  service )
    # Version info
    setprop magisk.version 7
    log_print "Magisk late_start service mode running..."
    run_scripts service

    # Enable magiskhide
    [ ! -f "$COREDIR/magiskhide/hidelist" ] && mktouch $COREDIR/magiskhide/hidelist
    # Add preset for Safety Net
    if [ $(grep -c "com.google.android.gms.unstable" $COREDIR/magiskhide/hidelist) -eq "0" ]; then
      mv $COREDIR/magiskhide/hidelist $COREDIR/magiskhide/hidelist.tmp
      echo "com.google.android.gms.unstable" > $COREDIR/magiskhide/hidelist
      cat $COREDIR/magiskhide/hidelist.tmp >> $COREDIR/magiskhide/hidelist
    fi
    log_print "Starting Magisk Hide"
    (/data/magisk/magiskhide &)

    ;;

esac
