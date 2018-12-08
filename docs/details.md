# Magisk Details
## File Structure
### Paths in "sbin tmpfs overlay"
sbin tmpfs overlay is the key to hiding Magisk from detection. All Magisk binaries, applets, mirrors, mountpoints, loop devices, and other trivial stuffs are all located in the `tmpfs` mounted on `/sbin`. MagiskHide can just simply unmount `/sbin` and the bind mounts into `/system` and `/vendor` to hide all modifications easily. 

```
# The actual Magisk binary
/sbin/magisk.bin
# Wrapper script to prevent stupid linker errors
/sbin/magisk

# Other binaries like magiskinit, and all symlinks to
# applets are also directly stored in /sbin, so they
# are all in PATH for apps and shell to access them

# Other crazy stuffs are stored in this directory
MAGISKTMP=/sbin/.core

# Magisk BusyBox path
BBPATH=$MAGISKTMP/busybox

# The mountpoint of magisk.img
MOUNTPATH=$MAGISKTMP/img

# Mount mirrors for MagicMount
MIRRORDIR=$MAGISKTMP/mirror
# System mirror
SYSTEMMIR=$MIRRORDIR/system
# Vendor mirror, could be a symlink to $SYSTEMMIR/vendor
# if vendor is not a separate partition
VENDORMIR=$MIRRORDIR/vendor
# Mirror to magisk's persist binary directory
BINMIRROR=$MIRRORDIR/bin

# Database storing settings and root permissons
# Each user (in the case of multiuser) has a different path
MAGISKDB_MANAGER=$MAGISKTMP/db-<USER>/magisk.db

# Loop devices to mount ext4 images, the ending number will vary
MAGISKLOOP=$MAGISKTMP/block/loop<XX>
```

### Paths in `/data`
Some binaries and files should be stored on non-volatile storages in `/data`. In order to prevent detection from crazy apps, everything has to be stored somewhere safe and undetectable in `/data`. The folder `/data/adb` was chosen because of the following advantages:

- It is an existing folder on modern Android, so it cannot be used as an indication of the existence of Magisk.
- The permission of the folder is by default `700`, owner as `root`, so non-root processes are unable to enter, read, write the folder in any possible way.
- The folder is labeled with secontext `u:object_r:adb_data_file:s0`, and very few processes have the permission to do any interaction with that secontext.
- The folder is located in *Device encrypted storage*, so it is accessible as soon as data is properly mounted in FBE (File-Based Encryption) devices.

There are some drawbacks though (only applies to FBE setups):

- Since the folder is encrypted, it is inaccessible in custom recoveries without any form of decryption (very rare case but still exists).
- FBE is very finicky. If you wiped `/data` in TWRP (which wipes all encryption) and manually create folders that are meant to be encrypted without special treatment, it will result in bootloop.

The solution of both situations above is to use `/data/magisk` when `/data/adb` does not exist. Magisk will handle migration at boot and properly setup the files to the expected location.

```
SECURE_DIR=/data/adb

# Magisk image, storing modules and scripts
MAINIMG=$SECURE_DIR/magisk.img

# The image to store updated modules when installing
# in Magisk Manager since contents in $MAINIMG are not
# safe to be modified live. This image will be merged to
# $MAINIMG in the next reboot
MERGEIMG=$SECURE_DIR/magisk_merge.img

# Database storing settings and root permissions
# This file will be bind mounted to $MAGISKDB_MANAGER
MAGISKDB=$SECURE_DIR/magisk.db

# All magisk related binaries, containing busybox,
# scripts, and magisk binaries. Used in supporting
# module installation, addon.d, Magisk Manager etc.
# This folder will be bind mounted to $BINMIRROR
DATABIN=$SECURE_DIR/magisk

# The location to store simple_mount files
SIMPLEMOUNT=$SECURE_DIR/magisk_simple
```

### Paths in `$MAINIMG`
Each folder in `$MAINIMG` is a Magisk module, except the folder `.core` which stores files that are unrelated to any modules.

```
# The directory storing all non-module files
COREDIR=$MOUNTPATH/.core

# MagiskHide hidelist
HIDELIST=$COREDIR/hidelist

# Folder storing scripts that should be executed in post-fs-data mode
POSTFSDATA_DIR=$COREDIR/post-fs-data.d

# Folder storing scripts that should be executed in service mode
SERVICE_DIR=$COREDIR/service.d
```

### Final Words
The file structure of Magisk is designed in a weird and overly complicated way. But all of these quirks are done to properly support hiding modifications from detection. These design choices are mostly what makes Magisk difficult to implement properly and maintain.

## Magisk Booting Process
### Pre-Init
`magiskinit` will replace `init` as the first program to run. It is responsible for constructing rootfs on system-as-root devices: it parses kernel cmdline, sysfs, device tree fstabs, uevents etc., recreating **early-mount** and clones rootfs files from the system. On traditional devices, it will simply revert `init` to the original one and continue on to the following steps.

It then extracts `init.magisk.rc` and inject imports in `init.rc` for additional startup services, extracts `magisk` to `/sbin`, compile split sepolicy and patch `init` to always load `/sepolicy` instead of other sources (only on Treble devices), and patches `/sepolicy` with the additional rules. Finally, it will execute the original `init` to start the ordinary boot process.

### Startup
This triggers on `post-fs-data` when `/data` is properly decrypted (if required) and mounted. The command `/sbin/magisk --starup` is executed by `init`. The startup stage will remove all traces of Magisk in ramdisk, and do the extremely complicated initialization for sbin tmpfs overlay. After the setup is done, it will execute `/sbin/magisk --post-fs-data` to switch to the `magisk` binary located in `tmpfs` and start "post-fs-data" mode.

### post-fs-data
In this mode, the daemons, `magiskd` and `magisklogd`, will be launched. `$MAINIMG` is resized / merged and mounted to `$MOUNTPOINT`, post-fs-data scripts are executed, and module files are magic mounted.

### late_start
Later in the booting process, the class `late_start` will be triggered, and Magisk "service" mode will be started. In this mode, service scripts are executed, and it will try to install Magisk Manager if it doesn't exist.

## Resetprop
Usually, system properties are designed to only be updated by a single `init` process and read-only to non-root processes. With root you can change properties by sending requests via `property_service` using commands such as `setprop`, but you are still prohibited from changing read-only props (props that start with `ro.` like `ro.build.product`) and deleting properties.

`resetprop` is implemented by distilling out the source code related to system properties from AOSP with modifications to map the property area, or `prop_area`, r/w and some clever hacks to modify the trie structure in ways it wasn't intended, like detaching nodes. In a nut shell, it directly do modifications to `prop_area`, bypassing the need to go through `property_service`. Since we are bypassing `property_service`, there are a few caveats:

- `on property:foo=bar` actions registered in `*.rc` scripts will not be triggered if property changes does not go through `property_service`. The default set property behavior of `resetprop` matches `setprop`, which **WILL** trigger events (implemented by first deleting the property then set it via `property_service`), but there is a flag `-n` to disable it if you need this special behavior.
- persist properties (props that starts with `persist.`, like `persist.sys.usb.config`) are stored in both `prop_area` and `/data/property`. By default, deleting props will **NOT** remove it from persistent storage, meaning the property will be restored after the next reboot; reading props will **NOT** read from persistent storage, as this is the behavior of `getprop`. With the flag `-p`, deleting props will remove the prop in **BOTH** `prop_area` and `/data/property`, and reading props will be read from **BOTH** `prop_area` and persistent storage.

## Magic Mount
I will skip the details in the actual implementation of how Magic Mount works as it will become a lecture, but you can always directly dive into the source code if interested. (`bootstages.c`)

Even though the mounting logic and traversal algorithm is pretty complicated, the final result of Magic Mount is actually pretty simple. For each module, the folder `$MODPATH/system` will be recursively merged into the real `/system`; that is: existing files in the real system will be replaced by the one in modules' system, and new files in modules' system will be added to the real system.

There is one additional trick you can use: if you place an empty file named `.replace` in any of the folders in a module's system, instead of merging the contents, that folder will directly replace the one in the real system. This will be very handy in some cases, for example swapping out a system app.

If you want to replace files in `/vendor`, please place it under `$MODPATH/system/vendor`. Magisk will transparently handle both cases, whether vendor is a separate partition or not.

## Simple Mount
**(Note: this implementation is and will not be fully tested, your mileage may vary)**

Some files might have to be mounted earlier in the boot process like bootanimations. To replace it with Magisk, all you need to do is simply place the modified file into the corresponding path under `$SIMPLEMOUNT`. For example, your goal is to replace `/system/media/bootanimation.zip`; you only need to copy the new zip to `$SIMPLEMOUNT/system/media/bootanimation.zip` and you're done! Magisk will clone all the attributes from the target file to your replaced file before bind mounting, so you don't need to worry about it.

## Miscellaneous
Here are some tidbits in Magisk but unable to be categorized into any sections:

- Socket name randomization: when you call `su`, `magiskhide`, and some commands in `magisk`, it connects to the magisk daemon `magiskd` running in the background. The connections are established through an abstract Unix socket. Any process can go through all active Unix sockets and see if the specifc name used by Magisk is in the list to determine whether `magiskd` is running. Starting from v15.4, the abstract name used in `magiskd` and `magisklogd` are randomized by `magiskinit` on each boot.
- Sevice name randomization: each service started up by `init` will be recorded. Some apps will detect the name of magisk boot services to determine whether Magisk is installed. Starting from v17.2, the service name assigned in `init.magisk.rc` is randomized by `magiskinit`.
