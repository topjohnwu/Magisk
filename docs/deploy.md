# Deployment
(Note: This is not a user tutorial for installing Magisk, this is an explaination of how Magisk can be installed, and a guide for developers to properly deploy Magisk in various different situations)

## Systemless
When a user flashes a Magisk zip in custom recoveries or have boot images patched in Magisk Manager, Magisk is installed in the systemless fashion. This is the only officially supported method to install Magisk on a device. The systemless method installs Magisk into a boot image's ramdisk CPIO, sometimes require additional patches to the kernel.

Here are the bare minimum commands to install Magisk into a stock boot/recovery image. Be aware that the actual Magisk installation is a little more complicated, the following commands will work but should be treat as proof-of-concepts.

```
# Push 2 binaries, magiskboot and magiskinit to the device

# Assume the 2 binaries are in the current directory and
# both have executing permissions.

# The path to stock boot image, can be a file or an on-device block
BOOTIMAGE=<path to boot>

# First unpack the image
./magiskboot --unpack $BOOTIMAGE

# In normal cases, after unpacking you should get at least kernel and ramdisk.cpio

# Patch ramdisk
./magiskboot --cpio ramdisk.cpio \
"mkdir 000 .backup" \          # create a folder to store our init backup
"mv init .backup/init" \       # backup the original init
"add 750 init magiskinit"      # replace init with magiskinit

# Patch kernel to always use ramdisk as rootfs
# You only need to do this on system-as-root devices
./magiskboot --hexpatch kernel \
736B69705F696E697472616D6673 \
77616E745F696E697472616D6673

# Repack the boot image
./magiskboot --repack $BOOTIMAGE

# The patched image should be located in new-boot.img
```

## System Only
WIP

```
# Currently not available
```

## Exploits
**(Note: Magisk could only be used as root)**

Occasionally, there would be exploits in certain devices that could lead to full fledged root. On modern Android, it is possible to use MagiskSU if you can gain a shell with the following conditions:

- Effective UID should be privileged (root, or `euid=0`)
- Have the ability to reload `sepolicy` (which 99.9% of the time means SELinux permissive)

Once you got a proper root shell, you should have `magiskinit` somewhere on the device. The basic idea is try to live patch `sepolicy` with `magiskpolicy`, and start `magiskd` with `magisk --daemon`. Here are some examples you could use as a reference.

If dm-verity is enforced (no system r/w allowed)

```
# Assume magiskinit is in current directory
# All commands are required to run on each reboot

# Live patch selinux
ln -s ./magiskinit magiskpolicy
./magiskpolicy --live --magisk "allow magisk * * *"

# Mount tmpfs to /sbin
mount -t tmpfs tmpfs /sbin
chmod 755 /sbin
chcon u:object_r:magisk_file:s0 /sbin

# Add files to /sbin
./magiskinit -x magisk /sbin/magisk
cp -a magiskpolicy /sbin
/sbin/magisk --install /sbin

# Launch magisk daemon
/sbin/magisk --daemon

# (Optional) switch back to enforced
setenforce 1
```

If dm-verity is not enforced (can modify system)

```
# Assume magiskinit is in current directory
# The following commands should only need to run once

# Mount system rw
mount -o rw,remount /system

# Add files to system
./magiskinit -x magisk /system/xbin/magisk
cp -a magiskinit /system/xbin
ln -s /system/xbin/magiskinit /system/xbin/magiskpolicy
/system/xbin/magisk --install /system/xbin

# The following commands should run on each reboot
/system/xbin/magiskpolicy --live --magisk "allow magisk * * *"
/system/xbin/magisk --daemon

```
