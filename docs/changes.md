
# Magisk Changelog

### v26.1

- [App] Fix crashing when revoking root permissions
- [MagiskInit] Always prefer `ext4` partitions over `f2fs` when selecting the pre-init partition
- [General] Restore module files' context/owner/group from mirror. This is a regression introduced in v26.0

### v26.0

- [General] Bump minimum supported Android version to Android 6.0
- [General] New magic mount backend. It supports loading modules into system with `overlayfs` files injected
- [Zygisk] Release new API version 4
- [Zygisk] Prevent crashing daemon in error
- [Zygisk] Rewrite zygote code injection with new loader library approach
- [Zygisk] Rewrite code unloading implementation
- [MagiskBoot] Support amonet microloader devices
- [General] Move the whole logging infrastructure into Rust

### v25.1

- [MagiskBoot] Fix ramdisk backup being incorrectly skipped
- [MagiskBoot] Add new feature to detect unsupported dtb and abort during installation
- [Zygisk] Change binary hijack paths
- [App] Fix incorrect recovery mode detection and installation

pport patching `init_boot.img` for Samsung ODIN firmware
- [MagiskPolicy] Fix minor bug in command line argument parsing
- [MagiskPolicy] Update rules to support Android U

### v25.2

- [MagiskInit] Fix a potential issue when stub cpio is used
- [MagiskInit] Fix reboot to recovery when stub cpio is used
- [MagiskInit] Fix sepolicy.rules symlink for rootfs devices
- [General] Better data encryption detection
- [General] Move the whole logging infrastructure into Rust

### v25.1

- [MagiskBoot] Fix ramdisk backup being incorrectly skipped
- [MagiskBoot] Add new feature to detect unsupported dtb and abort during installation
- [Zygisk] Change binary hijack paths
- [App] Fix incorrect recovery mode detection and installation
