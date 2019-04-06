# Installation
If you already have Magisk installed, it is **strongly recommended to upgrade directly via Magisk Manager**.
The following tutorial is for first time users. For Huawei users, please check the specific section for more information.

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

In order to use this method, you are required to obtain a copy of the stock boot image, which can be found by extracting OEM provided factory images, or extracted from OTA update zips. If you are unable to obtain one yourself, someone on the Internet might share it somewhere. The following instructions will guide you through the process after you have the copy of boot image.

- Copy the boot image to your device
- Download and install the latest Magisk Manager
- If you're planning to flash the patched boot image through ODIN (Samsung only), go to **Settings > Update Settings > Patched Boot Output Format**, and select *.img.tar*, or else leave it as the default (*.img*)
- Press **Install > Install > Patch Boot Image File**, and select your stock boot image file
- Magisk Manager will install Magisk to your boot image, and store it in
`[Internal Storage]/Download/patched_boot.img[.tar]`
- Copy the patched boot image from your device to your PC. If you can't find it via MTP, you can pull the file with ADB:
`adb pull /sdcard/Download/patched_boot.img[.tar]`
- Flash the patched boot image to your device and reboot. Here is the command if using fastboot on most devices:
`fastboot flash boot /path/to/patched_boot.img`

## Huawei
Huawei devices using Kirin processors have a different partitioning method from most common devices. Magisk is usually installed to the `boot` partition of the device, however Huawei devices does not have this partition. Depending on what EMUI version your device is running the instructions are slightly different. Even if you have switched to a custom ROM, you shall still know which version of EMUI you are running before switching.

### Obtain Stock Images
Huawei does not release official factory images, however most firmware zips can be downloaded from the [Huawei Firmware Database](http://pro-teammt.ru/firmware-database/). To extract the images from `UPDATE.APP` in the zip, you have to use [Huawei Update Extractor](https://forum.xda-developers.com/showthread.php?t=2433454) (Windows only!)

### EMUI 8
For EMUI 8 devices, your device have a partition named `ramdisk`, which will be where Magisk is going to be installed.

- If you plan to use custom recoveries, simply follow the instructions for custom recovery above.
Note that to install TWRP, you will first download the TWRP recovery image, and use
`fastboot flash recovery_ramdisk /path/to/twrp.img` to install the custom recovery.
- If you plan not to use custom recoveries, you will have to extract `RAMDISK.img` from your firmware. Follow the instructions for boot image patching above, but use the `RAMDISK.img` file instead of a boot image. To install the patched image back to your device, here is the fastboot command:
`fastboot flash ramdisk /path/to/patched_boot.img`.
Be aware you are flashing to `ramdisk`, not `boot`!

### EMUI 9
For EMUI 9 devices, the `ramdisk` partition no longer exists. As a workaround, Magisk will be installed to the `recovery_ramdisk` partition. **This means that you HAVE TO boot to recovery every time you reboot.** To boot to recovery, press **Power + Volume Up** when booting your device.

- If you plan to use custom recoveries, simply follow the instructions for custom recovery above.
Note that to install TWRP, you will first download the TWRP recovery image, and use
`fastboot flash recovery_ramdisk /path/to/twrp.img` to install the custom recovery.
**Magisk will overwrite the custom recovery.**
- If you plan not to use custom recoveries, you will have to extract `RECOVERY_RAMDIS.img` from your firmware. Follow the instructions for boot image patching above, but use the `RECOVERY_RAMDIS.img` file instead of a boot image. To install the patched image back to your device, here is the fastboot command:
`fastboot flash recovery_ramdisk /path/to/patched_boot.img`.
Be aware you are flashing to `recovery_ramdisk`, not `boot` nor `ramdisk`!
- You can still install custom recoveries to the `erecovery_ramdisk` partition. Boot to erecovery mode to boot into custom recovery in this case
