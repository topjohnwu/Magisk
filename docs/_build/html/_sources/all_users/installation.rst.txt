Installation Instructions
===========================

.. toctree::
    :maxdepth: 4

    installation.rst

If you already have Magisk installed, it is recommended that you use the direct upgrade option in the Magisk App

Before you Start
------------------

* This tutorial assumes you already know how to use adb and fastboot
* Your device's bootloader is already unlocked
* Make sure to remove any "boot image mods" such as other root solutions before installing Magisk. The easiest
  way to be sure is to restore the stock boot image or flash a non-prerooted custom ROM
* If you want to use a custom kernel this should be installed after Magisk

Getting Started
----------------

Download and install the latest Magisk App. This app gathers information about
your device that we will need to use to install Magisk. The home screen should look like this:

.. image:: /Images/device_info.png

On this screen there are certain areas that you need to pay attention to:

Ramdisk: if the result is yes then everything is fine and you can continue. However, if the result is
no then your device does not have a Ramdisk and you need to read the :ref:`magiskrecovery` section.

SAR - there are two scenarios when you need to pay attention to this:

* If the result is yes and you have a samsung device proceed to :ref:`samsungsar` 
* If the result is yes and you have a huawei device then proceed to :ref:`huaweisar` 


.. _magiskrecovery:

Magisk in Recovery
-------------------

If your device does not have ramdisk in boot images, Magisk has no choice but to be installed in the recovery partition. For these devices, you will have to reboot to recovery every time you want Magisk.

When Magisk is installed in your recovery, you CANNOT use custom recoveries to install/upgrade Magisk! The only way to install/upgrade Magisk is through the Magisk app. It will be aware of your device state and install to the correct partition and reboot into the correct mode.

Since Magisk now hijacks the recovery of the device, there is a mechanism to let you actually boot into recovery mode when needed: it is determined by how long you press the recovery key combo.

Each device has its own key combo to boot into recovery, as an example for Galaxy S10 it is (Power + Bixby + Volume Up). A quick Google search should easily get you this info of your device. As soon as you press the combo and the device vibrates with a splash screen, release all buttons to boot into Magisk. If you decide to boot into actual recovery mode, continue to press volume up until you see the recovery screen.

After installing Magisk in recovery (starting from power off):

(Power up normally) → (System with NO Magisk)

(Recovery Key Combo) → (Splash screen) → (Release all buttons) → (System with Magisk)

(Recovery Key Combo) → (Splash screen) → (Keep pressing volume up) → (Recovery Mode)


.. _samsungsar:

Samsung Devices (System as root)
---------------------------------

If your device is NOT launched with Android 9.0 or higher, you are reading the wrong section.

**Before Installing Magisk**

* Installing Magisk WILL trip KNOX
* Installing Magisk for the first time REQUIRES a full data wipe (this is NOT counting the data wipe when unlocking bootloader). Backup your data before continue.

**Unlocking Bootloader**

Unlocking bootloader on modern Samsung devices have some caveats:

* Allow bootloader unlocking in Developer options → OEM unlocking
* Reboot to download mode: power off your device and press the download mode key combo for your device
* Long press volume up to unlock the bootloader. This will wipe your data and automatically reboot.

If you think the bootloader is fully unlocked, it is actually not! Samsung introduced VaultKeeper, meaning the bootloader will still reject any unofficial partitions before VaultKeeper explicitly allows it.

Go through the initial setup. Skip through all the steps since data will be wiped again later when we are installing Magisk. Connect the device to Internet during the setup.
Enable developer options, and confirm that the OEM unlocking option exists and is grayed out. This means the VaultKeeper service has unleashed the bootloader.
Your bootloader now accepts unofficial images in download mode

Instructions
^^^^^^^^^^^^^

Use either samfirm.js, Frija, or Samloader to download the latest firmware zip of your device directly from Samsung servers.
Unzip the firmware and copy the AP tar file to your device. It is normally named as AP_[device_model_sw_ver].tar.md5

Press the Install button in the Magisk card

If your device does NOT have boot ramdisk, make sure “Recovery Mode” is checked in options.

In most cases it should already be automatically checked.

Choose “Select and Patch a File” in method, and select the AP tar file

The Magisk app will patch the whole firmware file to [Internal Storage]/Download/magisk_patched_[random_strings].tar

Copy the patched tar file to your PC with ADB: ::

    adb pull /sdcard/Download/magisk_patched_[random_strings].tar

DO NOT USE MTP as it is known to corrupt large files.


Reboot to download mode. Open Odin on your PC, and flash magisk_patched.tar as AP, together with BL, CP, and CSC (NOT HOME_CSC because we want to wipe data) from the original firmware. This could take a while (>10 mins).

After Odin is done, your device should reboot. You may continue with standard initial setup.

If you are stuck in a bootloop, agree to do a factory reset if prompted.

If your device does NOT have boot ramdisk, reboot to recovery now to boot Android with Magisk (reason stated in Magisk in Recovery).
Although Magisk is installed, it still need some additional setup. Please connect to the Internet.
Install the latest Magisk app and launch the app. It should show a dialog asking for additional setups. Let it do its job and the app will automatically reboot your device.

Voila! Enjoy Magisk 😃

Additional Notes
^^^^^^^^^^^^^^^^

Never, ever try to restore either boot or recovery partitions back to stock! You can easily brick your device by doing so, and the only way out is to do a full Odin restore with data wipe.
To upgrade your device with a new firmware, NEVER directly use the stock AP tar file with reasons mentioned above. Always pre-patch AP in the Magisk app before flashing in Odin.
Use HOME_CSC to preserve your data when doing a firmware upgrade in the future. Using CSC is only necessary for the initial Magisk installation.
Never just flash only AP, or else Odin can shrink your /data filesystem. Flash full AP + BL + CP + HOME_CSC when upgrading


.. _huaweisar:

Huawei Devices (System as Root)
--------------------------------

Magisk no longer officially support modern Huawei devices as the bootloader on their devices are not unlockable, and more importantly they do not follow standard Android partitioning schemes. The following are just some general guidance.

Huawei devices using Kirin processors have a different partitioning method from most common devices. Magisk is usually installed to the boot partition of the device, however Huawei devices do not have this partition.

Generally, follow Patching Images with some differences from the original instructions:

After downloading your firmware zip (you may find a lot in Huawei Firmware Database), you have to extract images from UPDATE.APP in the zip with Huawei Update Extractor (Windows only!)
Regarding patching images:


* If your device has boot ramdisk, patch RAMDISK.img instead of boot.img
* If your device does NOT have boot ramdisk, patch RECOVERY_RAMDIS.img (this is not a typo) instead of recovery.img 

When flashing the image back with fastboot
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

* If you patched RAMDISK.img, flash with command fastboot flash ramdisk magisk_patched.img
* If you patched RECOVERY_RAMDIS.img, flash with command fastboot flash recovery_ramdisk magisk_patched.img
