# Internal Details

## File Structure

### Paths in "Magisk tmpfs directory"

Magisk will mount a `tmpfs` directory to store some temporary data. For devices with the `/sbin` folder, it will be chosen as it will also act as an overlay to inject binaries into `PATH`. From Android 11 onwards, the `/sbin` folder might not exist, so Magisk will use `/debug_ramdisk` as the base folder.

```
# In order to get the current base folder Magisk is using,
# use the command `magisk --path`.
# Binaries like magisk, magiskinit, and all symlinks to
# applets are directly stored in this path. This means when
# this is /sbin, these binaries will be directly in PATH.
MAGISKTMP=$(magisk --path)

# Magisk internal stuffs
INTERNALDIR=$MAGISKTMP/.magisk

# /data/adb/modules will be bind mounted here as read-only.
# The original folder is not used directly due to the nosuid mount flag on /data.
$INTERNALDIR/modules

# The current Magisk installation config
$INTERNALDIR/config

# Symlink/mount to the pre-init storage directory
# Used to persist custom sepolicy.rule across reboots so it can be read during pre-init
$INTERNALDIR/preinit

# Internal tmpfs workspace used for constructing magic mount trees
$INTERNALDIR/worker

# Internal BusyBox directory containing busybox and its applet symlinks
$INTERNALDIR/busybox

# Internal device nodes, pipes, and sockets (magiskd socket, preinit block device, log pipe)
$INTERNALDIR/device

# Root directory patch/overlay files
# On system-as-root devices, / is not writable.
# All pre-init patched files are stored here and bind mounted.
$INTERNALDIR/rootdir

# Isolated devpts mount for root shells (if supported by the kernel)
$INTERNALDIR/pts
```

### Paths in `/data`

Some binaries and files should be stored on non-volatile storages in `/data`. In order to prevent detection, everything has to be stored somewhere safe and undetectable in `/data`. The folder `/data/adb` was chosen because of the following advantages:

- It is an existing folder on modern Android, so it cannot be used as an indication of the existence of Magisk.
- The permission of the folder is by default `700`, owner as `root`, so non-root processes are unable to enter, read, write the folder in any possible way.
- The folder is labeled with secontext `u:object_r:adb_data_file:s0`, and very few processes have the permission to do any interaction with that secontext.
- The folder is located in _Device encrypted storage_, so it is accessible as soon as data is properly mounted in FBE (File-Based Encryption) devices.

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
# Modules installed through the Magisk app will be stored here
# and will be merged into $SECURE_DIR/modules in the next reboot
$SECURE_DIR/modules_update

# Database storing settings and root permissions
MAGISKDB=$SECURE_DIR/magisk.db

# All magisk related binaries, including busybox,
# scripts, and magisk binaries. Used in supporting
# module installation, addon.d, the Magisk app etc.
DATABIN=$SECURE_DIR/magisk
```

## Magisk Booting Process

### Pre-Init

`magiskinit` replaces `init` as the first program to run.

- Early mount required partitions (`sysfs`, `procfs`, `devtmpfs`, and the pre-init partition if available).
- On legacy system-as-root devices, `magiskinit` switches root to `/system`. On 2-Stage Init (2SI) devices, `magiskinit` intercepts first-stage `init`'s root switch (`SwitchRoot`) to `/system` by bind-mounting `magiskinit` over `/system/bin/init` (using a `/sdcard` -> `/storage/self/primary` -> `/system/system/bin/init` symlink trick in ramfs, or falling back to hex-patching `/init` to redirect `/system/bin/init` to `/data/magiskinit`), allowing the original first-stage `init` to mount dynamic partitions for us before handing control back to `magiskinit` as the second-stage `init`.
- Inject Magisk services into `init.rc` and apply any `overlay.d` root directory replacements/mounts.
- Intercept and patch SELinux policy (incorporating built-in Magisk rules and any custom `sepolicy.rule` files from the pre-init partition) using one of three strategies depending on the device's architecture:
  - **2SI (Android 10+)**: Sets `LD_PRELOAD` (`init-ld`) on `init` to hook `security_load_policy` and intercept policy loading directly in memory before it is written to the kernel.
  - **Treble / Split Policy (Android 8.0–9.0)**: Mounts `selinuxfs` early and replaces `/sys/fs/selinux/load` and `/sys/fs/selinux/enforce` with custom nodes (using a FIFO to pause `init` until `magiskinit` finishes patching and loading the policy into the kernel).
  - **Legacy (Android 7.0–7.1)**: Replaces `/selinux_version` (or `/sepolicy_version`) with a FIFO to pause `init` right after it mounts `selinuxfs`, then replaces `/sys/fs/selinux/load` and `/sys/fs/selinux/enforce` with custom nodes in the same way as the Treble strategy.
- Restore the original `init` and execute it to continue the boot process.

### post-fs-data

This triggers on `post-fs-data` when `/data` is decrypted and mounted. The daemon `magiskd` is launched, Safe Mode is evaluated (if the device failed to reach `boot-complete` twice in a row, or if Safe Mode was triggered via key combo or the Magisk app, all modules and Zygisk are disabled), `post-fs-data` scripts and module `system.prop` files are processed, and enabled module files are magic mounted.

### late_start

Later in the booting process, the class `late_start` will be triggered, and Magisk "service" mode will be started. In this mode, general and module `service.sh` scripts are executed.

### boot-complete

When `sys.boot_completed=1` is triggered, Magisk resets the bootloop counter, mounts/symlinks the writable `preinit` storage directory for future module installations, and finalizes Zygisk state.

## Resetprop

Usually, system properties are designed to only be updated by `init` and read-only to non-root processes. With root you can change properties by sending requests to `property_service` (hosted by `init`) using commands such as `setprop`, but changing read-only props (props that start with `ro.` like `ro.build.product`) and deleting properties are still prohibited.

`resetprop` is implemented by distilling out the source code related to system properties from AOSP and patched to allow direct modification to property area, or `prop_area`, bypassing the need to go through `property_service`. Since we are bypassing `property_service`, there are a few caveats:

- `on property:foo=bar` actions registered in `*.rc` scripts will not be triggered if property changes do not go through `property_service`. For non-read-only properties, the default set property behavior of `resetprop` goes through `property_service` (matching `setprop`), which **WILL** trigger events. Modifying read-only (`ro.*`) properties or passing the `-n` flag bypasses `property_service` and modifies `prop_area` directly, which will **NOT** trigger property events.
- persist properties (props that start with `persist.`, like `persist.sys.usb.config`) are stored in both `prop_area` and `/data/property`. By default, deleting props will **NOT** remove them from persistent storage, meaning the property will be restored after the next reboot; reading props will **NOT** read from persistent storage, as this is the behavior of `getprop`. With the flag `-p`, deleting props will remove the prop in **BOTH** `prop_area` and `/data/property`, and reading props will read from **BOTH** `prop_area` and persistent storage.

## SELinux Policies

Magisk patches the stock `sepolicy` to ensure root and Magisk operations can be performed safely and reliably:

- `magisk` (`u:r:magisk:s0`): An unconstrained, permissive domain that `magiskd` and all root shells run in.
- `magisk_file` (`u:object_r:magisk_file:s0`): An unrestricted file type accessible by all domains, used to label Magisk sockets, binaries, and internal nodes.
- `magisk_log_file` (`u:object_r:magisk_log_file:s0`): A specialized FIFO file type for the Magisk log pipe that all domains are allowed to write to, while only `zygote` (and `magisk`) can open and read.
- Selected client domains (`zygote`, `shell`, `platform_app`, `system_app`, `priv_app`, `untrusted_app`, `untrusted_app_all`) are permitted to connect to `magisk` Unix stream sockets (used for `su`, `magisk`, and Zygisk IPC) and share file descriptors for root shell pseudo-terminals (`devpts`).

The full set of built-in policy rules can be found in `native/src/sepolicy/rules.rs`.

