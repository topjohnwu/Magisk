# Magisk Changelog

### v21.0

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

### v20.4

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

### v20.3
- [MagiskBoot] Fix `lz4_legacy` decompression

### v20.2

- [MagiskSU] Properly handle communication between daemon and application (root request prompt)
- [MagiskInit] Fix logging in kmsg
- [MagiskBoot] Support patching dtb/dtbo partition formats
- [General] Support pre-init sepolicy patch in modules
- [Scripts] Update magisk stock image backup format

### v20.1

- [MagiskSU] Support component name agnostic communication (for stub APK)
- [MagiskBoot] Set proper `header_size` in boot image headers (fix vbmeta error on Samsung devices)
- [MagiskHide] Scan zygote multiple times
- [MagiskInit] Support recovery images without /sbin/recovery binary. This will fix some A/B devices unable to boot to recovery after flashing Magisk
- [General] Move acct to prevent daemon being killed
- [General] Make sure "--remove-modules" will execute uninstall.sh after removal

### v20.0

- [MagiskBoot] Support inject/modify `mnt_point` value in DTB fstab
- [MagiskBoot] Support patching QCDT
- [MagiskBoot] Support patching DTBH
- [MagiskBoot] Support patching PXA-DT
- [MagiskInit] [2SI] Support non A/B setup (Android 10)
- [MagiskHide] Fix bug that reject process names with ":"
- [MagicMount] Fix a bug that cause /product mirror not created

### v19.4

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

### v19.3

- [MagiskHide] Hugely improve process monitor implementation, hopefully should no longer cause 100% CPU and daemon crashes
- [MagiskInit] Wait for partitions to be ready for early mount, should fix bootloops on a handful of devices
- [MagiskInit] Support EROFS used in EMUI 9.1
- [MagiskSU] Properly implement mount namespace isolation
- [MagiskBoot] Proper checksum calculation for header v2

### v19.2

- [General] Fix uninstaller
- [General] Fix bootloops on some devices with tmpfs mounting to /data
- [MagiskInit] Add Kirin hi6250 support
- [MagiskSU] Stop claiming device focus for su logging/notify if feasible.
This fix issues with users locking Magisk Manager with app lock, and prevent
video apps get messed up when an app is requesting root in the background.

### v19.1

- [General] Support recovery based Magisk
- [General] Support Android Q Beta 2
- [MagiskInit] New sbin overlay setup process for better compatibility
- [MagiskInit] Allow long pressing volume up to boot to recovery in recovery mode
- [MagicMount] Use proper system\_root mirror
- [MagicMount] Use self created device nodes for mirrors
- [MagicMount] Do not allow adding new files/folders in partition root folder (e.g. /system or /vendor)

### v19.0

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

### v18.1

- [General] Support EMUI 9.0
- [General] Support Kirin 960 devices
- [General] Support down to Android 4.2
- [General] Major code base modernization under-the-hood

### v18.0

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

### v17.3

- [MagiskBoot] Support boot image header v1 (Pixel 3)
- [MagiskSU] No more linked lists for caching `su_info`
- [MagiskSU] Parse command-lines in client side and send only options to daemon
- [MagiskSU] Early ACK to prevent client freezes and early denies
- [Daemon] Prevent bootloops in situations where /data is mounted twice
- [Daemon] Prevent logcat failures when /system/bin is magic mounting, could cause MagiskHide to fail
- [Scripts] Switch hexpatch to remove Samsung Defex to a more general pattern
- [Scripts] Update data encryption detection for better custom recovery support

### v17.2

- [ResetProp] Update to AOSP upstream to support serialized system properties
- [MagiskInit] Randomize Magisk service names to prevent detection (e.g. FGO)
- [MagiskSU] New communication scheme to communicate with Magisk Manager

### v17.0/17.1

- [General] Bring back install to inactive slot for OTAs on A/B devices
- [Script] Remove system based root in addon.d
- [Script] Add proper addon.d-v2 for preserving Magisk on custom ROMs on A/B devices
- [Script] Enable KEEPVERITY when the device is using system\_root\_image
- [Script] Add hexpatch to remove Samsung defex in new Oreo kernels
- [Daemon] Support non ext4 filesystems for mirrors (system/vendor)
- [MagiskSU] Make pts sockets always run in dev_pts secontext, providing all terminal emulator root shell the same power as adb shells
- [MagiskHide] Kill all processes with same UID of the target to workaround OOS embryo optimization
- [MagiskInit] Move all sepolicy patches pre-init to prevent Pixel 2 (XL) boot service breakdown

### v16.7

- [Scripts] Fix boot image patching errors on Android P (workaround the strengthened seccomp)
- [MagiskHide] Support hardlink based ns proc mnt (old kernel support)
- [Daemon] Fix permission of /dev/null after logcat commands, fix ADB on EMUI
- [Daemon] Log fatal errors only on debug builds
- [MagiskInit] Detect early mount partname from fstab in device tree

### v16.6

- [General] Add wrapper script to overcome weird `LD_XXX` flags set in apps
- [General] Prevent bootloop when flashing Magisk after full wipe on FBE devices
- [Scripts] Support patching DTB placed in extra sections in boot images (Samsung S9/S9+)
- [Scripts] Add support for addon.d-v2 (untested)
- [Scripts] Fix custom recovery console output in addon.d
- [Scripts] Fallback to parsing sysfs for detecting block devices
- [Daemon] Check whether a valid Magisk Manager is installed on boot, if not, install stub APK embedded in magiskinit
- [Daemon] Check whether Magisk Manager is repackaged (hidden), and prevent malware from hijacking com.topjohnwu.magisk
- [Daemon] Introduce new daemon: magisklogd, a dedicated daemon to handle all logcat related monitoring
- [Daemon] Replace old invincible mode with handshake between magiskd and magisklogd, one will respwan the other if disconnected
- [Daemon] Support GSI adbd bind mounting
- [MagiskInit] Support detecting block names in upper case (Samsung)
- [MagiskBoot] Check DTB headers to prevent false detections within kernel binary
- [MagiskHide] Compare mount namespace with PPID to make sure the namespace is actually separated, fix root loss
- [MagiskSU] Simplify `su_info` caching system, should use less resources and computing power
- [MagiskSU] Reduce the amount of broadcasting to Magisk Manager
- [ImgTool] Separate all ext4 image related operations to a new applet called "imgtool"
- [ImgTool] Use precise free space calculation methods
- [ImgTool] Use our own set of loop devices hidden along side with sbin tmpfs overlay. This not only eliminates another possible detection method, but also fixes apps that mount OBB files as loop devices (huge thanks to dev of Pzizz for reporting this issue)

### v16.4

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

### v16.3

- [General] Remove symlinks used for backwards compatibility
- [MagiskBoot] Fix a small size calculation bug

### v16.2

- [General] Force use system binaries in handling ext4 images (fix module installation on Android P)
- [MagiskHide] Change property state to disable if logd is disabled

### v16.1

- [MagiskBoot] Fix MTK boot image packaging
- [MagiskBoot] Add more Nook/Acclaim headers support
- [MagiskBoot] Support unpacking DTB with empty kernel image
- [MagiskBoot] Update high compression mode detection logic
- [Daemon] Support new mke2fs tool on Android P
- [resetprop] Support Android P new property context files
- [MagiskPolicy] Add new rules for Android P

### v16.0

- [MagiskInit] Support non `skip_initramfs` devices with slot suffix (Huawei Treble)
- [MagiskPolicy] Add rules for Magisk Manager
- [Compiler] Workaround an NDK compiler bug that causes bootloops

### v15.4

- [MagiskBoot] Support Samsung PXA, DHTB header images
- [MagiskBoot] Support ASUS blob images
- [MagiskBoot] Support Nook Green Loader images
- [MagiskBoot] Support pure ramdisk images
- [MagiskInit] Prevent OnePlus angela `sepolicy_debug` from loading
- [MagiskInit] Obfuscate Magisk socket entry to prevent detection and security
- [Daemon] Fix subfolders in /sbin shadowed by overlay
- [Daemon] Obfuscate binary names to prevent naive detections
- [Daemon] Check logd before force trying to start logcat in a loop

### v15.3

- [Daemon] Fix the bug that only one script would be executed in post-fs-data.d/service.d
- [Daemon] Add `MS_SILENT` flag when mounting, should fix some devices that cannot mount magisk.img
- [MagiskBoot] Fix potential segmentation fault when patching ramdisk, should fix some installation failures

### v15.2

- [MagiskBoot] Fix dtb verity patches, should fix dm-verity bootloops on newer devices placing fstabs in dtb
- [MagiskPolicy] Add new rules for proper Samsung support, should fix MagiskHide
- [MagiskInit] Support non `skip_initramfs` devices using split sepolicies (e.g. Zenfone 4 Oreo)
- [Daemon] Use specific logcat buffers, some devices does not support all log buffers
- [scripts] Update scripts to double check whether boot slot is available, some devices set a boot slot without A/B partitions

### v15.1

- [MagiskBoot] Fix faulty code in ramdisk patches which causes bootloops in some config and fstab format combos

### v15.0

- [Daemon] Fix the bug that Magisk cannot properly detect /data encryption state
- [Daemon] Add merging `/cache/magisk.img` and `/data/adb/magisk_merge.img` support
- [Daemon] Update to upstream libsepol to support cutting edge split policy custom ROM cil compilations

### v14.6 (1468)

- [General] Move all files into a safe location: /data/adb
- [Daemon] New invincible implementation: use `magiskinit_daemon` to monitor sockets
- [Daemon] Rewrite logcat monitor to be more efficient
- [Daemon] Fix a bug where logcat monitor may spawn infinite logcat processes
- [MagiskSU] Update su to work the same as proper Linux implementation:
Initialize window size; all environment variables will be migrated (except HOME, SHELL, USER, LOGNAME, these will be set accordingly),
"--preserve-environment" option will preserve all variables, including those four exceptions.
Check the Linux su manpage for more info
- [MagiskBoot] Massive refactor, rewrite all cpio operations and CLI
- [MagiskInit][MagiskBoot] Support ramdisk high compression mode

### v14.5 (1456)

- [Magiskinit] Fix bootloop issues on several devices
- [misc] Build binaries with NDK r10e, should get rid of the nasty linker warning when executing magisk

### v14.5 (1455)

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

### v14.3 (1437)

- [MagiskBoot] Fix Pixel C installtion
- [MagiskBoot] Handle special `lz4_legacy` format properly, should fix all LG devices
- [Daemon] New universal logcat monitor is added, support plug-and-play to worker threads
- [Daemon] Invincible mode: daemon will be restarted by init, everything should seamlessly through daemon restarts
- [Daemon] Add new restorecon action, will go through and fix all Magisk files with selinux unlabled to `system_file` context
- [Daemon] Add brute-force image resizing mode, should prevent the notorious Samsung crappy resize2fs from affecting the result
- [resetprop] Add new "-p" flag, used to toggle whether alter/access the actual persist storage for persist props

### v14.2

- [MagicMount] Clone attributes to tmpfs mountpoint, should fix massive module breakage

### v14.1

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
- [MagicMount] Remove usage of dummy folder, directly mount tmpfs and constuct file structure skeleton in place

### v14.0

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

### v13.3

- [MagiskHide] Update to bypass Google CTS (2017.7.17)
- [resetprop] Properly support removing persist props
- [uninstaller] Remove Magisk Manager and persist props

### v13.2

- [magiskpolicy] Fix magiskpolicy segfault on old Android versions, should fix tons of older devices that couldn't use v13.1
- [MagiskHide] Set proper selinux context while re-linking /sbin to hide Magisk, should potentially fix many issues
- [MagiskBoot] Change lzma compression encoder flag from `LZMA_CHECK_CRC64` to `LZMA_CHECK_CRC32`, kernel only supports latter
- [General] Core-only mode now properly mounts systemless hosts and magiskhide

### v13.1

- [General] Merge MagiskSU, magiskhide, resetprop, magiskpolicy into one binary
- [General] Add Android O support (tested on DP3)
- [General] Dynamic link libselinux.so, libsqlite.so from system to greatly reduce binary size
- [General] Remove bundled busybox because it casues a lot of issues
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

### v12.0

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

### v11.5/11.6

- [Magic Mount] Fix mounting issues with devices that have separate /vendor partitions
- [MagiskBoot] Whole new boot image patching tool, please check release note for more info
- [magiskpolicy] Rename sepolicy-inject to magiskpolicy
- [magiskpolicy] Update a rule to allow chcon everything properly
- [MagiskHide] Prevent multirom crashes
- [MagiskHide] Add patches for ro.debuggable, ro.secure, ro.build.type, ro.build.tags, ro.build.selinux
- [MagiskHide] Change /sys/fs/selinux/enforce, /sys/fs/selinux/policy permissions for Samsung compatibility
- [MagiskSU] Fix read-only partition mounting issues
- [MagiskSU] Disable -cn option, the option will do nothing, preserved for compatibility

### v11.1

- [sepolicy-inject] Add missing messages
- [magiskhide] Start MagiskHide with scripts

### v11.0

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

### v10.2

- [Magic Mount] Remove apps/priv-app from whitelist, should fix all crashes
- [phh] Fix binary out-of-date issue
- [scripts] Fix root disappear issue when upgrading within Magisk Manager

### v10

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

### v9

- **[API Change] Remove the interface for post-fs modules**
- [resetprop] New tool "resetprop" is added to Magisk to replace most post-fs modules' functionality
- [resetprop] Magisk will now patch "ro.boot.verifiedbootstate", "ro.boot.flash.locked", "ro.boot.veritymode" to bypass Safety Net
- [Magic Mount] Move dummy skeleton / mirror / mountinfo filesystem tree to tmpfs
- [Magic Mount] Rewritten dummy cloning mechanism from scratch, will result in minimal bind mounts, minimal file traversal, eliminate all possible issues that might happen in extreme cases
- [Magic Mount] Adding new items to /systen/bin, /system/vendor, /system/lib(64) is properly supported (devices with seperate vendor partition is not supported yet)
- [Magisk Hide] Rewritten from scratch, now run in daemon mode, proper list monitoring, proper mount detection, and maybe more.....
- [Boot Image] Add support for Motorola boot image dtb, it shall now unpack correctly
- [Uninstaller] Add removal of SuperSU custom patch script

### v8

- Add Magisk Hide to bypass SafetyNet
- Improve SuperSU integration: no longer changes the SuperSU PATH
- Support rc script entry points not located in init.rc

### v7

- Fully open source
- Remove supolicy dependency, use my own sepolicy-injection
- Run everything in its own selinux domain, should fix all selinux issues
- Add Note 7 stock kernel hex patches
- Add support to install Magisk in Magisk Manager
- Add support for image merging for module flashing in Magisk Manager
- Add root helpers for SuperSU auto module-ize and auto upgrading legacy phh superuser
- New paths to toggle busybox, and support all root solutions
- Remove root management API; both SuperSU and phh has their own superior solutions

### v6

- Fixed the algorithm for adding new files and dummy system
- Updated the module template with a default permission, since people tend to forget them :)

### v5

- Hotfix for older Android versions (detect policy before patching)
- Update uninstaller to NOT uninstall Magisk Manager, since it cause problems

### v4

- Important: Uninstall v1 - v3 Magisk before upgrading with the uninstaller in the OP!!
- Massive Rewrite Magisk Interface API! All previous mods are NOT compatible! Please download the latest version of the mods you use (root/xposed)
- Mods are now installed independently in their own subfolder. This paves the way for future Magisk Manager versions to manage mods, **just like how Xposed Modules are handled**
- Support small boot partition devices (Huawei devices)
- Use minimal sepolicy patch in boot image for smaller ramdisk size. Live patch policies after bootup
- Include updated open source sepolicy injection tool (source code available), support nearly all SuperSU supolicy tool's functionality

### v3

- Fix bootimg-extract for Exynos Samsung devices (thanks to @phhusson), should fix all Samsung device issues
- Add supolicy back to patch sepolicy (stock Samsung do not accept permissive domain)
- Update sepolicy-injection to patch su domain for Samsung devices to use phh's root
- Update root disable method, using more aggressive approach
- Use lazy unmount to unmount root from system, should fix some issues with custom roms
- Use the highest possible compression rate for ramdisk, hope to fix some devices with no boot partition space
- Detect boot partition space insufficient, will abort installer instead of breaking your device

### v2

- Fix verity patch. It should now work on all devices (might fix some of the unable-to-boot issues)
- All scripts will now run in selinux permissive mode for maximum compatibility (this will **NOT** turn your device to permissive)
- Add Nougat Developer Preview 5 support
- Add systemless host support for AdBlock Apps (enabled by default)
- Add support for new root disable method
- Remove sepolicy patches that uses SuperSU's supolicy tool; it is now using a minimal set of modifications
- Removed Magisk Manager in Magisk patch, it is now included in Magisk phh's superuser only

### v1
- Initial releaseFrom 7b9ef3d1b4fded998e7da1f59f6297182dd4270a Mon Sep 17 00:00:00 2001
From: MASVA <masva10@gmail.com>
Date: Thu, 8 Oct 2020 05:42:14 +0200
Subject: [PATCH] update croatian language

---
 app/src/main/res/values-hr/strings.xml  | 271 ++++++++++++++++--------
 stub/src/main/res/values-hr/strings.xml |   8 +-
 2 files changed, 194 insertions(+), 85 deletions(-)

diff --git a/app/src/main/res/values-hr/strings.xml b/app/src/main/res/values-hr/strings.xml
index 622af1d174..51d2c8e19c 100644
--- a/app/src/main/res/values-hr/strings.xml
+++ b/app/src/main/res/values-hr/strings.xml
@@ -6,73 +6,154 @@
     <string name="superuser">Superuser</string>
     <string name="logs">Zapisnik događaja</string>
     <string name="settings">Postavke</string>
+    <string name="refresh">Osvježite lokalne podatke</string>
     <string name="install">Instaliranje</string>
+    <string name="section_home">Početna</string>
+    <string name="section_theme">Teme</string>
+    <string name="safetynet">SafetyNet</string>
+
+    <!--Home-->
+    <string name="no_connection">Nema dostupne veze</string>
+    <string name="app_changelog">Popis izmjena</string>
+    <string name="manager">Manager</string>
+    <string name="loading">Učitavanje…</string>
+    <string name="update">Ažuriraj</string>
+    <string name="not_available">N/A</string>
+    <string name="hide">Sakrij</string>
+    <string name="status">Status</string>
+    <string name="home_package">Paket</string>
+
+    <string name="home_notice_content">Uvijek provjerite koristite li Magisk Manager s otvorenim kodom. Manager nepoznatog izvora može izvoditi zlonamjerne radnje.</string>
+    <string name="home_support_title">Podržite nas</string>
+    <string name="home_item_source">Izvor</string>
+    <string name="home_support_content">Magisk je i uvijek će biti besplatan i otvoren. Međutim, možete nam pokazati da vam je stalo slanjem male donacije.</string>
+    <string name="home_status_normal">Normalno</string>
+    <string name="home_status_stub">Stub</string>
+    <string name="home_installed_version">Instalirana</string>
+    <string name="home_latest_version">Najnovija</string>
+    <string name="invalid_update_channel">Nevažeći kanal ažuriranja</string>
+    <string name="uninstall_magisk_title">Deinstaliraj Magisk</string>
+    <string name="uninstall_magisk_msg">Svi će moduli biti onemogućeni/uklonjeni!\nRoot će biti uklonjen!\nVaši su podaci potencijalno šifrirani, ako već nisu!</string>
+    <string name="home_check_safetynet">Provjeri SafetyNet</string>
 
-    <!--Status Fragment-->
-
-    <string name="safetynet_api_error">SafetyNet API greška</string>
-    <string name="safetynet_res_invalid">Odgovor je nevažeći</string>
-
-    <!--Install Fragment-->
+    <!--Install-->
     <string name="keep_force_encryption">Zadrži prisilno šifirannje</string>
     <string name="keep_dm_verity">Zadrži AVB 2.0/dm-verity</string>
-    <string name="uninstall_magisk_title">Deinstaliraj Magisk</string>
-    <string name="uninstall_magisk_msg">Svi moduli će biti onemogućeni/uklonjeni. Root će biti uklonjen i potencijalno šifrirati Vaše podatke, ukoliko Vaši podaci trenutačno nisu šifrirani</string>
-    <string name="update">Ažuriraj</string>
-
-    <!--Module Fragment-->
-    <string name="no_info_provided">(Nema podataka)</string>
-
-    <!--Repo Fragment-->
-    <string name="update_available">Ažuriranje dostupno</string>
-    <string name="home_installed_version">Instalirano</string>
-
-    <!--Log Fragment-->
-    <string name="menuSaveLog">"Spremi zapisnik "</string>
-    <string name="menuClearLog">Očisti zapisnik sada</string>
-    <string name="logs_cleared">Zapisnik je uspješno izbrisan</string>
+    <string name="recovery_mode">Recovery način</string>
+    <string name="install_options_title">Opcije</string>
+    <string name="install_method_title">Metoda</string>
+    <string name="install_next">Dalje</string>
+    <string name="install_start">Krenimo</string>
+    <string name="manager_download_install">Dodirnite za preuzimanje i instalaciju</string>
+    <string name="download_zip_only">Preuzmi samo zip</string>
+    <string name="direct_install">Izravna instalacija (preporučeno)</string>
+    <string name="install_inactive_slot">Instaliraj u neaktivni utor (Nakon OTA)</string>
+    <string name="install_inactive_slot_msg">Vaš će se uređaj prisiliti pokrenuti s trenutnog neaktivnog mjesta nakon ponovnog pokretanja!\nKoristite tu opciju samo nakon što je OTA dovršena.\nContinue?</string>
+    <string name="setup_title">Dodatno postavljanje</string>
+    <string name="select_patch_file">Odaberite patch datoteku</string>
+    <string name="patch_file_msg">Odaberite raw (*.img) ili ODIN tarfile (*.tar)</string>
+    <string name="reboot_delay_toast">Ponovno pokretanje za 5 sekundi…</string>
+    <string name="flash_screen_title">Instalacija</string>
 
-    <!--About Activity-->
-    <string name="app_changelog">Popis izmjena aplikacije</string>
+    <!--Superuser-->
+    <string name="su_request_title">Superuser zahtjev</string>
+    <string name="deny">Odbij</string>
+    <string name="prompt">Upit</string>
+    <string name="grant">Odobri</string>
+    <string name="su_warning">Odobren puni pristup vašem uređaju.\nOdbijte ako niste sigurni!</string>
+    <string name="forever">Zauvijek</string>
+    <string name="once">Jednom</string>
+    <string name="tenmin">10 min</string>
+    <string name="twentymin">20 min</string>
+    <string name="thirtymin">30 min</string>
+    <string name="sixtymin">60 min</string>
+    <string name="su_allow_toast">%1$s su dodijeljena Superuser prava</string>
+    <string name="su_deny_toast">%1$s su odbijena Superuser prava</string>
+    <string name="su_snack_grant">Superuser prava su dodijeljena za %1$s</string>
+    <string name="su_snack_deny">Superuser prava su odbijena za %1$s</string>
+    <string name="su_snack_notif_on">Obavijesti za %1$s su odobrene</string>
+    <string name="su_snack_notif_off">Obavijesti za %1$s su odbijene</string>
+    <string name="su_snack_log_on">Zapisnik događaja omogućen je za %1$s</string>
+    <string name="su_snack_log_off">Zapisnik događaja onemogućen je za %1$s</string>
+    <string name="su_revoke_title">Opozvati?</string>
+    <string name="su_revoke_msg">Potvrdite da biste opozvali prava za %1$s?</string>
+    <string name="toast">Pop-up</string>
+    <string name="none">Nijedan</string>
 
-    <!--Toasts, Dialogs-->
-    <string name="repo_install_title">Instaliraj %1$s</string>
-    <string name="repo_install_msg">Da li želite instalirati %1$s sada?</string>
-    <string name="download">Preuzmi</string>
-    <string name="reboot">Ponovno podizanje sustava</string>
-    <string name="magisk_update_title">Dostupno je novo Magisk ažuriranje!</string>
-    <string name="release_notes">Bilješke o izdavanju aplikacije</string>
-    <string name="repo_cache_cleared">Predmemorija repozitorija izbrisana</string>
-    <string name="manager_update_title">Dostupno je novo ažuriranje Magisk Manager aplikacije!</string>
-    <string name="manager_download_install">Pritisnite za preuzimanje i instalaciju</string>
-    <string name="update_channel">Magisk ažuriranja</string>
-    <string name="flashing">Apliciranje</string>
-    <string name="hide_manager_title">Skrivanje Magisk Manager aplikacije…</string>
-    <string name="hide_manager_fail_toast">Skrivanje Magisk Manager nije uspjelo</string>
-    <string name="download_zip_only">Preuzmi samo zip</string>
-    <string name="direct_install">Izravna instalacija (preporuča se)</string>
-    <string name="complete_uninstall">Potpuna deinstalacija</string>
-    <string name="restore_done">Obnova je dovršena!</string>
-    <string name="restore_fail">Stock backup does not exist!</string>
-    <string name="proprietary_title">Preuzmite vlasnički kod</string>
-    <string name="proprietary_notice">Magisk Manager je FOSS aplikacija te ne sadrži Googleov SafetyNet API kod.\n\nDopuštate li Magisk Manager aplikaciji da preuzme proširenje (sadrži GoogleApiClient) za SafetyNet provjere?</string>
-    <string name="download_file_error">Pogreška prilikom preuzimanja datoteke</string>
+    <string name="superuser_toggle_notification">Obavijesti</string>
+    <string name="superuser_toggle_revoke">Opozovi</string>
+    <string name="superuser_policy_none">Još nijedna aplikacija nije zatražila Superuser prava.</string>
+
+    <!--Logs-->
+    <string name="log_data_none">Nemate zapisnika, pokušajte više upotrebljavati svoje aplikacije s omogućenim SU.</string>
+    <string name="log_data_magisk_none">Zapisnici Magiska su prazni, to je čudno.</string>
+    <string name="menuSaveLog">Spremi zapisnik</string>
+    <string name="menuClearLog">Izbriši zapisnik</string>
+    <string name="logs_cleared">Zapisnik uspješno izbrisan.</string>
+    <string name="pid">PID: %1$d</string>
+    <string name="target_uid">Ciljani UID: %1$d</string>
 
-    <!--Settings Activity -->
-    <string name="settings_clear_cache_title">Izbriši predmemoriju repozitorija</string>
-    <string name="settings_clear_cache_summary">Izbrišite predmemorirane informacije za online repozitorij, prisiljavajući aplikaciju da se osvježi online</string>
+    <!--SafetyNet-->
+    <string name="safetynet_api_error">SafetyNet API greška</string>
+    <string name="safetynet_res_invalid">Odgovor je nevažeći</string>
+    <string name="safetynet_attest_success">Uspjeh!</string>
+    <string name="safetynet_attest_failure">Provjera nije uspjela!</string>
+    <string name="safetynet_attest_loading">Samo trenutak…</string>
+    <string name="safetynet_attest_restart">Pokušajte ponovo</string>
+
+    <!-- MagiskHide -->
+    <string name="show_system_app">Prikaži sistemske aplikacije</string>
+    <string name="hide_filter_hint">Filtriraj prema nazivu</string>
+    <string name="hide_scroll_up">Pomicanje gore</string>
+    <string name="hide_filters">Filteri</string>
+    <string name="hide_search">Traži</string>
+
+    <!--Module -->
+    <string name="no_info_provided">(Nema podataka)</string>
+    <string name="reboot_recovery">Ponovo pokreni u Recovery</string>
+    <string name="reboot_bootloader">Ponovo pokreni u Bootloader</string>
+    <string name="reboot_download">Ponovo pokreni u Download</string>
+    <string name="reboot_edl">Ponovo pokreni u EDL</string>
+    <string name="module_version_author">%1$s od %2$s</string>
+    <string name="module_section_pending">Ažuriranja</string>
+    <string name="module_section_pending_action">Ažuriraj sve</string>
+    <string name="module_state_remove">Ukloni</string>
+    <string name="module_state_restore">Vrati</string>
+    <string name="module_action_install_external">Instaliraj iz pohrane</string>
+    <string name="update_available">Ažuriranje dostupno</string>
+    <string name="module_section_online">Online</string>
+    <string name="sorting_order">Redoslijed sortiranja</string>
+
+    <!--Settings -->
+    <string name="settings_dark_mode_title">Teme</string>
+    <string name="settings_dark_mode_message">Odaberite temu koja najbolje odgovara vašem stilu!</string>
+    <string name="settings_dark_mode_light">Uvijek svijetla</string>
+    <string name="settings_dark_mode_system">Prati sustav</string>
+    <string name="settings_dark_mode_dark">Uvijek crna</string>
+    <string name="settings_download_path_title">Putanja preuzimanja</string>
+    <string name="settings_download_path_message">Datoteke će biti spremljene u %1$s</string>
+    <string name="settings_clear_cache_title">Obriši predmemoriju repozitorija</string>
+    <string name="settings_clear_cache_summary">Obrišite predmemorirane podatke za mrežne repozitorije. To prisiljava aplikaciju da se osvježi na mreži</string>
     <string name="settings_hide_manager_title">Sakrij Magisk Manager</string>
-    <string name="settings_hide_manager_summary">Privremeno sakrij Magisk Manager.\nTo će instalirati novu aplikaciju pod nazivom \"Otkrij Magisk Manager\"</string>
+    <string name="settings_hide_manager_summary">Prepakiraj Magisk Manager s nasumičnim nazivima paketa i aplikacija</string>
+    <string name="settings_restore_manager_title">Vrati Magisk Manager</string>
+    <string name="settings_restore_manager_summary">Vrati Magisk Manager s izvornim nazivima paketa i aplikacija</string>
     <string name="language">Jezik</string>
-    <string name="system_default">(Zadana postavka sustava)</string>
+    <string name="system_default">(Zadano sustavom)</string>
+    <string name="settings_check_update_title">Provjeri za ažuriranja</string>
+    <string name="settings_check_update_summary">Povremeno provjeravaj ima li ažuriranja u pozadini</string>
     <string name="settings_update_channel_title">Kanal ažuriranja</string>
-    <string name="settings_update_stable">Stabilno</string>
+    <string name="settings_update_stable">Stabilna</string>
     <string name="settings_update_beta">Beta</string>
-
-    <string name="settings_magiskhide_summary">Sakrij Magisk od raznih detekcija</string>
-    <string name="settings_hosts_title">Systemless hostovi</string>
-    <string name="settings_hosts_summary">Systemless hostovi podržavaju Adblock aplikacije</string>
-
+    <string name="settings_update_custom">Vlastiti kanal</string>
+    <string name="settings_update_custom_msg">Umetnite prilagođeni URL</string>
+    <string name="settings_magiskhide_summary">Sakrij Magisk od različitih oblika otkrivanja</string>
+    <string name="settings_hosts_title">Hostovi bez sistema</string>
+    <string name="settings_hosts_summary">Podrška za hostove bez sistema za Adblock aplikacije</string>
+    <string name="settings_hosts_toast">Dodan je modul hosta bez sistema</string>
+    <string name="settings_app_name_hint">Novi naziv</string>
+    <string name="settings_app_name_helper">Aplikacija će se prepakirati na ovaj naziv</string>
+    <string name="settings_app_name_error">Nevažeći format</string>
     <string name="settings_su_app_adb">Aplikacije i ADB</string>
     <string name="settings_su_app">Samo aplikacije</string>
     <string name="settings_su_adb">Samo ADB</string>
@@ -88,7 +169,14 @@
     <string name="request_timeout">Vremensko ograničenje zahtjeva</string>
     <string name="superuser_notification">Superuser obavijest</string>
     <string name="settings_su_reauth_title">Ponovno provjerite autentičnost nakon ažuriranja</string>
-    <string name="settings_su_reauth_summary">Ponovno provjerite autentičnost Superuser dopuštenja nakon ažuriranja aplikacije</string>
+    <string name="settings_su_reauth_summary">Ponovno provjerite autentičnost Superuser-a dopuštenja nakon ažuriranja aplikacije</string>
+    <string name="settings_su_biometric_title">Omogući biometrijsku provjeru autentičnosti</string>
+    <string name="settings_su_biometric_summary">Koristi biometrijsku provjeru autentičnosti da biste omogućili zahtjeve Superuser-a</string>
+    <string name="no_biometric">Nepodržani uređaj ili nije omogućena biometrijska provjera autentičnosti</string>
+    <string name="settings_customization">Prilagodba</string>
+    <string name="setting_add_shortcut_summary">Dodajte lijepi prečac na početni zaslon u slučaju da je naziv i ikonu teško prepoznati nakon skrivanja aplikacije</string>
+    <string name="settings_doh_title">DNS preko HTTPS-a</string>
+    <string name="settings_doh_description">Zaobilazno trovanje DNS-om u nekim zemljama</string>
 
     <string name="multiuser_mode">Višekorisnički način rada</string>
     <string name="settings_owner_only">Samo vlasnik uređaja</string>
@@ -106,32 +194,47 @@
     <string name="requester_summary">Root sesije će naslijediti imenski prostor tražitelja</string>
     <string name="isolate_summary">Svaka root sesija ima svoj vlastiti imenski prostor</string>
 
-    <!--Superuser-->
-    <string name="su_request_title">Superuser zahtjev</string>
-    <string name="deny">Odbij</string>
-    <string name="prompt">Upitaj</string>
-    <string name="grant">Odobri</string>
-    <string name="su_warning">Omogućuje potpuni pristup Vašem uređaju.\nOdbijte ako niste sigurni!</string>
-    <string name="forever">Zauvijek</string>
-    <string name="once">Jednom</string>
-    <string name="tenmin">10 min</string>
-    <string name="twentymin">20 min</string>
-    <string name="thirtymin">30 min</string>
-    <string name="sixtymin">60 min</string>
-    <string name="su_allow_toast">%1$s su dodijeljena Superuser prava</string>
-    <string name="su_deny_toast">%1$s su odbijena Superuser prava</string>
-    <string name="su_snack_grant">Superuser prava su dodijeljena za %1$s</string>
-    <string name="su_snack_deny">Superuser prava su odbijena za %1$s</string>
-    <string name="su_snack_notif_on">Obavijesti za %1$s su odobrene</string>
-    <string name="su_snack_notif_off">Obavijesti za %1$s su odbijene</string>
-    <string name="su_snack_log_on">Zapisnik događaja omogućen je za %1$s</string>
-    <string name="su_snack_log_off">Zapisnik događaja onemogućen je za %1$s</string>
-    <string name="su_revoke_title">Opozvati?</string>
-    <string name="su_revoke_msg">Potvrdite da biste opozvali prava za %1$s?</string>
-    <string name="toast">Pop-up</string>
-    <string name="none">Nijedan</string>
-
-    <!--Superuser logs-->
-    <string name="target_uid">Ciljani UID: %1$d</string>
+    <!--Notifications-->
+    <string name="update_channel">Magisk Ažuriranja</string>
+    <string name="progress_channel">Obavijesti o napretku</string>
+    <string name="download_complete">Preuzimanje završeno</string>
+    <string name="download_file_error">Pogreška pri preuzimanju datoteke</string>
+    <string name="download_open_parent">Prikaži u nadređenoj mapi</string>
+    <string name="download_open_self">Prikaži datoteku</string>
+    <string name="magisk_update_title">Dostupno je Magisk ažuriranje!</string>
+    <string name="manager_update_title">Dostupno je Magisk Manager ažuriranje! </string>
 
+    <!--Toasts, Dialogs-->
+    <string name="yes">Da</string>
+    <string name="no">Ne</string>
+    <string name="repo_install_title">Instaliraj %1$s</string>
+    <string name="repo_install_msg">Da li želite instalirati %1$s sada?</string>
+    <string name="download">Preuzmi</string>
+    <string name="reboot">Ponovno pokreni</string>
+    <string name="release_notes">Bilješke o izdavanju aplikacije</string>
+    <string name="repo_cache_cleared">Predmemorija repozitorija izbrisana</string>
+    <string name="flashing">Apliciranje…</string>
+    <string name="done">Dovršeno!</string>
+    <string name="failure">Nije uspjelo</string>
+    <string name="hide_manager_title">Skrivanje Magisk Manager-a…</string>
+    <string name="hide_manager_fail_toast">Skrivanje Magisk Manager-a nije uspjelo.</string>
+    <string name="open_link_failed_toast">Nije pronađena aplikacija za otvaranje veze</string>
+    <string name="complete_uninstall">Dovršite deinstalaciju</string>
+    <string name="restore_img">Vrati Images</string>
+    <string name="restore_img_msg">Vraćanje…</string>
+    <string name="restore_done">Vraćanje je dovršeno!</string>
+    <string name="restore_fail">Izvorna Sig. kopija ne postoji!</string>
+    <string name="proprietary_title">Preuzmite vlasnički kod</string>
+    <string name="proprietary_notice">Magisk Manager je FOSS i ne sadrži Googleov zaštićeni API sigurnosni kod.\n\nHoćete li dopustiti Magisk Manageru da preuzme proširenje (sadrži GoogleApiClient) za SafetyNet provjeru?</string>
+    <string name="setup_fail">Postavljanje nije uspjelo</string>
+    <string name="env_fix_title">Zahtijeva dodatno postavljanje</string>
+    <string name="env_fix_msg">Vašem je uređaju potrebno dodatno postavljanje kako bi Magisk ispravno radio. Preuzet će se zip za postavljanje Magiska, želite li nastaviti sada?</string>
+    <string name="setup_msg">Postavljanje okruženja za pokretanje…</string>
+    <string name="authenticate">Potvrdi</string>
+    <string name="unsupport_magisk_title">Nepodržana Magisk verzija</string>
+    <string name="unsupport_magisk_msg">Ova verzija Magisk Manager-a ne podržava Magisk verziju nižu od %1$s.\n\nAplikacija će se ponašati kao da nije instaliran Magisk, ažurirajte Magisk što je prije moguće.</string>
+    <string name="external_rw_permission_denied">Odobrite dopuštenje za pohranu da biste omogućili ovu funkciju</string>
+    <string name="add_shortcut_title">Dodaj prečac na početni zaslon</string>
+    <string name="add_shortcut_msg">Nakon skrivanja Magisk Manager-a, njegov naziv i ikonu moglo bi postati teško prepoznati. Želite li dodati lijepi prečac na početni zaslon?</string>
+    <string name="app_not_found">Nije pronađena nijedna aplikacijaza obradu ove radnju</string>
 </resources>
diff --git a/stub/src/main/res/values-hr/strings.xml b/stub/src/main/res/values-hr/strings.xml
index 7abc06d3ba..82901c97f1 100644
--- a/stub/src/main/res/values-hr/strings.xml
+++ b/stub/src/main/res/values-hr/strings.xml
@@ -1 +1,7 @@
-<resources></resources>
+<?xml version="1.0" encoding="utf-8"?>
+<resources>
+    <string name="upgrade_msg">Nadogradite na full Magisk Manager da biste dovršili postavljanje. Preuzeti i instalirati?</string>
+    <string name="no_internet_msg">Povežite se na Internet! Potrebna je nadogradnja na full Magisk Manager.</string>
+    <string name="dling">Preuzimanje</string>
+    <string name="relaunch_app">Ručno ponovno pokrenite aplikaciju</string>
+</resources>
