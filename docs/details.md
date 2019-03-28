# Magisk Details
## File Structure
### Paths in "sbin tmpfs overlay"
sbin tmpfs overlay is the key to hiding Magisk from detection. All Magisk binaries, applets, mirrors, and other trivial stuffs are all located in the `tmpfs` mounted on `/sbin`. MagiskHide can just simply unmount `/sbin` and the bind mounts to hide all modifications easily.

```
# Binaries like magisk, magiskinit, and all symlinks to
# applets are directly stored in /sbin, so they
# are all in PATH for apps and shell to access them

# Magisk internal stuffs
MAGISKTMP=/sbin/.magisk

# Magisk BusyBox path
BBPATH=$MAGISKTMP/busybox

# /data/adb/modules will be bind mounted here
$MAGISKTMP/modules

# The configuration used in last installation
$MAGISKTMP/config

MIRRORDIR=$MAGISKTMP/mirror

# System mirror
$MIRRORDIR/system

# Vendor mirror, could be a symlink to $SYSTEMMIR/vendor
# if vendor is not a separate partition
$MIRRORDIR/vendor

# Data mirror to workaround nosuid flag
$MIRRORDIR/data
```

### Paths in `/data`
Some binaries and files should be stored on non-volatile storages in `/data`. In order to prevent detection, everything has to be stored somewhere safe and undetectable in `/data`. The folder `/data/adb` was chosen because of the following advantages:

- It is an existing folder on modern Android, so it cannot be used as an indication of the existence of Magisk.
- The permission of the folder is by default `700`, owner as `root`, so non-root processes are unable to enter, read, write the folder in any possible way.
- The folder is labeled with secontext `u:object_r:adb_data_file:s0`, and very few processes have the permission to do any interaction with that secontext.
- The folder is located in *Device encrypted storage*, so it is accessible as soon as data is properly mounted in FBE (File-Based Encryption) devices.

```
SECURE_DIR=/data/adb

# Folder storing general post-fs-data scripts
$SECURE_DIR/post-fs-data.d

# Folder storing general late_start service scripts
$SECURE_DIR/service.d

# Magisk modules
$SECURE_DIR/modules

# Magisk modules that are pending for upgrade
# Module files are not safe to be modified when mounted
# Modules installed in Magisk Manager will be stored here
# and will be merged into $SECURE_DIR/modules in the next reboot
$SECURE_DIR/modules_update

# Database storing settings and root permissions
MAGISKDB=$SECURE_DIR/magisk.db

# All magisk related binaries, containing busybox,
# scripts, and magisk binaries. Used in supporting
# module installation, addon.d, Magisk Manager etc.
# This folder will be bind mounted to $BINMIRROR
DATABIN=$SECURE_DIR/magisk

```

### Final Words
The file structure of Magisk is designed in a weird and overly complicated way. But all of these quirks are done to properly support hiding modifications from detection. These design choices are mostly what makes Magisk difficult to implement properly and maintain.

## Magisk Booting Process
### Pre-Init
`magiskinit` will replace `init` as the first program to run. It is responsible for constructing rootfs on system-as-root devices: it parses kernel cmdline, sysfs, device tree fstabs, uevents etc., recreating **early-mount** and clones rootfs files from the system. On traditional devices, it will simply revert `init` to the original one and continue on to the following steps.

- Inject magisk services into `init.rc`
- Load sepolicy either from `/sepolicy`, precompiled sepolicy in vendor, or compile split sepolicy
- Patch sepolicy rules and dump to `/sepolicy` and patch `init` to always load `/sepolicy`
- Fork a new daemon and wait for early-init trigger
- Execute the original `init` to start the ordinary boot process
- The early-init daemon will construct `/sbin` `tmpfs` overlay and remove all traces of Magisk in ramdisk

### post-fs-data
This triggers on `post-fs-data` when `/data` is properly decrypted (if required) and mounted. The daemon `magiskd` will be launched, post-fs-data scripts are executed, and module files are magic mounted.

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

## Miscellaneous
Here are some tidbits in Magisk but unable to be categorized into any sections:

- Socket name randomization: when you call `su`, `magiskhide`, and some commands in `magisk`, it connects to the magisk daemon `magiskd` running in the background. The connections are established through an abstract Unix socket. Any process can go through all active Unix sockets and see if the specifc name used by Magisk is in the list to determine whether `magiskd` is running. Starting from v15.4, the abstract name used in `magiskd` and `magisklogd` are randomized by `magiskinit` on each boot.
- Sevice name randomization: each service started up by `init` will be recorded. Some apps will detect the name of magisk boot services to determine whether Magisk is installed. Starting from v17.2, the service name assigned in `init.magisk.rc` is randomized by `magiskinit`.
