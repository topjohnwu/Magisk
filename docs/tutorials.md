# User Tutorials

## OTA Installation
Magisk does modifications systemless-ly, which means applying official OTAs is much simpler. Here I provide a few tutorials for several different kind of devices to apply OTAs and preserve Magisk after the installation if possible.

**This tutorial is only for Magisk v17.0+**

**NOTE: In order to apply OTAs, you HAVE to make sure you haven't modified `/system` (and `/vendor` if available) in any way. Even remounting the partition to rw will tamper block verification!!**

### Prerequisites
1. Please disable *Automatic system updates* in developer options, so it won't install OTAs without your acknowledgement.  
    <img src="images/disable_auto_ota.png" width="250">
1. When an OTA is available, first go to (Magisk Manager → Uninstall → Restore Images). **Do not reboot or you will have Magisk uninstalled.** This will restore your boot (and dtbo if applicable) back to 100% untouched stock images in order to pass pre-OTA block verifications. **This step is required before doing any of the following steps written below!**  
    <img src="images/restore_img.png" width="250">

### Devices with A/B Partitions

Due to the fact that these devices have two separate partitions, it is possible to have the OTA installed to the inactive slot and have Magisk Manager to install Magisk onto the updated partition. The out-of-the-box OTA installation works seamlessly and Magisk can be preserved after the installation.

1. After restoring stock boot image, apply OTAs as you normally would (Settings → System → System Update).
1. Wait for the installation to be fully done (both step 1 and step 2 of the OTA), **do not press the restart button!!** Instead, go to (Magisk Manager → Install → Install to Inactive Slot) and install Magisk to the slot that the OTA engine just updated.  
    <img src="images/ota_done.png" width="250"> <img src="images/install_inactive_slot.png" width="250">
1. After installation is done, press the reboot button in Magisk Manager. Under-the-hood Magisk Manager forces your device to switch to the updated slot, bypassing any possible post-OTA verifications.  
    <img src="images/manager_reboot.png" width="250">
1. After the reboot, your device should be fully updated, and most importantly, Magisk is still installed to the updated system!

### Devices with FlashFire Support
(If you are using a device with A/B partitions, I **strongly** recommend you to use the method stated above since it uses the stock OTA installation mechanism and will always work under any circumstances)

The [FlashFire](https://play.google.com/store/apps/details?id=eu.chainfire.flash) app developed by Chainfire is a great app to apply OTAs and preserve root at the same time. However, there is a very high chance that it does not support your device/system combination, and unfortunately the app is no longer maintained by Chainfire so no additonal compatibility would be added in the future.

1. After restoring the stock boot image, download the OTA (Settings → System → System Updates), **do not press reboot to install.**
1. Open FlashFire, it should detect your OTA zip. Select OK in the popup dialog to let it do its setup.
1. Please use the options shown in the screenshot below. The key point is to disable EverRoot (or it will install SuperSU), and add a new action to flash Magisk zip **after** the OTA update.zip (the update.zip should be auto generated in the previous step).  
    <img src="images/flashfire.png" width="250">
1. Press the big **Flash** button, after a few minutes it should reboot and updated with Magisk installed.

### Legacy "Non A/B" Devices - General Case
Unfortunately, there are no real good ways to apply OTAs on these devices. The following tutorial will not preserve Magisk - you will have to manually re-root your device after the upgrade, and this will require PC access. This is a general "best practice".

1. To properly install OTAs, you must have your stock recovery installed on your device. If you have custom recovery installed, you can restore it from your previous backup, or dumps found online, or factory images provided by OEMs.  
If you decide to start by installing Magisk without touching your recovery partition, you have a few choices, either way you will end up with a Magisk rooted device, but recovery remains stock untouched:
    - If supported, use `fastboot boot <recovery_img>` to boot the custom recovery and install Magisk. 
    - If you have a copy of your stock boot image dump, install Magisk by patching boot image via Magisk Manager, and manually flash it through download mode / fastboot mode / Odin.
1. Once your device has stock recovery and stock boot image restored, download the OTA. Optionally, once you have downloaded the OTA update zip, you can find a way to copy the zip out, since you are still rooted. Personally, I extract the stock boot image and recovery image from the OTA zip for future usage (to patch via Magisk Manager or restore stock recovery etc.)
1. Apply and reboot your device. This will use the official stock OTA installation mechanism of your device to upgrade your system.
1. Once it's done you will be left with an upgraded, 100% stock, un-rooted device. You will have to manually flash Magisk back. Consider using the methods stated in step 1. to flash Magisk without touching the recovery partition if you want to receive stock OTAs frequently.
