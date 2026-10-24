# Android Booting Shenanigans

## Terminologies

- **rootdir**: the root directory (`/`). All files/folders/filesystems are stored in or mounted under rootdir. On Android, the filesystem may be either `rootfs` or the `system` partition.
- **`initramfs`**: a section in Android's boot image that the Linux kernel will use as `rootfs`. People also use the term **ramdisk** interchangeably
- **`recovery` and `boot` partition**: these 2 are actually very similar: both are Android boot images containing ramdisk and Linux kernel (plus some other stuff). The only difference is that booting `boot` partition will bring us to Android, while `recovery` has a minimalist self contained Linux environment for repairing and upgrading the device.
- **SAR**: System-as-root. That is, the device uses `system` as rootdir instead of `rootfs`
- **A/B, A-only**: For devices supporting [Seamless System Updates](https://source.android.com/docs/core/ota/ab), it will have 2 slots of all read-only partitions; we call these **A/B devices**. To differentiate, non A/B devices will be called **A-only**
- **2SI**: Two Stage Init. The way Android 10+ boots. More info later.

Here are a few parameters to more precisely define a device's Android version:

- **LV**: Launch Version. The Android version the device is **launched** with. That is, the Android version pre-installed when the device first hit the market.
- **RV**: Running Version. The Android version the device is currently running on.

We will use **Android API level** to represent LV and RV. The mapping between API level and Android versions can be seen in [this table](https://source.android.com/docs/setup/start/build-numbers#platform-code-names-versions-api-levels-and-ndk-releases). For example: Pixel XL is released with Android 7.1, and is running Android 10, these parameters will be `(LV = 25, RV = 29)`

## Boot Methods

Android booting can be roughly categorized into 3 major different methods. We provide a general rule of thumb to determine which method your device is most likely using, with exceptions listed separately.

Method | Initial rootdir | Final rootdir
:---: | --- | ---
**A** | `rootfs` | `rootfs`
**B** | `system` | `system`
**C** | `rootfs` | `system`

- **Method A - Legacy ramdisk**: This is how *all* Android devices used to boot (good old days). The kernel uses `initramfs` as rootdir, and exec `/init` to boot.
	- Devices that do not fall in any of Method B and C's criteria
- **Method B - Legacy SAR**: This method was first seen on Pixel 1. The kernel directly mounts the `system` partition as rootdir and exec `/init` to boot.
	- Devices with `(LV = 28)`
	- Google: Pixel 1 and 2. Pixel 3 and 3a when `(RV = 28)`.
	- OnePlus: 6 - 7
	- Maybe some `(LV < 29)` Android Go devices?
- **Method C - 2SI ramdisk SAR**: This method was first seen on Pixel 3 Android 10 developer preview. The kernel uses `initramfs` as rootdir and exec `/init` in `rootfs`. This `init` is responsible to mount the `system` partition and use it as the new rootdir, then finally exec `/system/bin/init` to boot.
	- Devices with `(LV >= 29)`
	- Devices with `(LV < 28, RV >= 29)`, excluding those that were already using Method B
	- Google: Pixel 3 and 3a with `(RV >= 29)`

### Discussion

From documents online, Google's definition of SAR only considers how the kernel boots the device (**Initial rootdir** in the table above), meaning that only devices using **Method B** are *officially* considered SAR devices from Google's standpoint.

However for Magisk, the real difference lies in what the device ends up using when fully booted (**Final rootdir** in the table above), meaning that **as far as Magisk is concerned, both Method B and C are a form of SAR**, but just implemented differently. Every instance of SAR later mentioned in this document will refer to **Magisk's definition** unless specifically stated otherwise.

The criteria for Method C is a little complicated, in layman's words: either your device is modern enough to launch with Android 10+, or you are running an Android 10+ custom ROM on a device that was using Method A.

- Any Method A device running Android 10+ will automatically be using Method C
- **Method B devices are stuck with Method B**, with the only exception being Pixel 3 and 3a, which Google retrofitted the device to adapt the new method.

SAR is a very important part of [Project Treble](https://source.android.com/docs/core/architecture#hidl) as rootdir should be tied to the platform. This is also the reason why Method B and C come with `(LV >= ver)` criteria as Google has enforced all OEMs to comply with updated requirements every year.

## Some History

When Google released the first generation Pixel, it also introduced [A/B (Seamless) System Updates](https://source.android.com/docs/core/ota/ab). Due to [storage size concerns](https://source.android.com/docs/core/ota/ab/ab_faqs), there are several differences compared to A-only, the most relevant one being the removal of `recovery` partition and the recovery ramdisk being merged into `boot`.

Let's go back in time when Google was first designing A/B. If using SAR (only Boot Method B existed at that time), the kernel doesn't need `initramfs` to boot Android (because rootdir is in `system`). This means we can be smart and just stuff the recovery ramdisk (containing the minimalist Linux environment) into `boot`, remove `recovery`, and let the kernel pick whichever rootdir to use (ramdisk or `system`) based on information from the bootloader.

As time passed from Android 7.1 to Android 10, Google introduced [Dynamic Partitions](https://source.android.com/docs/core/ota/dynamic_partitions/implement). This is bad news for SAR, because the Linux kernel cannot directly understand this new partition format, thus unable to directly mount `system` as rootdir. This is when they came up with Boot Method C: always boot into `initramfs`, and let userspace handle the rest of booting. This includes deciding whether to boot into Android or recovery, or as they officially call: `USES_RECOVERY_AS_BOOT`.

Some modern devices using A/B with 2SI also come with `recovery_a/_b` partitions. This is officially supported with Google's standard. These devices will then only use the boot ramdisk to boot into Android as recovery is stored on a separate partition.

Starting with Android 13 (`LV >= 33`) and [Generic Kernel Image (GKI) 2.0](https://source.android.com/docs/core/architecture/kernel/generic-kernel-image), Google decoupled the generic kernel from the generic ramdisk so that the `boot` partition contains only the GKI kernel, moving the generic first-stage ramdisk into a separate `init_boot` partition (or `vendor_boot` on some devices). From the booting perspective, these devices still use Boot Method C (2SI); the bootloader simply loads and concatenates the ramdisk(s) from `init_boot` / `vendor_boot` in memory before handing off to `/init`.

## Piecing Things Together

With all the knowledge above, now we can categorize all Android devices into these different types:

Type | Boot Method | Partition | 2SI | Ramdisk in `boot` (or `init_boot`/`vendor_boot`)
:---: | :---: | :---: | :---: | :---:
**I** | A | A-only | No | `boot` ramdisk
**II** | B | A/B | Any | `recovery` ramdisk
**III** | B | A-only | Any | ***N/A***
**IV** | C | Any | Yes | Hybrid / 2SI ramdisk

These types are ordered chronologically by the time they were first available.

- **Type I**: Good old legacy ramdisk boot
- **Type II**: Legacy A/B devices. Pixel 1 is the first device of this type, being both the first A/B and SAR device
- **Type III**: Late 2018 - 2019 devices that are A-only. **The worst type of device to ever exist as far as Magisk is concerned.**
- **Type IV**: All devices using Boot Method C are Type IV. A/B Type IV ramdisk can boot into either Android or recovery based on info from bootloader; A-only Type IV ramdisk (or devices with dedicated `recovery` / `vendor_boot` recovery ramdisks) only boots into Android. On Android 13+ GKI devices, this ramdisk is located in `init_boot` (or `vendor_boot`) rather than `boot`.

Further details on Type III devices: Magisk is always installed in the ramdisk of a boot image. For all other device types, because their boot partitions (`boot`, `init_boot`, or `vendor_boot`) have ramdisk included, Magisk can be easily installed by patching the image through the Magisk app. However for Type III devices, they are **limited to installing Magisk into the `recovery` partition**. Magisk will not function when booted normally; instead Type III device owners have to always reboot to recovery to maintain Magisk access.

Some Type III devices' bootloader will still accept and provide `initramfs` that was manually added to the `boot` image to the kernel (e.g. some Xiaomi phones), but many devices don't (e.g. Samsung S10, Note 10). It solely depends on how the OEM implements its bootloader.
