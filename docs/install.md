# Installation
If you already have Magisk installed, it is **strongly recommended to upgrade directly via Magisk Manager**. The following tutorial is for first time users.

- If you are using a Huawei device running **EMUI 8 and higher**, please check its own section.
- If you are using a Samsung device that is **launched with Android 9.0** (new devices in 2019), please check its own section.

## Prerequisite
- If you plan to install custom kernels, flash the zip **AFTER** installing Magisk
- Make sure to remove any 'boot image mods' such as other root solutions. The easiest way is to restore the boot image from factory images, or reflash a *non-prerooted* custom ROM

## Custom Recovery
If your device have custom recovery support, the easiest way is to install it through custom recoveries, such as TWRP.

- Download the Magisk installer zip
- Reboot to custom recovery
- Flash the zip and reboot
- Check whether Magisk Manager is installed. If for some reason it isn't installed automatically, manually install the APK

## Boot Image Patching
This is the "cool" way to install Magisk on your device. Either your device does not have proper custom recoveries, your device is using the A/B partition scheme and you don't want to mix recovery and boot images together, or you have other concerns (e.g. [OTA Installation](tutorials.md#ota-installation)), you should use this method instead.

In order to use this method, you are required to obtain a copy of the stock boot image, which can be found by extracting OEM provided factory images or extracting from OTA update zips. If you are unable to obtain one yourself, someone on the Internet might share it somewhere. The following instructions will guide you through the process after you have the copy of boot image.

- Copy the boot image to your device
- Download and install the latest Magisk Manager
- Press **Install → Install → Select and Patch a File**, and select your stock boot image file
- Magisk Manager will install Magisk to your boot image, and store it in `[Internal Storage]/Download/magisk_patched.img`
- Copy the patched boot image from your device to your PC. If you can't find it via MTP, you can pull the file with ADB: <br>
`adb pull /sdcard/Download/magisk_patched.img`
- Flash the patched boot image to your device and reboot. Here is the command if using fastboot on most devices: <br>
`fastboot flash boot /path/to/magisk_patched.img`

## Magisk in Recovery
Due to the fact that some devices no longer uses ramdisk in boot images, Magisk has no choice but to be installed in the recovery partition. For these devices, you will have to **boot to recovery every time** if you want Magisk. Since both Magisk and recovery lives in the same partition, what you actually end up getting when you choose to boot to recovery will be determined by **how long you press volume up**.

Each OEM has its own key combo to boot into recovery. For example on Samsung it is **(Power + Bixby + Volume Up)**, and for Huawei it is **(Power + Volume Up)**. As soon as you press the combo and the device vibrates with a splash screen, the bootloader has already chosen which mode it is booting, either it be `boot`, `recovery`, or some OEM specific modes like `download`, `fastboot`, or `erecovery`. After the splash screen, release all buttons to boot into Magisk, since by default `recovery` mode will boot to the system with Magisk enabled. If you decide to boot to actual recovery, continue to press volume up until you see the recovery screen.

**In summary, after installing Magisk:**
- **(Powering up normally) → (System with NO Magisk)**
- **(OEM Recovery Key Combo) → (Splash screen) → (Release all buttons) → (System with Magisk)**
- **(OEM Recovery Key Combo) → (Splash screen) → (Keep pressing volume up) → (Actual recovery)**

## Samsung (System-as-root)
**If your device is NOT launched with Android 9.0 or higher (released after 2019), follow the normal tutorial**

### Before Installing Magisk
- Installing Magisk **WILL** trip KNOX
- Installing Magisk for the first time **REQUIRES** a full data wipe, backup before continue
- You have to have your bootloader unlocked before following the instructions
- Magisk will be installed to the **recovery** partition of your device. **Please read the Magisk in Recovery section before following the instructions below!**
- After installing Magisk, you can directly upgrade Magisk within Magisk Manager without an issue. **Flashing in custom recovery is not supported for now.**

### Unlocking Bootloader
Normally I won't provide instructions for this, but since things had changed drastically from previous Samsung devices, and there are some details that many might not know, I figure this would be helpful.
- Allow bootloader unlocking in Developer options → OEM unlocking
- Power off your device. Press *Bixby + Volume Down* and plug in your device to a PC to boot into download mode
- Long press volume up to unlock the bootloader. **This will wipe your data and automatically reboot.**

Just when you think the bootloader is unlocked, surprise surprise, it is *actually* not! Samsung introduced `VaultKeeper` in the system, meaning the bootloader will reject any unofficial partitions before `VaultKeeper` explicitly allows it.

- Go through the initial setup. Skip through all the steps since data will be wiped again later when we are installing Magisk. **Connect the device to internet in the setup!**
- Enable developer options, and **confirm that the OEM unlocking option exists and grayed out!** The `VaultKeeper` service will unleash the bootloader after it confirms that the user has the OEM unlocking option enabled. This step is to simply make sure the service gets the correct info, and also double check that our device is in a correct state
- Your bootloader now accepts unofficial images in download mode.

### Instructions
1. Download the firmware for your device.
2. Unzip the firmware and copy the **AP** tar file to your device. It is normally named as `AP_[device_model_sw_ver].tar.md5`
3. Install the latest Magisk Manager
4. In Magisk Manager: **Install → Install → Select and Patch a File** and select the AP tar file.
5. Magisk Manager will patch the whole firmware file and store the output to
`[Internal Storage]/Download/magisk_patched.tar`
6. Copy the tar file to your PC (using `adb`: some people report corruption using MTP), and boot your device to download mode.
7. Flash `magisk_patched.tar` as AP in ODIN, together with the BL, CP and HOME_CSC files. Never flash only an AP file, as Odin can shrink your `/data` file-system if you do.<br> **Important: Uncheck "Auto Reboot" in Options!**
8. Magisk is now successfully flashed to your device! But there are still several steps before you can properly use the device.
9. We now want to boot into the stock recovery to factory reset our device. <br>
**Full data wipe is mandatory! Do not skip this step.** <br>
Press *Power + Volume Down* to exit download mode. As soon as the screen turns off, immediately press *Power + Bixby + Volume Up* to boot to recovery partition. Just as mentioned in the previous section, since we want to boot into stock recovery, **continue pressing the volume up button until you see the stock recovery screen**.
10. In the stock recovery menu, use volume buttons to navigate through menus, and the power button to select the option. Select *Wipe data/factory reset* to wipe the data of the device.
11. This time, we can finally boot to the system with Magisk. Select *Reboot system now*, and immediately press *Power + Bixby + Volume Up*. After seeing the bootloader warning screen, release all buttons so it can boot to the system.
12. The device will automatically reboot for the first time it boots. This is completely normal and done by design.
13. After the device is booted up, do the usual initial setup. **The following steps will need internet connection.**
14. You shall see Magisk Manager in your app drawer; if not, manually install the APK you downloaded in step 3 and continue to the next step. The app would be a stub and it shall automatically upgrade to the full Magisk Manager when you open it.
15. Magisk Manager will ask to do additional setups. Let it do its job and the app will automatically reboot your device.
16. Voila! Enjoy Magisk :)

### Additional Info
- Magisk actually patches 3 partitions on your device:
    - `vbmeta`: replace with empty vbmeta image to disable partition verification
    - `boot`: remove the signature of the image to prevent soft bricks
    - `recovery`: this is where Magisk is actually installed
- **Never, ever** try to restore either of the 3 images mentioned back to stock! You can easily brick your device by doing so, and the only way out is to do full ODIN restore following with factory reset. Just don't do it.
- If you want to upgrade your device, **never** flash the stock **AP** tar file with reasons mentioned above. **Always** pre-patch the firmware before flashing in ODIN.
-  If you don't need to patch the full firmware, you can manually create a tar file with **at least** `vbmeta.img`, `boot.img`, and `recovery.img` to let Magisk Manager patch your images in the proper way.

## Huawei
Huawei devices using Kirin processors have a different partitioning method from most common devices. Magisk is usually installed to the `boot` partition of the device, however Huawei devices does not have this partition. Depending on what EMUI version your device is running, the instructions will be slightly different.

### Obtain Stock Images
Huawei does not release official factory images, however most firmware zips can be downloaded from the [Huawei Firmware Database](http://pro-teammt.ru/firmware-database/). To extract images from `UPDATE.APP` in the zip, you have to use [Huawei Update Extractor](https://forum.xda-developers.com/showthread.php?t=2433454) (Windows only!)

### EMUI 8
For EMUI 8 devices, your device have a partition named `ramdisk`, which is where Magisk is going to be installed.

- If you plan to use custom recoveries, simply follow the instructions for custom recovery and you're all set.
- If you plan not to use custom recoveries, you will have to extract `RAMDISK.img` from your firmware. Follow the instructions for boot image patching above, but use the `RAMDISK.img` file instead of a boot image.
- To flash the patched image to your device, here is the fastboot command: <br>
`fastboot flash ramdisk /path/to/magisk_patched.img` <br>
Be aware you are flashing to `ramdisk`, not `boot`!

### EMUI 9 or Higher
For EMUI 9+ devices, the `ramdisk` partition no longer exists. As a workaround, Magisk will be installed to the `recovery_ramdisk` partition. **Please read the Magisk in Recovery section before following the instructions below!**

*Note: As I tested on my Honor View 10, Huawei's kernel does not seem to be able to capture key button press events in early boot, so long pressing Volume Up does **NOT** boot to recovery on my device. Your experience may vary.*

- If you plan to use custom recoveries, simply follow the instructions for custom recovery and you're all set. <br>
**Warning: Magisk will overwrite the custom recovery.**
- If you plan not to use custom recoveries, you will have to extract `RECOVERY_RAMDIS.img` from your firmware. Follow the instructions for boot image patching above, but use the `RECOVERY_RAMDIS.img` file instead of a boot image.
- To flash the patched image to your device, here is the fastboot command: <br>
`fastboot flash recovery_ramdisk /path/to/magisk_patched.img` <br>
Be aware you are flashing to `recovery_ramdisk`, not `boot`!
