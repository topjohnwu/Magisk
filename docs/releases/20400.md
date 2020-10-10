## 2020.3.23 Magisk v20.4

### Miscellaneous
This release is mainly focused on stability and bug squashing. Please be aware that MagiskHide is no longer enabled by default. Since Google has enabled [hardware-based key attestation](https://twitter.com/topjohnwu/status/1237656703929180160?s=20) in SafetyNet ([FAQ](https://twitter.com/topjohnwu/status/1237830555523149824?s=20)), there is no effective way to pass full CTS SafetyNet anymore (although Google seems to have temporarily [reverted the change](https://twitter.com/topjohnwu/status/1238514375150850048?s=20)).

I decided that the fully redesigned Magisk Manager isn't fully ready for prime time yet, so this time around no Magisk Manager update is released. The WIP manager will continue to be improved and is available for testing on the canary channel.

### BusyBox Standalone Mode
Starting with Magisk v20.4, all Magisk related scripts, including boot scripts and module installation scripts, will run on BusyBox's shell (ash) in **standalone mode**. In BusyBox ash standalone mode, **every single command** will be **forced** to use the one that is in Magisk's BusyBox (if available). For instance, no matter how you change the environment variable `PATH`, the `rm` command will always use the one in BusyBox, not the one in system, external BusyBox, vendor, or included in custom recovery.

The reason behind this change is that all scripts will be guaranteed to have 100% consistent results no matter how the environment is setup. The internal BusyBox is significantly beefed up with patches from @osm0sis, and also with SELinux features enabled. It shall offer a very complete, reliable, and consistent scripting environment. If in any case you **require** to use a command outside of BusyBox, please call it with the full path (e.g. `/system/bin/nslookup`)

### Magisk Changelog
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
