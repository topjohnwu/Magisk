# Magisk Changelog

### v30.1 (2025.7.3)

- [Core] Fix bug in module mounting implementation
- [MagiskSU] Add ability to restrict Linux capabilities even if running as root (uid=0)

### v30.0 (2025.7.1)

- [General] Various minor bug fixes
- [Core] Migrate module implementation to Rust
- [Core] Improve Magisk specific files injection logic
- [MagiskBoot] Migrate compression code to Rust

### v29.0 (2025.5.14)

- [General] Massive internal refactoring and code migration
- [App] Support downloading module zip files with XZ compression
- [App] Disable app animations when system animations are disabled
- [Core] Support systemlessly deleting files with modules using blank file nodes
- [MagiskInit] Redesign sepolicy patching and injection logic
- [MagiskSU] Better TTY/PTY support

### v28.1 (2024.12.6)

- [App] Fix stub APK download link
- [App] Fix support for Android lower than 8.0
- [General] Fix support for MTK Samsung devices
- [MagiskInit] Fix a regression for 2SI devices
- [MagiskPolicy] Fix a regression causing `overlay.d` replaced files to be not accessible

### v28.0 (2024.10.10)

- [General] Support 16k page size
- [General] Add basic support for RISC-V (not built in releases)
- [General] Use a minimal libc to build static executables (`magiskinit` and `magiskboot`) for smaller sizes
- [Core] Remove unnecessary mirror for magic mount
- [Core] Update boot image detection logic to support more devices
- [MagiskInit] Rewrite 2SI logic for injecting `magiskinit` as `init`
- [MagiskInit] Update preinit partition detection
- [Zygisk] Update internal JNI hooking implementation
- [MagiskPolicy] Preserve sepolicy config flag after patching
- [MagiskPolicy] Optimize patching rules to reduce the amount of new rules being injected
- [DenyList] Support enforcing denylist when Zygisk is disabled
- [Resetprop] Improve implementation to workaround several property modification detections
- [Resetprop] Update to properly work with property overlays
- [App] Major internal code refactoring
- [App] Support patching Samsung firmware with images larger than 8GiB
- [App] Use user-initiated job instead of foreground services on Android 14
- [App] Support Android 13+ built-in per-app language preferences
- [App] Add `action.sh` support to allow modules to define an action triggered from UI
- [MagiskBoot] Support spliting kernel images without decompression
- [MagiskBoot] Properly support vendor boot images
- [MagiskBoot] Disable Samsung PROCA from kernel image

### v27.0 (2024.2.3)

- [Zygisk] Introduce new code injection mechanism
- [Zygisk] Support new signature introduced in U QPR2
- [SEPolicy] Update libsepol to properly set some policy config bits
- [MagiskBoot] Support compressing `init` so Magisk is installable on devices with small boot partitions
- [ResetProp] Add new wait for property feature `resetprop -w`

### v26.4 (2023.11.5)

- [MagiskBoot] Don't pad zeros if signed boot image is larger
- [MagiskPolicy] Fix `genfscon` and `filename_trans`
- [MagiskPolicy] Fix bug in `libsepol`
- [Zygisk] Fix and simplify file descriptor sanitization logic
- [App] Prevent OOM when patching AP tarfiles
- [App] Fix bug in device configuration detection
- [Daemon] Fix certificate parsing of APKs
- [General] Fix logging errors from C++ code being ignored

### v26.3 (2023.9.4)

- [General] Fix device information detection script
- [General] Update BusyBox to 1.36.1
- [General] Update toolchain that produces broken arm32 executables
- [App] Fix root service unable to bind on OnePlus devices

### v26.2 (2023.8.27)

- [MagiskBoot] Support extracting boot image from `payload.bin`
- [MagiskBoot] Support cpio files containing character files
- [MagiskBoot] Support listing cpio content
- [MagiskBoot] Directly handle AVB 1.0 signing and verification without going through Java implementation
- [Daemon] Make daemon socket a fixed path in MAGISKTMP
- [resetprop] Support printing property context
- [resetprop] Support only printing persistent properties from storage
- [resetprop] Properly support setting persistent properties bypassing property_service
- [MagiskSU] Support `-g` and `-G` options
- [MagiskSU] Support switching mount namespace to PID with `-t`
- [MagiskPolicy] Fix patching extended permissions
- [MagiskPolicy] Support more syntax for extended permissions
- [MagiskPolicy] Support printing out the loaded sepolicy rules
- [App] Support patching boot image from ROM zips
- [App] Properly preserve `boot.img` when patching Samsung firmware with `init_boot.img`

### v26.1 (2023.4.11)

- [App] Fix crashing when revoking root permissions
- [MagiskInit] Always prefer `ext4` partitions over `f2fs` when selecting the pre-init partition
- [General] Restore module files' context/owner/group from mirror. This is a regression introduced in v26.0

### v26.0 (2023.4.5)

- [General] Bump minimum supported Android version to Android 6.0
- [General] New magic mount backend. It supports loading modules into system with `overlayfs` files injected
- [Zygisk] Release new API version 4
- [Zygisk] Prevent crashing daemon in error
- [Zygisk] Rewrite zygote code injection with new loader library approach
- [Zygisk] Rewrite code unloading implementation
- [MagiskBoot] Support amonet microloader devices
- [MagiskBoot] Always use lz4_legacy compression on v4 boot images. This fixes boot image patching issues on Android U preview.
- [MagiskInit] Support replacing existing \*.rc files in `overlay.d`
- [MagiskInit] Rewrite sepolicy.rules mounting and loading implementation
- [App] Make stub patching 100% offline
- [App] Support patching `init_boot.img` for Samsung ODIN firmware
- [MagiskPolicy] Fix minor bug in command line argument parsing
- [MagiskPolicy] Update rules to support Android U

### v25.2 (2022.7.20)

- [MagiskInit] Fix a potential issue when stub cpio is used
- [MagiskInit] Fix reboot to recovery when stub cpio is used
- [MagiskInit] Fix sepolicy.rules symlink for rootfs devices
- [General] Better data encryption detection
- [General] Move the whole logging infrastructure into Rust

### v25.1 (2022.6.19)

- [MagiskBoot] Fix ramdisk backup being incorrectly skipped
- [MagiskBoot] Add new feature to detect unsupported dtb and abort during installation
- [Zygisk] Change binary hijack paths
- [App] Fix incorrect recovery mode detection and installation
- [MagiskInit] Fix config not properly exported in legacy SAR devices
- [General] Enforce the Magisk app to always match or be newer than `magiskd`

### v25.0 (2022.6.7)

- [MagiskInit] Update 2SI implementation, significantly increase device compatibility (e.g. Sony Xperia devices)
- [MagiskInit] Introduce new `sepolicy` injection mechanism
- [MagiskInit] Support Oculus Go
- [MagiskInit] Support Android 13 GKIs (Pixel 6)
- [MagiskBoot] Fix vbmeta extraction implementation
- [App] Fix stub app on older Android versions
- [App] [MagiskSU] Properly support apps using `sharedUserId`
- [MagiskSU] Fix a possible crash in `magiskd`
- [MagiskSU] Prune unused UIDs as soon as `system_server` restarts to prevent UID reuse attacks
- [MagiskSU] Verify and enforce the installed Magisk app's certificate to match the distributor's signature
- [MagiskSU] [Zygisk] Proper package management and detection
- [Zygisk] Fix function hooking on devices running Android 12 with old kernels
- [Zygisk] Fix Zygisk's self code unloading implementation
- [DenyList] Fix DenyList on shared UID apps
- [BusyBox] Add workaround for devices running old kernels

### v24.3 (2022.3.10)

- [General] Stop using `getrandom` syscall
- [Zygisk] Update API to v3, adding new fields to `AppSpecializeArgs`
- [App] Improve app repackaging installation workflow

### v24.2 (2022.3.1)

- [MagiskSU] Fix buffer overflow
- [MagiskSU] Fix owner managed multiuser superuser settings
- [MagiskSU] Fix command logging when using `su -c <cmd>`
- [MagiskSU] Prevent su request indefinite blocking
- [MagiskBoot] Support `lz4_legacy` archive with multiple magic
- [MagiskBoot] Fix `lz4_lg` compression
- [DenyList] Allow targeting processes running as system UID
- [Zygisk] Workaround Samsung's "early zygote"
- [Zygisk] Improved Zygisk loading mechanism
- [Zygisk] Fix application UID tracking
- [Zygisk] Fix improper `umask` being set in zygote
- [App] Fix BusyBox execution test
- [App] Improve stub loading mechanism
- [App] Major app upgrade flow improvements
- [General] Improve commandline error handling and messaging

### v24.1 (2022.1.28)

- [App] Stability improvements

### v24.0 (2022.1.26)

- [General] MagiskHide is removed from Magisk
- [General] Support Android 12
- [General] Support devices that do not support 32-bit and only runs 64-bit code
- [General] Update BusyBox to 1.34.1
- [Zygisk] Introduce new feature: Zygisk
- [Zygisk] Introduce DenyList feature to revert Magisk features in user selected processes
- [MagiskBoot] Support patching 32-bit kernel zImages
- [MagiskBoot] Support boot image header v4
- [MagiskBoot] Support patching out `skip_initramfs` from dtb bootargs
- [MagiskBoot] Add new env variable `PATCHVBMETAFLAG` to configure whether vbmeta flags should be patched
- [MagiskInit] Support loading fstab from `/system/etc` (required for Pixel 6)
- [MagiskInit] Support `/proc/bootconfig` for loading boot configurations
- [MagiskInit] Better support for some Meizu devices
- [MagiskInit] Better support for some OnePlus/Oppo/Realme devices
- [MagiskInit] Support `init.real` on some Sony devices
- [MagiskInit] Skip loading Magisk when detecting DSU
- [MagiskPolicy] Load `*_compat_cil_file` from system_ext
- [MagiskSU] Use isolated devpts if the kernel supports it
- [MagiskSU] Fix root shell if isolated mount namespace is set
- [resetprop] Deleted properties are now wiped from memory instead of just unlinking
- [App] Build a single APK for all ABIs
- [App] Switch to use standard bottom navigation bar
- [App] Downloading modules from the centralized Magisk-Modules-Repo is removed
- [App] Support user configuration of boot image vbmeta patching
- [App] Restore the ability to install Magisk on the other slot on some A/B devices
- [App] Allow modules to specify an update URL for in-app update + install

### v23.0 (2021.5.12)

- [App] Update snet extension. This fixes SafetyNet API errors.
- [App] Fix a bug in the stub app that causes APK installation to fail
- [App] Hide annoying errors in logs when hidden as stub
- [App] Fix issues when patching ODIN tar files when the app is hidden
- [General] Remove all pre Android 5.0 support
- [General] Update BusyBox to use proper libc
- [General] Fix C++ undefined behaviors
- [General] Several `sepolicy.rule` copy/installation fixes
- [MagiskPolicy] Remove unnecessary sepolicy rules
- [MagiskHide] Update package and process name validation logic
- [MagiskHide] Some changes that prevents zygote deadlock

### v22.1 (2021.4.9)

- [App] Prevent multiple installation sessions running in parallel
- [App] Prevent OutOfMemory crashes when checking boot signature on PXA boot images
- [General] Proper cgroup migration implementation
- [General] Rewrite log writer from scratch, should resolve any crashes and deadlocks
- [General] Many scripts updates fixing regressions
- [MagiskHide] Prevent possible deadlock when signal arrives
- [MagiskHide] Partial match process names if necessary
- [MagiskBoot] Preserve and patch AVB 2.0 structures/headers in boot images
- [MagiskBoot] Properly strip out data encryption flags
- [MagiskBoot] Prevent possible integer overflow
- [MagiskInit] Fix `sepolicy.rule` mounting strategy
- [resetprop] Always delete existing `ro.` props before updating. This will fix bootloops that could be caused by modifying device fingerprint properties.

### v22.0 (2021.2.23)

- [General] Magisk and Magisk Manager is now merged into the same package!
- [App] The term "Magisk Manager" is no longer used elsewhere. We refer it as the Magisk app.
- [App] Support hiding the Magisk app with advanced technique (stub APK loading) on Android 5.0+ (it used to be 9.0+)
- [App] Disallow re-packaging the Magisk app on devices lower than Android 5.0
- [App] Detect and warn about multiple invalid states and provide instructions on how to resolve it
- [MagiskHide] Fix a bug when stopping MagiskHide does not take effect
- [MagiskBoot] Fix bug when unpacking `lz4_lg` compressed boot images
- [MagiskInit] Support Galaxy S21 series
- [MagiskSU] Fix incorrect APEX paths that caused `libsqlite.so` fail to load

### v21.4 (2021.1.17)

- [MagiskSU] Fix `su -c` behavior that broke many root apps
- [General] Properly handle read/write over sockets (the `broken pipe` issue)

### v21.3 (2021.1.16)

- [MagiskInit] Avoid mounting `f2fs` userdata as it may result in kernel crashes. This shall fix a lot of bootloops
- [MagiskBoot] Fix a minor header checksum bug for `DHTB` header and ASUS `blob` image formats
- [MagiskHide] Allowing hiding isolated processes if the mount namespace is separated

### v21.2 (2020.12.28)

- [MagiskInit] Detect 2SI after mounting `system_root` on legacy SAR devices
- [General] Make sure `post-fs-data` scripts cannot block more than 35 seconds
- [General] Fix the `magisk --install-module` command
- [General] Trim Windows newline when reading files
- [General] Directly log to file to prevent `logcat` weirdness
- [MagiskBoot] Fix header dump/load for header v3 images

### v21.1 (2020.11.13)

- [MagiskBoot] Support boot header v3 (Pixel 5 and 4a 5G)
- [MagiskBoot] Distinguish `lz4_lg` and `lz4_legacy` (Pixel 5 and 4a 5G)
- [MagiskBoot] Support vendor boot images (for dev, not relevant for Magisk installation)
- [MagiskInit] Support kernel cmdline `androidboot.fstab_suffix`
- [MagiskInit] Support kernel initialized dm-verity on legacy SAR
- [General] Significantly broaden sepolicy.rule compatibility
- [General] Add Magisk binaries to `PATH` when executing boot scripts
- [General] Update `--remove-modules` command implementation
- [General] Make Magisk properly survive after factory reset on Android 11
- [MagiskSU] Add APEX package `com.android.i18n` to `LD_LIBRARY_PATH` when linking `libsqlite.so`
- [MagiskHide] Support hiding apps installed in secondary users (e.g. work profile)
- [MagiskHide] Make zygote detection more robust

### v21.0 (2020.10.3)

- [General] Support Android 11 🎉
- [General] Add Safe Mode detection. Disable all modules when the device is booting into Safe Mode.
- [General] Increase `post-fs-data` mode timeout from 10 seconds to 40 seconds
- [MagiskInit] Rewritten 2SI support from scratch
- [MagiskInit] Support when no `/sbin` folder exists (Android 11)
- [MagiskInit] Dump fstab from device-tree to rootfs and force `init` to use it for 2SI devices
- [MagiskInit] Strip out AVB for 2SI as it may cause bootloop
- [Modules] Rewritten module mounting logic from scratch
- [MagiskSU] For Android 8.0+, a completely new policy setup is used. This reduces compromises in Android's sandbox, providing more policy isolation and better security for root users.
- [MagiskSU] Isolated mount namespace will now first inherit from parent process, then isolate itself from the world
- [MagiskSU] Update communication protocol with Magisk Manager to work with the hardened SELinux setup
- [MagiskPolicy] Optimize match all rules. This will significantly reduce policy binary size and save memory and improve general kernel performance.
- [MagiskPolicy] Support declaring new types and attributes
- [MagiskPolicy] Make policy statement closer to stock `*.te` format. Please check updated documentation or `magiskpolicy --help` for more details.
- [MagiskBoot] Support compressed `extra` blobs
- [MagiskBoot] Pad boot images to original size with zeros
- [MagiskHide] Manipulate additional vendor properties

### v20.4 (2020.3.23)

- [MagiskInit] Fix potential bootloop in A-only 2SI devices
- [MagiskInit] Properly support Tegra partition naming
- [General] Load libsqlite.so dynamically, which removes the need to use wrapper scripts on Android 10+
- [General] Detect API level with a fallback method on some devices
- [General] Workaround possible bug in x86 kernel readlinkat system call
- [BusyBox] Enable SELinux features. Add chcon/runcon etc., and '-Z' option to many applets
- [BusyBox] Introduce standalone mode. More details in release notes
- [MagiskHide] Disable MagiskHide by default
- [MagiskHide] Add more potential detectable system properties
- [MagiskHide] Add workaround for Xiaomi devices bootloop when MagiskHide is enabled on cross region ROMs
- [MagiskBoot] Support patching special Motorolla DTB format
- [MagiskPolicy] Support 'genfscon' sepolicy rules
- [Scripts] Support NAND based boot images (character nodes in /dev/block)
- [Scripts] Better addon.d (both v1 and v2) support
- [Scripts] Support Lineage Recovery for Android 10+

### v20.3 (2020.1.10)

- [MagiskBoot] Fix `lz4_legacy` decompression

### v20.2 (2020.1.2)

- [MagiskSU] Properly handle communication between daemon and application (root request prompt)
- [MagiskInit] Fix logging in kmsg
- [MagiskBoot] Support patching dtb/dtbo partition formats
- [General] Support pre-init sepolicy patch in modules
- [Scripts] Update magisk stock image backup format

### v20.1 (2019.11.2)

- [MagiskSU] Support component name agnostic communication (for stub APK)
- [MagiskBoot] Set proper `header_size` in boot image headers (fix vbmeta error on Samsung devices)
- [MagiskHide] Scan zygote multiple times
- [MagiskInit] Support recovery images without /sbin/recovery binary. This will fix some A/B devices unable to boot to recovery after flashing Magisk
- [General] Move acct to prevent daemon being killed
- [General] Make sure "--remove-modules" will execute uninstall.sh after removal

### v20.0 (2019.10.11)

- [MagiskBoot] Support inject/modify `mnt_point` value in DTB fstab
- [MagiskBoot] Support patching QCDT
- [MagiskBoot] Support patching DTBH
- [MagiskBoot] Support patching PXA-DT
- [MagiskInit] [2SI] Support non A/B setup (Android 10)
- [MagiskHide] Fix bug that reject process names with ":"
- [MagicMount] Fix a bug that cause /product mirror not created

### v19.4 (2019.9.19)

- [MagiskInit] [SAR] Boot system-as-root devices with system mounted as /
- [MagiskInit] [2SI] Support 2-stage-init for A/B devices (Pixel 3 Android 10)
- [MagiskInit] [initramfs] Delay sbin overlay creation to post-fs-data
- [MagiskInit] [SARCompat] Old system-as-root implementation is deprecated, no more future changes
- [MagiskInit] Add overlay.d support for root directory overlay for new system-as-root implementation
- [MagiskSU] Unblock all signals in root shells (fix bash on Android)
- [MagicMount] Support replacing files in /product
- [MagiskHide] Support Android 10's Zygote blastula pool
- [MagiskHide] All random strings now also have random length
- [MagiskBoot] Allow no recompression for ramdisk.cpio
- [MagiskBoot] Support some weird Huawei boot images
- [General] Add new `--remove-modules` command to remove modules without root in ADB shell
- [General] Support Android 10 new APEX libraries (Project Mainline)

### v19.3 (2019.6.5)

- [MagiskHide] Hugely improve process monitor implementation, hopefully should no longer cause 100% CPU and daemon crashes
- [MagiskInit] Wait for partitions to be ready for early mount, should fix bootloops on a handful of devices
- [MagiskInit] Support EROFS used in EMUI 9.1
- [MagiskSU] Properly implement mount namespace isolation
- [MagiskBoot] Proper checksum calculation for header v2

### v19.2 (2019.5.20)

- [General] Fix uninstaller
- [General] Fix bootloops on some devices with tmpfs mounting to /data
- [MagiskInit] Add Kirin hi6250 support
- [MagiskSU] Stop claiming device focus for su logging/notify if feasible.
  This fix issues with users locking Magisk Manager with app lock, and prevent
  video apps get messed up when an app is requesting root in the background.

### v19.1 (2019.5.1)

- [General] Support recovery based Magisk
- [General] Support Android Q Beta 2
- [MagiskInit] New sbin overlay setup process for better compatibility
- [MagiskInit] Allow long pressing volume up to boot to recovery in recovery mode
- [MagicMount] Use proper system_root mirror
- [MagicMount] Use self created device nodes for mirrors
- [MagicMount] Do not allow adding new files/folders in partition root folder (e.g. /system or /vendor)

### v19.0 (2019.3.28)

- [General] Remove usage of magisk.img
- [General] Add 64 bit magisk binary for native 64 bit support
- [General] Support A only system-as-root devices that released with Android 9.0
- [General] Support non EXT4 system and vendor partitions
- [MagiskHide] Use Zygote ptracing for monitoring new processes
- [MagiskHide] Targets are now per-application component
- [MagiskInit] Support Android Q (no logical partition support yet!)
- [MagiskPolicy] Support Android Q new split sepolicy setup
- [MagiskInit] Move sbin overlay creation from main daemon post-fs-data to early-init
- [General] Service scripts now run in parallel
- [MagiskInit] Directly inject magisk services to init.rc
- [General] Use lzma2 compressed ramdisk in extreme conditions
- [MagicMount] Clone attributes from original file if exists
- [MagiskSU] Use `ACTION_REBOOT` intent to workaround some OEM broadcast restrictions
- [General] Use `skip_mount` instead of `auto_mount`: from opt-in to opt-out

### v18.1 (2019.2.4)

- [General] Support EMUI 9.0
- [General] Support Kirin 960 devices
- [General] Support down to Android 4.2
- [General] Major code base modernization under-the-hood

### v18.0 (2018.12.8)

- [General] Migrate all code base to C++
- [General] Modify database natively instead of going through Magisk Manager
- [General] Deprecate path /sbin/.core, please start using /sbin/.magisk
- [General] Boot scripts are moved from `<magisk_img>/.core/<stage>.d` to `/data/adb/<stage>.d`
- [General] Remove native systemless hosts (Magisk Manager is updated with a built-in systemless hosts module)
- [General] Allow module post-fs-data.sh scripts to disable/remove modules
- [MagiskHide] Use component names instead of process names as targets
- [MagiskHide] Add procfs protection on SDK 24+ (Nougat)
- [MagiskHide] Remove the folder /.backup to prevent detection
- [MagiskHide] Hide list is now stored in database instead of raw textfile in images
- [MagiskHide] Add "--status" option to CLI
- [MagiskHide] Stop unmounting non-custom related mount points
- [MagiskSU] Add `FLAG_INCLUDE_STOPPED_PACKAGES` in broadcasts to force wake Magisk Manager
- [MagiskSU] Fix a bug causing SIGWINCH not properly detected
- [MagiskPolicy] Support new av rules: type_change, type_member
- [MagiskPolicy] Remove all AUDITDENY rules after patching sepolicy to log all denies for debugging
- [MagiskBoot] Properly support extra_cmdline in boot headers
- [MagiskBoot] Try to repair broken v1 boot image headers
- [MagiskBoot] Add new CPIO command: "exists"

### v17.3 (2018.10.20)

- [MagiskBoot] Support boot image header v1 (Pixel 3)
- [MagiskSU] No more linked lists for caching `su_info`
- [MagiskSU] Parse command-lines in client side and send only options to daemon
- [MagiskSU] Early ACK to prevent client freezes and early denies
- [Daemon] Prevent bootloops in situations where /data is mounted twice
- [Daemon] Prevent logcat failures when /system/bin is magic mounting, could cause MagiskHide to fail
- [Scripts] Switch hexpatch to remove Samsung Defex to a more general pattern
- [Scripts] Update data encryption detection for better custom recovery support

### v17.2 (2018.9.21)

- [ResetProp] Update to AOSP upstream to support serialized system properties
- [MagiskInit] Randomize Magisk service names to prevent detection (e.g. FGO)
- [MagiskSU] New communication scheme to communicate with Magisk Manager

### v17.0/17.1 (2018.9.1)

- [General] Bring back install to inactive slot for OTAs on A/B devices
- [Script] Remove system based root in addon.d
- [Script] Add proper addon.d-v2 for preserving Magisk on custom ROMs on A/B devices
- [Script] Enable KEEPVERITY when the device is using system_root_image
- [Script] Add hexpatch to remove Samsung defex in new Oreo kernels
- [Daemon] Support non ext4 filesystems for mirrors (system/vendor)
- [MagiskSU] Make pts sockets always run in dev_pts secontext, providing all terminal emulator root shell the same power as adb shells
- [MagiskHide] Kill all processes with same UID of the target to workaround OOS embryo optimization
- [MagiskInit] Move all sepolicy patches pre-init to prevent Pixel 2 (XL) boot service breakdown

### v16.7 (2018.7.19)

- [Scripts] Fix boot image patching errors on Android P (workaround the strengthened seccomp)
- [MagiskHide] Support hardlink based ns proc mnt (old kernel support)
- [Daemon] Fix permission of /dev/null after logcat commands, fix ADB on EMUI
- [Daemon] Log fatal errors only on debug builds
- [MagiskInit] Detect early mount partname from fstab in device tree

### v16.6 (2018.7.8)

- [General] Add wrapper script to overcome weird `LD_XXX` flags set in apps
- [General] Prevent bootloop when flashing Magisk after full wipe on FBE devices
- [Scripts] Support patching DTB placed in extra sections in boot images (Samsung S9/S9+)
- [Scripts] Add support for addon.d-v2 (untested)
- [Scripts] Fix custom recovery console output in addon.d
- [Scripts] Fallback to parsing sysfs for detecting block devices
- [Daemon] Check whether a valid Magisk Manager is installed on boot, if not, install stub APK embedded in magiskinit
- [Daemon] Check whether Magisk Manager is repackaged (hidden), and prevent malware from hijacking com.topjohnwu.magisk
- [Daemon] Introduce new daemon: magisklogd, a dedicated daemon to handle all logcat related monitoring
- [Daemon] Replace old invincible mode with handshake between magiskd and magisklogd, one will respawn the other if disconnected
- [Daemon] Support GSI adbd bind mounting
- [MagiskInit] Support detecting block names in upper case (Samsung)
- [MagiskBoot] Check DTB headers to prevent false detections within kernel binary
- [MagiskHide] Compare mount namespace with PPID to make sure the namespace is actually separated, fix root loss
- [MagiskSU] Simplify `su_info` caching system, should use less resources and computing power
- [MagiskSU] Reduce the amount of broadcasting to Magisk Manager
- [ImgTool] Separate all ext4 image related operations to a new applet called "imgtool"
- [ImgTool] Use precise free space calculation methods
- [ImgTool] Use our own set of loop devices hidden along side with sbin tmpfs overlay. This not only eliminates another possible detection method, but also fixes apps that mount OBB files as loop devices (huge thanks to dev of Pzizz for reporting this issue)

### v16.4 (2018.4.29)

- [Daemon] Directly check logcat command instead of detecting logd, should fix logging and MagiskHide on several Samsung devices
- [Daemon] Fix startup Magisk Manager APK installation on Android P
- [MagiskPolicy] Switch from AOSP u:r:su:s0 to u:r:magisk:s0 to prevent conflicts
- [MagiskPolicy] Remove unnecessary sepolicy rules to reduce security penalty
- [Daemon] Massive re-design /sbin tmpfs overlay and daemon start up
- [MagiskInit] Remove `magiskinit_daemon`, the actual magisk daemon (magiskd) shall handle everything itself
- [Daemon] Remove post-fs stage as it is very limited and also will not work on A/B devices; replaced with simple mount in post-fs-data, which will run ASAP even before the daemon is started
- [General] Remove all 64-bit binaries as there is no point in using them; all binaries are now 32-bit only.
  Some weirdly implemented root apps might break (e.g. Tasker, already reported to the developer), but it is not my fault :)
- [resetprop] Add Protobuf encode/decode to support manipulating persist properties on Android P
- [MagiskHide] Include app sub-services as hiding targets. This might significantly increase the amount of apps that could be properly hidden

### v16.3 (2018.3.28)

- [General] Remove symlinks used for backwards compatibility
- [MagiskBoot] Fix a small size calculation bug

### v16.2 (2018.3.18)

- [General] Force use system binaries in handling ext4 images (fix module installation on Android P)
- [MagiskHide] Change property state to disable if logd is disabled

### v16.1 (2018.3.11)

- [MagiskBoot] Fix MTK boot image packaging
- [MagiskBoot] Add more Nook/Acclaim headers support
- [MagiskBoot] Support unpacking DTB with empty kernel image
- [MagiskBoot] Update high compression mode detection logic
- [Daemon] Support new mke2fs tool on Android P
- [resetprop] Support Android P new property context files
- [MagiskPolicy] Add new rules for Android P

### v16.0 (2018.2.22)

- [MagiskInit] Support non `skip_initramfs` devices with slot suffix (Huawei Treble)
- [MagiskPolicy] Add rules for Magisk Manager
- [Compiler] Workaround an NDK compiler bug that causes bootloops

### v15.4 (2018.2.13)

- [MagiskBoot] Support Samsung PXA, DHTB header images
- [MagiskBoot] Support ASUS blob images
- [MagiskBoot] Support Nook Green Loader images
- [MagiskBoot] Support pure ramdisk images
- [MagiskInit] Prevent OnePlus angela `sepolicy_debug` from loading
- [MagiskInit] Obfuscate Magisk socket entry to prevent detection and security
- [Daemon] Fix subfolders in /sbin shadowed by overlay
- [Daemon] Obfuscate binary names to prevent naive detections
- [Daemon] Check logd before force trying to start logcat in a loop

### v15.3 (2018.1.12)

- [Daemon] Fix the bug that only one script would be executed in post-fs-data.d/service.d
- [Daemon] Add `MS_SILENT` flag when mounting, should fix some devices that cannot mount magisk.img
- [MagiskBoot] Fix potential segmentation fault when patching ramdisk, should fix some installation failures

### v15.2 (2018.1.1)

- [MagiskBoot] Fix dtb verity patches, should fix dm-verity bootloops on newer devices placing fstabs in dtb
- [MagiskPolicy] Add new rules for proper Samsung support, should fix MagiskHide
- [MagiskInit] Support non `skip_initramfs` devices using split sepolicies (e.g. Zenfone 4 Oreo)
- [Daemon] Use specific logcat buffers, some devices does not support all log buffers
- [scripts] Update scripts to double check whether boot slot is available, some devices set a boot slot without A/B partitions

### v15.1 (2017.12.29)

- [MagiskBoot] Fix faulty code in ramdisk patches which causes bootloops in some config and fstab format combos

### v15.0 (2017.12.26)

- [Daemon] Fix the bug that Magisk cannot properly detect /data encryption state
- [Daemon] Add merging `/cache/magisk.img` and `/data/adb/magisk_merge.img` support
- [Daemon] Update to upstream libsepol to support cutting edge split policy custom ROM cil compilations

### v14.6 (2017.12.22)

- [General] Move all files into a safe location: /data/adb
- [Daemon] New invincible implementation: use `magiskinit_daemon` to monitor sockets
- [Daemon] Rewrite logcat monitor to be more efficient
- [Daemon] Fix a bug where logcat monitor may spawn infinite logcat processes
- [MagiskSU] Update su to work the same as proper Linux implementation:
  Initialize window size; all environment variables will be migrated (except HOME, SHELL, USER, LOGNAME, these will be set accordingly),
  "--preserve-environment" option will preserve all variables, including those four exceptions.
  Check the Linux su manpage for more info
- [MagiskBoot] Massive refactor, rewrite all cpio operations and CLI
- [MagiskInit][magiskboot] Support ramdisk high compression mode

### v14.5 (1456) (2017.11.23)

- [Magiskinit] Fix bootloop issues on several devices
- [misc] Build binaries with NDK r10e, should get rid of the nasty linker warning when executing magisk

### v14.5 (1455) (2017.11.23)

- [Daemon] Moved internal path to /sbin/.core, new image mountpoint is /sbin/.core/img
- [MagiskSU] Support switching package name, used when Magisk Manager is hidden
- [MagiskHide] Add temporary /magisk removal
- [MagiskHide] All changes above contributes to hiding from nasty apps like FGO and several banking apps
- [Magiskinit] Use magiskinit for all devices (dynamic initramfs)
- [Magiskinit] Fix Xiaomi A1 support
- [Magiskinit] Add Pixel 2 (XL) support
- [Magiskboot] Add support to remove avb-verity in dtbo.img
- [Magiskboot] Fix typo in handling MTK boot image headers
- [script] Along with updates in Magisk Manager, add support to sign boot images (AVB 1.0)
- [script] Add dtbo.img backup and restore support
- [misc] Many small adjustments to properly support old platforms like Android 5.0

### v14.3 (2017.10.15)

- [MagiskBoot] Fix Pixel C installation
- [MagiskBoot] Handle special `lz4_legacy` format properly, should fix all LG devices
- [Daemon] New universal logcat monitor is added, support plug-and-play to worker threads
- [Daemon] Invincible mode: daemon will be restarted by init, everything should seamlessly through daemon restarts
- [Daemon] Add new restorecon action, will go through and fix all Magisk files with selinux unlabeled to `system_file` context
- [Daemon] Add brute-force image resizing mode, should prevent the notorious Samsung crappy resize2fs from affecting the result
- [resetprop] Add new "-p" flag, used to toggle whether alter/access the actual persist storage for persist props

### v14.2 (2017.9.28)

- [MagicMount] Clone attributes to tmpfs mountpoint, should fix massive module breakage

### v14.1 (2017.9.28)

- [MagiskInit] Introduce a new init binary to support `skip_initramfs` devices (Pixel family)
- [script] Fix typo in update-binary for x86 devices
- [script] Fix stock boot image backup not moved to proper location
- [script] Add functions to support A/B slot and `skip_initramfs` devices
- [script] Detect Meizu boot blocks
- [MagiskBoot] Add decompress zImage support
- [MagiskBoot] Support extracting dtb appended to zImage block
- [MagiskBoot] Support patching fstab within dtb
- [Daemon/MagiskSU] Proper file based encryption support
- [Daemon] Create core folders if not exist
- [resetprop] Fix a bug which delete props won't remove persist props not in memory
- [MagicMount] Remove usage of dummy folder, directly mount tmpfs and construct file structure skeleton in place

### v14.0 (2017.9.6)

- [script] Simplify installation scripts
- [script] Fix a bug causing backing up and restoring stock boot images failure
- [script] Installation and uninstallation will migrate old or broken stock boot image backups to proper format
- [script] Fix an issue with selabel setting in `util_functions.sh` on Lollipop
- [rc script] Enable logd in post-fs to start logging as early as possible
- [MagiskHide] magisk.img mounted is no longer a requirement
  Devices with issues mounting magisk.img can now run in proper core-only mode
- [MagiskBoot] Add native function to extract stock SHA1 from ramdisk
- [b64xz] New tool to extract compressed and encoded binary dumps in shell script
- [busybox] Add busybox to Magisk source, and embed multi-arch busybox binary into update-binary shell script
- [busybox] Busybox is added into PATH for all boot scripts (post-fs-data.d, service.d, and all module scripts)
- [MagiskSU] Fully fix multiuser issues
- [Magic Mount] Fix a typo in cloning attributes
- [Daemon] Fix the daemon crashing when boot scripts opens a subshell
- [Daemon] Adjustments to prevent stock Samsung kernel restrictions on exec system calls for binaries started from /data
- [Daemon] Workaround on Samsung device with weird fork behaviors

### v13.3 (2017.7.18)

- [MagiskHide] Update to bypass Google CTS (2017.7.17)
- [resetprop] Properly support removing persist props
- [uninstaller] Remove Magisk Manager and persist props

### v13.2 (2017.7.14)

- [magiskpolicy] Fix magiskpolicy segfault on old Android versions, should fix tons of older devices that couldn't use v13.1
- [MagiskHide] Set proper selinux context while re-linking /sbin to hide Magisk, should potentially fix many issues
- [MagiskBoot] Change lzma compression encoder flag from `LZMA_CHECK_CRC64` to `LZMA_CHECK_CRC32`, kernel only supports latter
- [General] Core-only mode now properly mounts systemless hosts and magiskhide

### v13.1 (2017.7.11)

- [General] Merge MagiskSU, magiskhide, resetprop, magiskpolicy into one binary
- [General] Add Android O support (tested on DP3)
- [General] Dynamic link libselinux.so, libsqlite.so from system to greatly reduce binary size
- [General] Remove bundled busybox because it causes a lot of issues
- [General] Unlock all block devices for read-write support instead of emmc only (just figured not all devices uses emmc lol)
- [Scripts] Run all ext4 image operations through magisk binary in flash scripts
- [Scripts] Updated scripts to use magisk native commands to increase compatibility
- [Scripts] Add addon.d survival support
- [Scripts] Introduce `util_functions.sh`, used as a global shell script function source for all kinds of installation
- [MagiskBoot] Moved boot patch logic into magiskboot binary
- [MagiskSU] Does not fork new process for each request, add new threads instead
- [MagiskSU] Added multiuser support
- [MagiskSU] Introduce new timeout queue mechanism, prevent performance hit with poorly written su apps
- [MagiskSU] Multiple settings moved from prop detection to database
- [MagiskSU] Add namespace mode option support
- [MagiskSU] Add master-mount option
- [resetprop] Updated to latest AOSP upstream, support props from 5.0 to Android O
- [resetprop] Renamed all functions to prevent calling functions from external libc
- [magiskpolicy] Updated libsepol from official SELinux repo
- [magiskpolicy] Added xperm patching support (in order to make Android O work properly)
- [magiskpolicy] Updated rules for Android O, and Liveboot support
- [MagiskHide] Remove pseudo permissive mode, directly hide permissive status instead
- [MagiskHide] Remove unreliable list file monitor, change to daemon request mode
- [MagiskHide] MagiskHide is now enabled by default
- [MagiskHide] Update unmount policies, passes CTS in SafetyNet!
- [MagiskHide] Add more props for hiding
- [MagiskHide] Remove background magiskhide daemon, spawn short life process for unmounting purpose
- [Magic Mount] Ditched shell script based mounting, use proper C program to parse and mount files. Speed is SIGNIFICANTLY improved

### v12.0 (2017.3.31)

- [General] Move most binaries into magisk.img (Samsung cannot run su daemon in /data)
- [General] Move sepolicy live patch to `late_start` service
  This shall fix the long boot times, especially on Samsung devices
- [General] Add Samsung RKP hexpatch back, should now work on Samsung stock kernels
- [General] Fix installation with SuperSU
- [MagiskHide] Support other logcat `am_proc_start` patterns
- [MagiskHide] Change /sys/fs/selinux/enforce(policy) permissions if required
  Samsung devices cannot switch selinux states, if running on permissive custom kernel, the users will stuck at permissive
  If this scenario is detected, change permissions to hide the permissive state, leads to SafetyNet passes
- [MagiskHide] Add built in prop rules to fake KNOX status
  Samsung apps requiring KNOX status to be 0x0 should now work (Samsung Pay not tested)
- [MagiskHide] Remove all ro.build props, since they cause more issues than they benefit...
- [MagiskBoot] Add lz4 legacy format support (most linux kernel using lz4 for compression is using this)
- [MagiskBoot] Fix MTK kernels with MTK headers

### v11.5/11.6 (2017.3.21)

- [Magic Mount] Fix mounting issues with devices that have separate /vendor partitions
- [MagiskBoot] Whole new boot image patching tool, please check release note for more info
- [magiskpolicy] Rename sepolicy-inject to magiskpolicy
- [magiskpolicy] Update a rule to allow chcon everything properly
- [MagiskHide] Prevent multirom crashes
- [MagiskHide] Add patches for ro.debuggable, ro.secure, ro.build.type, ro.build.tags, ro.build.selinux
- [MagiskHide] Change /sys/fs/selinux/enforce, /sys/fs/selinux/policy permissions for Samsung compatibility
- [MagiskSU] Fix read-only partition mounting issues
- [MagiskSU] Disable -cn option, the option will do nothing, preserved for compatibility

### v11.1 (2017.2.6)

- [sepolicy-inject] Add missing messages
- [magiskhide] Start MagiskHide with scripts

### v11.0 (2017.2.6)

- [Magic Mount] Support replacing symlinks.
  Symlinks cannot be a target of a bind mounted, so they are treated the same as new files
- [Magic Mount] Fix the issue when file/folder name contains spaces
- [BusyBox] Updated to v1.26.2. Should fix the black screen issues of FlashFire
- [resetprop] Support reading prop files that contains spaces in prop values
- [MagiskSU] Adapt communication to Magisk Manager; stripped out unused data transfer
- [MagiskSU] Implement SuperUser access option (Disable, APP only, ADB Only, APP & ADB)
  phh Superuser app has this option but the feature isn't implemented within the su binary
- [MagiskSU] Fixed all issues with su -c "commands" (run commands with root)
  This feature is supposed to only allow one single option, but apparently adb shell su -c "command" doesn't work this way, and plenty of root apps don't follow the rule. The su binary will now consider everything after -c as a part of the command.
- [MagiskSU] Removed legacy context hack for TiBack, what it currently does is slowing down the invocation
- [MagiskSU] Preserve the current working directory after invoking su
  Previously phh superuser will change the path to /data/data after obtaining root shell. It will now stay in the same directory where you called su
- [MagiskSU] Daemon now also runs in u:r:su:s0 context
- [MagiskSU] Removed an unnecessary fork, reduce running processes and speed up the invocation
- [MagiskSU] Add -cn option to the binary
  Not sure if this is still relevant, and also not sure if implemented correctly, but hey it's here
- [sepolicy-inject] Complete re-write the command-line options, now nearly matches supolicy syntax
- [sepolicy-inject] Support all matching mode for nearly every action (makes pseudo enforced possible)
- [sepolicy-inject] Fixed an ancient bug that allocated memory isn't reset
- [uninstaller] Now works as a independent script that can be executed at boot
  Fully support recovery with no /data access, Magisk uninstallation with Magisk Manager
- [Addition] Busybox, MagiskHide, hosts settings can now be applied instantly; no reboots required
- [Addition] Add post-fs-data.d and service.d
- [Addition] Add option to disable Magisk (MagiskSU will still be started)

### v10.2 (2017.1.2)

- [Magic Mount] Remove apps/priv-app from whitelist, should fix all crashes
- [phh] Fix binary out-of-date issue
- [scripts] Fix root disappear issue when upgrading within Magisk Manager

### v10 (2017.1.2)

- [Magic Mount] Use a new way to mount system (vendor) mirrors
- [Magic Mount] Use universal way to deal with /vendor, handle both separate partition or not
- [Magic Mount] Adding **anything to any place** is now officially supported (including /system root and /vendor root)
- [Magic Mount] Use symlinks for mirroring back if possible, reduce bind mounts for adding files
- [Magisk Hide] Check init namespace, zygote namespace to prevent Magic Mount breakage (a.k.a root loss)
- [Magisk Hide] Send SIGSTOP to pause target process ASAP to prevent crashing if unmounting too late
- [Magisk Hide] Hiding should work under any conditions, including adding libs and /system root etc.
- [phh] Root the device if no proper root detected
- [phh] Move `/sbin` to `/sbin_orig` and link back, fix Samsung no-suid issue
- [scripts] Improve SuperSU integration, now uses sukernel to patch ramdisk, support SuperSU built in ramdisk restore
- [template] Add PROPFILE option to load system.prop

### v9 (2016.11.14)

- **[API Change] Remove the interface for post-fs modules**
- [resetprop] New tool "resetprop" is added to Magisk to replace most post-fs modules' functionality
- [resetprop] Magisk will now patch "ro.boot.verifiedbootstate", "ro.boot.flash.locked", "ro.boot.veritymode" to bypass Safety Net
- [Magic Mount] Move dummy skeleton / mirror / mountinfo filesystem tree to tmpfs
- [Magic Mount] Rewritten dummy cloning mechanism from scratch, will result in minimal bind mounts, minimal file traversal, eliminate all possible issues that might happen in extreme cases
- [Magic Mount] Adding new items to /system/bin, /system/vendor, /system/lib(64) is properly supported (devices with separate vendor partition is not supported yet)
- [Magisk Hide] Rewritten from scratch, now run in daemon mode, proper list monitoring, proper mount detection, and maybe more.....
- [Boot Image] Add support for Motorola boot image dtb, it shall now unpack correctly
- [Uninstaller] Add removal of SuperSU custom patch script

### v8 (2016.10.19)

- Add Magisk Hide to bypass SafetyNet
- Improve SuperSU integration: no longer changes the SuperSU PATH
- Support rc script entry points not located in init.rc

### v7 (2016.10.04)

- Fully open source
- Remove supolicy dependency, use my own sepolicy-injection
- Run everything in its own selinux domain, should fix all selinux issues
- Add Note 7 stock kernel hex patches
- Add support to install Magisk in Magisk Manager
- Add support for image merging for module flashing in Magisk Manager
- Add root helpers for SuperSU auto module-ize and auto upgrading legacy phh superuser
- New paths to toggle busybox, and support all root solutions
- Remove root management API; both SuperSU and phh has their own superior solutions

### [v6 (2016.8.21)](https://xdaforums.com/t/magisk-general-support-discussion.3432382/post-68298121)

- Fixed the algorithm for adding new files and dummy system
- Updated the module template with a default permission, since people tend to forget them :)

### [v5 (2016.8.20)](https://xdaforums.com/t/magisk-general-support-discussion.3432382/post-68274534)

- Hotfix for older Android versions (detect policy before patching)
- Update uninstaller to NOT uninstall Magisk Manager, since it cause problems

### [v4 (2016.8.19)](https://xdaforums.com/t/magisk-general-support-discussion.3432382/post-68269300)

- Important: Uninstall v1 - v3 Magisk before upgrading with the uninstaller in the OP!!
- Massive Rewrite Magisk Interface API! All previous mods are NOT compatible! Please download the latest version of the mods you use (root/xposed)
- Mods are now installed independently in their own subfolder. This paves the way for future Magisk Manager versions to manage mods, **just like how Xposed Modules are handled**
- Support small boot partition devices (Huawei devices)
- Use minimal sepolicy patch in boot image for smaller ramdisk size. Live patch policies after bootup
- Include updated open source sepolicy injection tool (source code available), support nearly all SuperSU supolicy tool's functionality

### [v3 (2016.8.11)](https://xdaforums.com/t/magisk-general-support-discussion.3432382/post-68146978)

- Fix bootimg-extract for Exynos Samsung devices (thanks to @phhusson), should fix all Samsung device issues
- Add supolicy back to patch sepolicy (stock Samsung do not accept permissive domain)
- Update sepolicy-injection to patch su domain for Samsung devices to use phh's root
- Update root disable method, using more aggressive approach
- Use lazy unmount to unmount root from system, should fix some issues with custom roms
- Use the highest possible compression rate for ramdisk, hope to fix some devices with no boot partition space
- Detect boot partition space insufficient, will abort installer instead of breaking your device

### [v2 (2016.8.9)](https://xdaforums.com/t/magisk-general-support-discussion.3432382/post-68108058)

- Fix verity patch. It should now work on all devices (might fix some of the unable-to-boot issues)
- All scripts will now run in selinux permissive mode for maximum compatibility (this will **NOT** turn your device to permissive)
- Add Nougat Developer Preview 5 support
- Add systemless host support for AdBlock Apps (enabled by default)
- Add support for new root disable method
- Remove sepolicy patches that uses SuperSU's supolicy tool; it is now using a minimal set of modifications
- Removed Magisk Manager in Magisk patch, it is now included in Magisk phh's superuser only

### [v1 (2016.8.3)](https://xdaforums.com/t/magisk-general-support-discussion.3432382/post-68034103)

- Initial release
