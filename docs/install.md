# Installation

If you already have Magisk installed, it is **strongly recommended** to upgrade directly via the Magisk app using its "Direct Install" method. The following tutorial is only for the initial installation.

## Getting Started

Before you start:

- This tutorial assumes you understand how to use `adb` and `fastboot`
- Your device's bootloader has to be unlocked
- Make sure to remove any "boot image mods" such as other root solutions before installing Magisk. The easiest way is to restore the boot image with factory images, or reflash a *non-prerooted* custom ROM
- If you plan to also install custom kernels, install it after Magisk

---

Download and install the latest Magisk app. We use the app to gather some information about your device. In the home screen, you should see this:

<p align="center"><img src="images/device_info.png" width="500"/></p>

Pay special attention to the **Ramdisk** info. If the result is **Yes**, congratulations, your device is perfect for installing Magisk! You can install with [Custom Recovery](#custom-recovery)

However, if the result is **No** this means your device's boot partition does **NOT** include ramdisk. This means you will have to go through some extra steps to make Magisk work properly.

If you are using a Samsung device and the **SAR** result is **Yes**, please check [its own section](#samsung-system-as-root).

If you are using a Huawei device and the **SAR** result is **Yes**, please check [its own section](#huawei).

Otherwise, continue to [Patching Images](#patching-images).

> **If your device does not have boot ramdisk, read the [Magisk in Recovery](#magisk-in-recovery) section after installing. The information in that section is VERY important!**

(P.S.1 If you are interested in how Android boots and how it affects Magisk, check out [this document](boot.md))

## Patching Images

If your device has boot ramdisk, you need a copy of the `boot.img`<br>
If your device does **NOT** have boot ramdisk, you need a copy of the `recovery.img`

You should be able to extract the file you need from official firmware packages or your custom ROM zip (if using one). If you are still having trouble, go to [XDA-Developers](https://forum.xda-developers.com/) and look for resources, guides, discussions, or ask for help in your device's forum.

- Copy the boot/recovery image to your device
- Press the **Install** button in the Magisk card
- If you are patching a recovery image, make sure **"Recovery Mode"** is checked in options.<br>In most cases it should already be automatically checked.
- Choose **"Select and Patch a File"** in method, and select the stock boot/recovery image
- The Magisk app will patch the image to `[Internal Storage]/Download/magisk_patched_[random_strings].img`.
- Copy the patched image to your PC with ADB:<br>
`adb pull /sdcard/Download/magisk_patched_[random_strings].img`
- Flash the patched boot/recovery image to your device.<br>
For most devices, reboot into fastboot mode and flash with command:<br>
`fastboot flash boot /path/to/magisk_patched.img` or <br>
`fastboot flash recovery /path/to/magisk_patched.img` if flashing a recovery image
- Reboot and voila!

## Custom Recovery

In some custom recoveries, the installation may fail silently (it might look like success but in reality it bootloops). This is because the installer scripts cannot properly detect the correct device info or the recovery environment does not meet its expectation. If you face any issues, use the [Patch Image](#patching-images) method as it is guaranteed to work 100% of the time. Due to this reason, installing Magisk through custom recoveries on modern devices is no longer recommended. The custom recovery installation method exists purely for legacy support.

- Download the Magisk APK
- Rename the `.apk` file extension to `.zip`, for example: `Magisk-v22.0.apk` → `Magisk-v22.0.zip`. If you have trouble renaming the file extension (like on Windows), use a file manager on Android or the one included in TWRP to rename the file.
- Flash the zip just like any other ordinary flashable zip.
- Warning: `sepolicy.rule` file of modules may be stored in `cache` partition, do not clear it.
- Check whether the Magisk app is installed. If it isn't installed automatically, manually install the APK.

## Uninstallation

The easiest way to uninstall Magisk is directly through the Magisk app.<br>
If you insist on using custom recoveries, rename the Magisk APK to `uninstall.zip` and flash it like any other ordinary flashable zip.

## Magisk in Recovery

If your device does not have ramdisk in boot images, Magisk has no choice but to be installed in the recovery partition. For these devices, you will have to **reboot to recovery** every time you want Magisk.

When Magisk is installed in your recovery, **you CANNOT use custom recoveries to install/upgrade Magisk!** The only way to install/upgrade Magisk is through the Magisk app. It will be aware of your device state and install to the correct partition and reboot into the correct mode.

Since Magisk now hijacks the recovery of the device, there is a mechanism to let you *actually* boot into recovery mode when needed: it is determined by **how long you press the recovery key combo**.

Each device has its own key combo to boot into recovery, as an example for Galaxy S10 it is (Power + Bixby + Volume Up). A quick Google search should easily get you this info of your device. As soon as you press the combo and the device vibrates with a splash screen, release all buttons to boot into Magisk. If you decide to boot into actual recovery mode, continue to press volume up until you see the recovery screen.

**After installing Magisk in recovery (starting from power off):**

- **(Power up normally) → (System with NO Magisk)**
- **(Recovery Key Combo) → (Splash screen) → (Release all buttons) → (System with Magisk)**
- **(Recovery Key Combo) → (Splash screen) → (Keep pressing volume up) → (Recovery Mode)**

## Samsung (System-as-root)

**If your device is NOT launched with Android 9.0 or higher, you are reading the wrong section.**

### Before Installing Magisk

- Installing Magisk **WILL** trip KNOX
- Installing Magisk for the first time **REQUIRES** a full data wipe (this is **NOT** counting the data wipe when unlocking bootloader). Backup your data before continue.

### Unlocking Bootloader

Unlocking bootloader on modern Samsung devices have some caveats:

- Allow bootloader unlocking in **Developer options → OEM unlocking**
- Reboot to download mode: power off your device and press the download mode key combo for your device
- Long press volume up to unlock the bootloader. **This will wipe your data and automatically reboot.**

If you think the bootloader is fully unlocked, it is actually not! Samsung introduced `VaultKeeper`, meaning the bootloader will still reject any unofficial partitions before `VaultKeeper` explicitly allows it.

- Go through the initial setup. Skip through all the steps since data will be wiped again later when we are installing Magisk. **Connect the device to Internet during the setup.**
- Enable developer options, and **confirm that the OEM unlocking option exists and is grayed out.** This means the `VaultKeeper` service has unleashed the bootloader.
- Your bootloader now accepts unofficial images in download mode

### Instructions

- Use either [samfirm.js](https://github.com/jesec/samfirm.js), [Frija](https://forum.xda-developers.com/s10-plus/how-to/tool-frija-samsung-firmware-downloader-t3910594), or [Samloader](https://forum.xda-developers.com/s10-plus/how-to/tool-samloader-samfirm-frija-replacement-t4105929) to download the latest firmware zip of your device directly from Samsung servers.
-  Unzip the firmware and copy the `AP` tar file to your device. It is normally named as `AP_[device_model_sw_ver].tar.md5`
- Press the **Install** button in the Magisk card
- If your device does **NOT** have boot ramdisk, make sure **"Recovery Mode"** is checked in options.<br>In most cases it should already be automatically checked.
- Choose **"Select and Patch a File"** in method, and select the `AP` tar file
- The Magisk app will patch the whole firmware file to `[Internal Storage]/Download/magisk_patched_[random_strings].tar`
- Copy the patched tar file to your PC with ADB:<br>
`adb pull /sdcard/Download/magisk_patched_[random_strings].tar`<br>
**DO NOT USE MTP** as it is known to corrupt large files.
- Reboot to download mode. Open Odin on your PC, and flash `magisk_patched.tar` as `AP`, together with `BL`, `CP`, and `CSC` (**NOT** `HOME_CSC` because we want to **wipe data**) from the original firmware. This could take a while (>10 mins).
- After Odin is done, your device should reboot. You may continue with standard initial setup.<br>
If you are stuck in a bootloop, agree to do a factory reset if promted.
- If your device does **NOT** have boot ramdisk, reboot to recovery now to boot Android with Magisk (reason stated in [Magisk in Recovery](#magisk-in-recovery)).
- Although Magisk is installed, it still need some additional setup. Please connect to the Internet.
- Install the latest Magisk app and launch the app. It should show a dialog asking for additional setups. Let it do its job and the app will automatically reboot your device.
- Voila! Enjoy Magisk 😃

### Additional Notes

- **Never, ever** try to restore either `boot` or `recovery` partitions back to stock! You can easily brick your device by doing so, and the only way out is to do a full Odin restore with data wipe.
- To upgrade your device with a new firmware, **NEVER** directly use the stock `AP` tar file with reasons mentioned above. **Always** pre-patch `AP` in the Magisk app before flashing in Odin.
- Use `HOME_CSC` to preserve your data when doing a firmware upgrade in the future. Using `CSC` is only necessary for the initial Magisk installation.
- Never just flash only `AP`, or else Odin can shrink your `/data` filesystem. Flash full `AP` + `BL` + `CP` + `HOME_CSC` when upgrading.

## Huawei
Magisk no longer officially support modern Huawei devices as the bootloader on their devices are not unlockable, and more importantly they do not follow standard Android partitioning schemes. The following are just some general guidance.

Huawei devices using Kirin processors have a different partitioning method from most common devices. Magisk is usually installed to the `boot` partition of the device, however Huawei devices do not have this partition.

Generally, follow [Patching Images](#patching-images) with some differences from the original instructions:

- After downloading your firmware zip (you may find a lot in [Huawei Firmware Database](http://pro-teammt.ru/firmware-database/)), you have to extract images from `UPDATE.APP` in the zip with [Huawei Update Extractor](https://forum.xda-developers.com/showthread.php?t=2433454) (Windows only!)
- Regarding patching images:
  - If your device has boot ramdisk, patch `RAMDISK.img` instead of `boot.img`
  - If your device does **NOT** have boot ramdisk, patch `RECOVERY_RAMDIS.img` (this is not a typo) instead of `recovery.img`
- When flashing the image back with `fastboot`
  - If you patched `RAMDISK.img`, flash with command `fastboot flash ramdisk magisk_patched.img`
  - If you patched `RECOVERY_RAMDIS.img`, flash with command `fastboot flash recovery_ramdisk magisk_patched.img`
