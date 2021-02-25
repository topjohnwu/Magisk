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

Pay special attention to the **Ramdisk** info. If the result is **Yes**, congratulations, your device is perfect for installing Magisk! However, if the result is no this means your device's boot partition does include ramdisk. This means you will have to go through some extra steps to make Magisk work properly.

> If your device does not have boot ramdisk, read the (#Magisk in Recovery) (#magisk-in-recovery) section after installing. The information in that section is VERY important!**

If you are using a Samsung device and the **SAR** result is **Yes**, please check its own section(#samsung-system-as-root).

If you are using a Huawei device and the **SAR** result is **Yes**, please check [its own section](#huawei).

Otherwise, continue to [Patching Images](#patching-images).

(P.S.1 If your device has boot ramdisk, you can also install with [Custom Recovery](#custom-recovery))<br>
(P.S.2 If you are interested in how Android boots and how it affects Magisk, check out [this document](boot.md))

## Patching Images

If your device has boot ramdisk, you need a copy of the `boot.img`<br>
If your device does **NOT** have boot ramdisk, you need a copy of the `recovery.img`

You should be able to extract the file you need from official firmware packages or your custom ROM zip (if using one). If you are still having trouble, go to (#XDA-Developers)(#https://forum.xda-developers.com/) and look for resources, guides, discussions, or ask for help in your device's forum.

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
- Voila! Enjoy Magisk 

### Additional Notes

- **Never, ever** try to restore either `boot` or `recovery` partitions back to stock! You can easily brick your device by doing so, and the only way out is to do a full Odin restore with data wipe.
- To upgrade your device with a new firmware, **NEVER** directly use the stock `AP` tar file with reasons mentioned above. **Always** pre-patch `AP` in the Magisk app before flashing in Odin.
- Use `HOME_CSC` to preserve your data when doing a firmware upgrade in the future. Using `CSC` is only necessary for the initial Magisk installation.
- Never just flash only `AP`, or else Odin can shrink your `/data` filesystem. Flash full `AP` + `BL` + `CP` + `HOME_CSC` when upgrading.

## Huawei
Magisk no longer officially support modern Huawei devices as the bootloader on their devices are not unlockable, and more importantly they do not follow standard Android partitioning schemes. The following are just some general guidance.

<?xml version="1.0" encoding="UTF-8"?>
<feed xml:lang="en-US" xmlns="http://www.w3.org/2005/Atom">
  <id>tag:www.githubstatus.com,2005:/history</id>
  <link rel="alternate" type="text/html" href="https://www.githubstatus.com"/>
  <link rel="self" type="application/atom+xml" href="https://www.githubstatus.com/history.atom"/>
  <title>GitHub Status - Incident History</title>
  <updated>2021-02-24T20:24:12Z</updated>
  <author>
    <name>GitHub</name>
  </author>
  <entry>
    <id>tag:www.githubstatus.com,2005:Incident/6390341</id>
    <published>2021-02-24T20:01:07Z</published>
    <updated>2021-02-24T20:01:07Z</updated>
    <link rel="alternate" type="text/html" href="https://www.githubstatus.com/incidents/cbphcjkww9r2"/>
    <title>Incident on 2021-02-24 19:25 UTC</title>
    <content type="html">&lt;p&gt;&lt;small&gt;Feb &lt;var data-var='date'&gt;24&lt;/var&gt;, &lt;var data-var='time'&gt;20:01&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Resolved&lt;/strong&gt; - This incident has been resolved.&lt;/p&gt;&lt;p&gt;&lt;small&gt;Feb &lt;var data-var='date'&gt;24&lt;/var&gt;, &lt;var data-var='time'&gt;19:25&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Investigating&lt;/strong&gt; - We are investigating reports of degraded performance for Git Operations.&lt;/p&gt;</content>
  </entry>
  <entry>
    <id>tag:www.githubstatus.com,2005:Incident/6368001</id>
    <published>2021-02-23T17:50:03Z</published>
    <updated>2021-02-23T17:50:03Z</updated>
    <link rel="alternate" type="text/html" href="https://www.githubstatus.com/incidents/7ct9z7mv9kw1"/>
    <title>Incident on 2021-02-23 16:37 UTC</title>
    <content type="html">&lt;p&gt;&lt;small&gt;Feb &lt;var data-var='date'&gt;23&lt;/var&gt;, &lt;var data-var='time'&gt;17:50&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Resolved&lt;/strong&gt; - This incident has been resolved.&lt;/p&gt;&lt;p&gt;&lt;small&gt;Feb &lt;var data-var='date'&gt;23&lt;/var&gt;, &lt;var data-var='time'&gt;16:37&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Investigating&lt;/strong&gt; - We are investigating reports of degraded performance for GitHub Actions.&lt;/p&gt;</content>
  </entry>
  <entry>
    <id>tag:www.githubstatus.com,2005:Incident/6306687</id>
    <published>2021-02-18T01:35:07Z</published>
    <updated>2021-02-18T01:35:07Z</updated>
    <link rel="alternate" type="text/html" href="https://www.githubstatus.com/incidents/bnd3tpf66m0b"/>
    <title>Incident on 2021-02-17 23:43 UTC</title>
    <content type="html">&lt;p&gt;&lt;small&gt;Feb &lt;var data-var='date'&gt;18&lt;/var&gt;, &lt;var data-var='time'&gt;01:35&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Resolved&lt;/strong&gt; - This incident has been resolved.&lt;/p&gt;&lt;p&gt;&lt;small&gt;Feb &lt;var data-var='date'&gt;17&lt;/var&gt;, &lt;var data-var='time'&gt;23:43&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Investigating&lt;/strong&gt; - We are investigating reports of degraded performance for GitHub Actions.&lt;/p&gt;</content>
  </entry>
  <entry>
    <id>tag:www.githubstatus.com,2005:Incident/6296822</id>
    <published>2021-02-17T08:29:14Z</published>
    <updated>2021-02-17T08:29:14Z</updated>
    <link rel="alternate" type="text/html" href="https://www.githubstatus.com/incidents/yxpgm9766lp0"/>
    <title>Incident on 2021-02-17 08:09 UTC</title>
    <content type="html">&lt;p&gt;&lt;small&gt;Feb &lt;var data-var='date'&gt;17&lt;/var&gt;, &lt;var data-var='time'&gt;08:29&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Resolved&lt;/strong&gt; - This incident has been resolved.&lt;/p&gt;&lt;p&gt;&lt;small&gt;Feb &lt;var data-var='date'&gt;17&lt;/var&gt;, &lt;var data-var='time'&gt;08:09&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Investigating&lt;/strong&gt; - We are investigating reports of degraded availability for GitHub Docs.&lt;/p&gt;</content>
  </entry>
  <entry>
    <id>tag:www.githubstatus.com,2005:Incident/6248518</id>
    <published>2021-02-12T01:36:23Z</published>
    <updated>2021-02-12T01:36:23Z</updated>
    <link rel="alternate" type="text/html" href="https://www.githubstatus.com/incidents/tqzj0mwds128"/>
    <title>Incident on 2021-02-11 22:15 UTC</title>
    <content type="html">&lt;p&gt;&lt;small&gt;Feb &lt;var data-var='date'&gt;12&lt;/var&gt;, &lt;var data-var='time'&gt;01:36&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Resolved&lt;/strong&gt; - This incident has been resolved.&lt;/p&gt;&lt;p&gt;&lt;small&gt;Feb &lt;var data-var='date'&gt;11&lt;/var&gt;, &lt;var data-var='time'&gt;22:15&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Investigating&lt;/strong&gt; - We are investigating reports of degraded performance for GitHub Actions.&lt;/p&gt;</content>
  </entry>
  <entry>
    <id>tag:www.githubstatus.com,2005:Incident/6239117</id>
    <published>2021-02-10T20:37:46Z</published>
    <updated>2021-02-10T20:37:46Z</updated>
    <link rel="alternate" type="text/html" href="https://www.githubstatus.com/incidents/mbk18c24mtlz"/>
    <title>Incident on 2021-02-10 20:34 UTC</title>
    <content type="html">&lt;p&gt;&lt;small&gt;Feb &lt;var data-var='date'&gt;10&lt;/var&gt;, &lt;var data-var='time'&gt;20:37&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Resolved&lt;/strong&gt; - This incident has been resolved.&lt;/p&gt;&lt;p&gt;&lt;small&gt;Feb &lt;var data-var='date'&gt;10&lt;/var&gt;, &lt;var data-var='time'&gt;20:34&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Investigating&lt;/strong&gt; - We are investigating reports of degraded availability for GitHub Docs.&lt;/p&gt;</content>
  </entry>
  <entry>
    <id>tag:www.githubstatus.com,2005:Incident/6228940</id>
    <published>2021-02-09T16:57:17Z</published>
    <updated>2021-02-09T16:57:17Z</updated>
    <link rel="alternate" type="text/html" href="https://www.githubstatus.com/incidents/5qdkkyg958vy"/>
    <title>Incident on 2021-02-09 16:19 UTC</title>
    <content type="html">&lt;p&gt;&lt;small&gt;Feb &lt;var data-var='date'&gt; 9&lt;/var&gt;, &lt;var data-var='time'&gt;16:57&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Resolved&lt;/strong&gt; - This incident has been resolved.&lt;/p&gt;&lt;p&gt;&lt;small&gt;Feb &lt;var data-var='date'&gt; 9&lt;/var&gt;, &lt;var data-var='time'&gt;16:44&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Update&lt;/strong&gt; - Webhooks is operating normally.&lt;/p&gt;&lt;p&gt;&lt;small&gt;Feb &lt;var data-var='date'&gt; 9&lt;/var&gt;, &lt;var data-var='time'&gt;16:25&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Update&lt;/strong&gt; - GitHub Actions is still  experiencing degraded performance. We are still investigating and will provide an update when we have one.&lt;/p&gt;&lt;p&gt;&lt;small&gt;Feb &lt;var data-var='date'&gt; 9&lt;/var&gt;, &lt;var data-var='time'&gt;16:21&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Update&lt;/strong&gt; - Webhooks is experiencing degraded performance. We are continuing to investigate.&lt;/p&gt;&lt;p&gt;&lt;small&gt;Feb &lt;var data-var='date'&gt; 9&lt;/var&gt;, &lt;var data-var='time'&gt;16:19&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Investigating&lt;/strong&gt; - We are investigating reports of degraded performance for GitHub Actions.&lt;/p&gt;</content>
  </entry>
  <entry>
    <id>tag:www.githubstatus.com,2005:Incident/6220042</id>
    <published>2021-02-08T16:28:44Z</published>
    <updated>2021-02-08T16:28:44Z</updated>
    <link rel="alternate" type="text/html" href="https://www.githubstatus.com/incidents/th4mgzt9y25t"/>
    <title>Incident on 2021-02-08 15:59 UTC</title>
    <content type="html">&lt;p&gt;&lt;small&gt;Feb &lt;var data-var='date'&gt; 8&lt;/var&gt;, &lt;var data-var='time'&gt;16:28&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Resolved&lt;/strong&gt; - This incident has been resolved.&lt;/p&gt;&lt;p&gt;&lt;small&gt;Feb &lt;var data-var='date'&gt; 8&lt;/var&gt;, &lt;var data-var='time'&gt;16:23&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Update&lt;/strong&gt; - Pull Requests is operating normally.&lt;/p&gt;&lt;p&gt;&lt;small&gt;Feb &lt;var data-var='date'&gt; 8&lt;/var&gt;, &lt;var data-var='time'&gt;16:20&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Update&lt;/strong&gt; - Webhooks is operating normally.&lt;/p&gt;&lt;p&gt;&lt;small&gt;Feb &lt;var data-var='date'&gt; 8&lt;/var&gt;, &lt;var data-var='time'&gt;16:20&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Update&lt;/strong&gt; - GitHub Packages is operating normally.&lt;/p&gt;&lt;p&gt;&lt;small&gt;Feb &lt;var data-var='date'&gt; 8&lt;/var&gt;, &lt;var data-var='time'&gt;16:20&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Update&lt;/strong&gt; - Issues is operating normally.&lt;/p&gt;&lt;p&gt;&lt;small&gt;Feb &lt;var data-var='date'&gt; 8&lt;/var&gt;, &lt;var data-var='time'&gt;16:09&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Update&lt;/strong&gt; - GitHub Actions is experiencing degraded performance. We are still investigating and will provide an update when we have one.&lt;/p&gt;&lt;p&gt;&lt;small&gt;Feb &lt;var data-var='date'&gt; 8&lt;/var&gt;, &lt;var data-var='time'&gt;16:09&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Update&lt;/strong&gt; - Pull Requests is experiencing degraded performance. We are continuing to investigate.&lt;/p&gt;&lt;p&gt;&lt;small&gt;Feb &lt;var data-var='date'&gt; 8&lt;/var&gt;, &lt;var data-var='time'&gt;16:05&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Update&lt;/strong&gt; - GitHub Packages is experiencing degraded performance. We are still investigating and will provide an update when we have one.&lt;/p&gt;&lt;p&gt;&lt;small&gt;Feb &lt;var data-var='date'&gt; 8&lt;/var&gt;, &lt;var data-var='time'&gt;16:01&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Update&lt;/strong&gt; - Issues is experiencing degraded performance. We are continuing to investigate.&lt;/p&gt;&lt;p&gt;&lt;small&gt;Feb &lt;var data-var='date'&gt; 8&lt;/var&gt;, &lt;var data-var='time'&gt;15:59&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Investigating&lt;/strong&gt; - We are investigating reports of degraded performance for Webhooks.&lt;/p&gt;</content>
  </entry>
  <entry>
    <id>tag:www.githubstatus.com,2005:Incident/6183972</id>
    <published>2021-02-03T17:47:44Z</published>
    <updated>2021-02-03T17:47:44Z</updated>
    <link rel="alternate" type="text/html" href="https://www.githubstatus.com/incidents/5zb8gfbl7qkt"/>
    <title>Incident on 2021-02-03 14:50 UTC</title>
    <content type="html">&lt;p&gt;&lt;small&gt;Feb &lt;var data-var='date'&gt; 3&lt;/var&gt;, &lt;var data-var='time'&gt;17:47&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Resolved&lt;/strong&gt; - This incident has been resolved.&lt;/p&gt;&lt;p&gt;&lt;small&gt;Feb &lt;var data-var='date'&gt; 3&lt;/var&gt;, &lt;var data-var='time'&gt;17:47&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Update&lt;/strong&gt; - Webhooks is operating normally.&lt;/p&gt;&lt;p&gt;&lt;small&gt;Feb &lt;var data-var='date'&gt; 3&lt;/var&gt;, &lt;var data-var='time'&gt;16:22&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Update&lt;/strong&gt; - Pull Requests is experiencing degraded performance. We are still investigating and will provide an update when we have one.&lt;/p&gt;&lt;p&gt;&lt;small&gt;Feb &lt;var data-var='date'&gt; 3&lt;/var&gt;, &lt;var data-var='time'&gt;14:50&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Investigating&lt;/strong&gt; - We are investigating reports of degraded performance for Webhooks.&lt;/p&gt;</content>
  </entry>
  <entry>
    <id>tag:www.githubstatus.com,2005:Incident/6173124</id>
    <published>2021-02-02T14:28:18Z</published>
    <updated>2021-02-02T14:28:18Z</updated>
    <link rel="alternate" type="text/html" href="https://www.githubstatus.com/incidents/jc7y32whwpzx"/>
    <title>Incident on 2021-02-02 13:52 UTC</title>
    <content type="html">&lt;p&gt;&lt;small&gt;Feb &lt;var data-var='date'&gt; 2&lt;/var&gt;, &lt;var data-var='time'&gt;14:28&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Resolved&lt;/strong&gt; - This incident has been resolved.&lt;/p&gt;&lt;p&gt;&lt;small&gt;Feb &lt;var data-var='date'&gt; 2&lt;/var&gt;, &lt;var data-var='time'&gt;14:28&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Update&lt;/strong&gt; - API Requests is operating normally.&lt;/p&gt;&lt;p&gt;&lt;small&gt;Feb &lt;var data-var='date'&gt; 2&lt;/var&gt;, &lt;var data-var='time'&gt;14:05&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Update&lt;/strong&gt; - GitHub Packages is experiencing degraded performance. We are still investigating and will provide an update when we have one.&lt;/p&gt;&lt;p&gt;&lt;small&gt;Feb &lt;var data-var='date'&gt; 2&lt;/var&gt;, &lt;var data-var='time'&gt;13:52&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Investigating&lt;/strong&gt; - We are investigating reports of degraded performance for API Requests.&lt;/p&gt;</content>
  </entry>
  <entry>
    <id>tag:www.githubstatus.com,2005:Incident/6132045</id>
    <published>2021-01-28T08:14:54Z</published>
    <updated>2021-01-28T17:30:21Z</updated>
    <link rel="alternate" type="text/html" href="https://www.githubstatus.com/incidents/tf9v5jjmq2lg"/>
    <title>Incident on 2021-01-28 04:21 UTC</title>
    <content type="html">&lt;p&gt;&lt;small&gt;Jan &lt;var data-var='date'&gt;28&lt;/var&gt;, &lt;var data-var='time'&gt;08:14&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Resolved&lt;/strong&gt; - This incident has been resolved.&lt;/p&gt;&lt;p&gt;&lt;small&gt;Jan &lt;var data-var='date'&gt;28&lt;/var&gt;, &lt;var data-var='time'&gt;07:10&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Update&lt;/strong&gt; - GitHub Actions is now  experiencing degraded performance. We are still investigating and will provide an update when we have one.&lt;/p&gt;&lt;p&gt;&lt;small&gt;Jan &lt;var data-var='date'&gt;28&lt;/var&gt;, &lt;var data-var='time'&gt;04:21&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Investigating&lt;/strong&gt; - We are investigating reports of degraded availability for GitHub Actions.&lt;/p&gt;</content>
  </entry>
  <entry>
    <id>tag:www.githubstatus.com,2005:Incident/6058441</id>
    <published>2021-01-19T14:51:40Z</published>
    <updated>2021-01-19T14:51:40Z</updated>
    <link rel="alternate" type="text/html" href="https://www.githubstatus.com/incidents/rk5rhrklkdk3"/>
    <title>Incident on 2021-01-19 11:36 UTC</title>
    <content type="html">&lt;p&gt;&lt;small&gt;Jan &lt;var data-var='date'&gt;19&lt;/var&gt;, &lt;var data-var='time'&gt;14:51&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Resolved&lt;/strong&gt; - This incident has been resolved.&lt;/p&gt;&lt;p&gt;&lt;small&gt;Jan &lt;var data-var='date'&gt;19&lt;/var&gt;, &lt;var data-var='time'&gt;14:50&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Update&lt;/strong&gt; - Pull Requests is  operating normally.&lt;/p&gt;&lt;p&gt;&lt;small&gt;Jan &lt;var data-var='date'&gt;19&lt;/var&gt;, &lt;var data-var='time'&gt;14:50&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Update&lt;/strong&gt; - GitHub Packages is  operating normally.&lt;/p&gt;&lt;p&gt;&lt;small&gt;Jan &lt;var data-var='date'&gt;19&lt;/var&gt;, &lt;var data-var='time'&gt;14:49&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Update&lt;/strong&gt; - Issues is  operating normally.&lt;/p&gt;&lt;p&gt;&lt;small&gt;Jan &lt;var data-var='date'&gt;19&lt;/var&gt;, &lt;var data-var='time'&gt;14:11&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Update&lt;/strong&gt; - Pull Requests is  experiencing degraded performance. We are still investigating and will provide an update when we have one.&lt;/p&gt;&lt;p&gt;&lt;small&gt;Jan &lt;var data-var='date'&gt;19&lt;/var&gt;, &lt;var data-var='time'&gt;13:50&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Update&lt;/strong&gt; - GitHub Packages is  experiencing degraded performance. We are continuing to investigate.&lt;/p&gt;&lt;p&gt;&lt;small&gt;Jan &lt;var data-var='date'&gt;19&lt;/var&gt;, &lt;var data-var='time'&gt;12:06&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Update&lt;/strong&gt; - GitHub Actions is  experiencing degraded performance. We are continuing to investigate.&lt;/p&gt;&lt;p&gt;&lt;small&gt;Jan &lt;var data-var='date'&gt;19&lt;/var&gt;, &lt;var data-var='time'&gt;11:36&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Investigating&lt;/strong&gt; - We are investigating reports of degraded performance for Issues.&lt;/p&gt;</content>
  </entry>
  <entry>
    <id>tag:www.githubstatus.com,2005:Incident/6025509</id>
    <published>2021-01-15T00:39:24Z</published>
    <updated>2021-01-15T00:39:24Z</updated>
    <link rel="alternate" type="text/html" href="https://www.githubstatus.com/incidents/fmt285091x64"/>
    <title>Incident on 2021-01-14 19:52 UTC</title>
    <content type="html">&lt;p&gt;&lt;small&gt;Jan &lt;var data-var='date'&gt;15&lt;/var&gt;, &lt;var data-var='time'&gt;00:39&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Resolved&lt;/strong&gt; - This incident has been resolved.&lt;/p&gt;&lt;p&gt;&lt;small&gt;Jan &lt;var data-var='date'&gt;14&lt;/var&gt;, &lt;var data-var='time'&gt;19:52&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Investigating&lt;/strong&gt; - We are investigating reports of degraded performance for GitHub Packages.&lt;/p&gt;</content>
  </entry>
  <entry>
    <id>tag:www.githubstatus.com,2005:Incident/5970188</id>
    <published>2021-01-09T17:00:35Z</published>
    <updated>2021-01-09T17:00:35Z</updated>
    <link rel="alternate" type="text/html" href="https://www.githubstatus.com/incidents/b48hhfrdlfk4"/>
    <title>GitHub Container Registry enters Read Only mode</title>
    <content type="html">&lt;p&gt;&lt;small&gt;Jan &lt;var data-var='date'&gt; 9&lt;/var&gt;, &lt;var data-var='time'&gt;17:00&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Completed&lt;/strong&gt; - The scheduled maintenance has been completed.&lt;/p&gt;&lt;p&gt;&lt;small&gt;Jan &lt;var data-var='date'&gt; 9&lt;/var&gt;, &lt;var data-var='time'&gt;16:00&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;In progress&lt;/strong&gt; - Scheduled maintenance is currently in progress. We will provide updates as necessary.&lt;/p&gt;&lt;p&gt;&lt;small&gt;Jan &lt;var data-var='date'&gt; 7&lt;/var&gt;, &lt;var data-var='time'&gt;16:07&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Scheduled&lt;/strong&gt; - On 1/9/21 at 16:00 UTC GitHub will be performing scheduled maintenance on GitHub Container Registry (ghcr.io). During this time GitHub Container Registry will be put in Read Only mode. This means that "docker pull" commands will continue to work, however docker push commands to the GitHub Container Registry will be denied during this period.&lt;/p&gt;</content>
  </entry>
  <entry>
    <id>tag:www.githubstatus.com,2005:Incident/5984694</id>
    <published>2021-01-09T16:23:55Z</published>
    <updated>2021-01-09T16:23:55Z</updated>
    <link rel="alternate" type="text/html" href="https://www.githubstatus.com/incidents/978zwz4crz1z"/>
    <title>Incident on 2021-01-09 16:03 UTC</title>
    <content type="html">&lt;p&gt;&lt;small&gt;Jan &lt;var data-var='date'&gt; 9&lt;/var&gt;, &lt;var data-var='time'&gt;16:23&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Resolved&lt;/strong&gt; - This incident has been resolved.&lt;/p&gt;&lt;p&gt;&lt;small&gt;Jan &lt;var data-var='date'&gt; 9&lt;/var&gt;, &lt;var data-var='time'&gt;16:03&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Investigating&lt;/strong&gt; - We are investigating reports of degraded performance for GitHub Packages.&lt;/p&gt;</content>
  </entry>
  <entry>
    <id>tag:www.githubstatus.com,2005:Incident/5961048</id>
    <published>2021-01-06T14:16:28Z</published>
    <updated>2021-01-06T14:16:28Z</updated>
    <link rel="alternate" type="text/html" href="https://www.githubstatus.com/incidents/tztj066g5m12"/>
    <title>Incident on 2021-01-06 14:06 UTC</title>
    <content type="html">&lt;p&gt;&lt;small&gt;Jan &lt;var data-var='date'&gt; 6&lt;/var&gt;, &lt;var data-var='time'&gt;14:16&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Resolved&lt;/strong&gt; - This incident has been resolved.&lt;/p&gt;&lt;p&gt;&lt;small&gt;Jan &lt;var data-var='date'&gt; 6&lt;/var&gt;, &lt;var data-var='time'&gt;14:06&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Investigating&lt;/strong&gt; - We are investigating reports of degraded availability for GitHub Docs.&lt;/p&gt;</content>
  </entry>
  <entry>
    <id>tag:www.githubstatus.com,2005:Incident/5849619</id>
    <published>2020-12-21T01:23:56Z</published>
    <updated>2020-12-21T01:23:56Z</updated>
    <link rel="alternate" type="text/html" href="https://www.githubstatus.com/incidents/mfclbl56d617"/>
    <title>Incident on 2020-12-21 00:20 UTC</title>
    <content type="html">&lt;p&gt;&lt;small&gt;Dec &lt;var data-var='date'&gt;21&lt;/var&gt;, &lt;var data-var='time'&gt;01:23&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Resolved&lt;/strong&gt; - This incident has been resolved.&lt;/p&gt;&lt;p&gt;&lt;small&gt;Dec &lt;var data-var='date'&gt;21&lt;/var&gt;, &lt;var data-var='time'&gt;00:20&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Investigating&lt;/strong&gt; - We are investigating reports of degraded performance for Issues.&lt;/p&gt;</content>
  </entry>
  <entry>
    <id>tag:www.githubstatus.com,2005:Incident/5842902</id>
    <published>2020-12-19T22:49:07Z</published>
    <updated>2020-12-19T22:49:07Z</updated>
    <link rel="alternate" type="text/html" href="https://www.githubstatus.com/incidents/w5mwfprfbzzy"/>
    <title>Incident on 2020-12-19 20:34 UTC</title>
    <content type="html">&lt;p&gt;&lt;small&gt;Dec &lt;var data-var='date'&gt;19&lt;/var&gt;, &lt;var data-var='time'&gt;22:49&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Resolved&lt;/strong&gt; - This incident has been resolved.&lt;/p&gt;&lt;p&gt;&lt;small&gt;Dec &lt;var data-var='date'&gt;19&lt;/var&gt;, &lt;var data-var='time'&gt;20:34&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Investigating&lt;/strong&gt; - We are investigating reports of degraded performance for Pull Requests.&lt;/p&gt;</content>
  </entry>
  <entry>
    <id>tag:www.githubstatus.com,2005:Incident/5799178</id>
    <published>2020-12-18T19:01:59Z</published>
    <updated>2020-12-18T19:01:59Z</updated>
    <link rel="alternate" type="text/html" href="https://www.githubstatus.com/incidents/3jr0nb7zjpbm"/>
    <title>macOS virtual environment maintenance</title>
    <content type="html">&lt;p&gt;&lt;small&gt;Dec &lt;var data-var='date'&gt;18&lt;/var&gt;, &lt;var data-var='time'&gt;19:01&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Completed&lt;/strong&gt; - The scheduled maintenance has been completed.&lt;/p&gt;&lt;p&gt;&lt;small&gt;Dec &lt;var data-var='date'&gt;15&lt;/var&gt;, &lt;var data-var='time'&gt;10:00&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;In progress&lt;/strong&gt; - Scheduled maintenance is currently in progress. We will provide updates as necessary.&lt;/p&gt;&lt;p&gt;&lt;small&gt;Dec &lt;var data-var='date'&gt;14&lt;/var&gt;, &lt;var data-var='time'&gt;16:37&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Scheduled&lt;/strong&gt; - During the week of December 14-18, GitHub will be performing scheduled maintenance on one of our datacenters that may result in longer than normal queue times for macOS workflows using macos-latest, macos-10.14, or macos-10.15. In some cases, it may be possible that jobs will time out or be cancelled, and will need to be restarted.&lt;br /&gt;&lt;br /&gt;We apologize for the inconvenience and encourage you to be patient, and give your workflows extra time to run. Follow our progress in the virtual-environments repo &lt;a href="https://github.com/actions/virtual-environments/issues/2247"&gt;here&lt;/a&gt;.&lt;/p&gt;</content>
  </entry>
  <entry>
    <id>tag:www.githubstatus.com,2005:Incident/5820041</id>
    <published>2020-12-17T00:54:34Z</published>
    <updated>2020-12-17T00:54:34Z</updated>
    <link rel="alternate" type="text/html" href="https://www.githubstatus.com/incidents/lxrh19v90f8x"/>
    <title>Incident on 2020-12-16 23:34 UTC</title>
    <content type="html">&lt;p&gt;&lt;small&gt;Dec &lt;var data-var='date'&gt;17&lt;/var&gt;, &lt;var data-var='time'&gt;00:54&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Resolved&lt;/strong&gt; - This incident has been resolved.&lt;/p&gt;&lt;p&gt;&lt;small&gt;Dec &lt;var data-var='date'&gt;17&lt;/var&gt;, &lt;var data-var='time'&gt;00:54&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Update&lt;/strong&gt; - Pull Requests is  operating normally.&lt;/p&gt;&lt;p&gt;&lt;small&gt;Dec &lt;var data-var='date'&gt;17&lt;/var&gt;, &lt;var data-var='time'&gt;00:54&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Update&lt;/strong&gt; - Issues is  operating normally.&lt;/p&gt;&lt;p&gt;&lt;small&gt;Dec &lt;var data-var='date'&gt;17&lt;/var&gt;, &lt;var data-var='time'&gt;00:54&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Update&lt;/strong&gt; - API Requests is  operating normally.&lt;/p&gt;&lt;p&gt;&lt;small&gt;Dec &lt;var data-var='date'&gt;16&lt;/var&gt;, &lt;var data-var='time'&gt;23:49&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Update&lt;/strong&gt; - GitHub Packages is  experiencing degraded performance. We are still investigating and will provide an update when we have one.&lt;/p&gt;&lt;p&gt;&lt;small&gt;Dec &lt;var data-var='date'&gt;16&lt;/var&gt;, &lt;var data-var='time'&gt;23:42&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Update&lt;/strong&gt; - Pull Requests is  experiencing degraded performance. We are still investigating and will provide an update when we have one.&lt;/p&gt;&lt;p&gt;&lt;small&gt;Dec &lt;var data-var='date'&gt;16&lt;/var&gt;, &lt;var data-var='time'&gt;23:42&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Update&lt;/strong&gt; - Issues is  experiencing degraded performance. We are still investigating and will provide an update when we have one.&lt;/p&gt;&lt;p&gt;&lt;small&gt;Dec &lt;var data-var='date'&gt;16&lt;/var&gt;, &lt;var data-var='time'&gt;23:34&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Investigating&lt;/strong&gt; - We are investigating reports of degraded performance for API Requests.&lt;/p&gt;</content>
  </entry>
  <entry>
    <id>tag:www.githubstatus.com,2005:Incident/5789559</id>
    <published>2020-12-13T08:38:26Z</published>
    <updated>2020-12-13T08:38:26Z</updated>
    <link rel="alternate" type="text/html" href="https://www.githubstatus.com/incidents/1n2vnq4v6mkf"/>
    <title>Incident on 2020-12-13 07:42 UTC</title>
    <content type="html">&lt;p&gt;&lt;small&gt;Dec &lt;var data-var='date'&gt;13&lt;/var&gt;, &lt;var data-var='time'&gt;08:38&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Resolved&lt;/strong&gt; - This incident has been resolved.&lt;/p&gt;&lt;p&gt;&lt;small&gt;Dec &lt;var data-var='date'&gt;13&lt;/var&gt;, &lt;var data-var='time'&gt;07:42&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Investigating&lt;/strong&gt; - We are investigating reports of degraded performance for Issues.&lt;/p&gt;</content>
  </entry>
  <entry>
    <id>tag:www.githubstatus.com,2005:Incident/5698072</id>
    <published>2020-12-01T22:33:53Z</published>
    <updated>2020-12-01T22:33:53Z</updated>
    <link rel="alternate" type="text/html" href="https://www.githubstatus.com/incidents/sxthxvdlljk4"/>
    <title>Incident on 2020-12-01 16:40 UTC</title>
    <content type="html">&lt;p&gt;&lt;small&gt;Dec &lt;var data-var='date'&gt; 1&lt;/var&gt;, &lt;var data-var='time'&gt;22:33&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Resolved&lt;/strong&gt; - This incident has been resolved.&lt;/p&gt;&lt;p&gt;&lt;small&gt;Dec &lt;var data-var='date'&gt; 1&lt;/var&gt;, &lt;var data-var='time'&gt;16:40&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Investigating&lt;/strong&gt; - We are investigating reports of degraded performance for GitHub Actions.&lt;/p&gt;</content>
  </entry>
  <entry>
    <id>tag:www.githubstatus.com,2005:Incident/5670486</id>
    <published>2020-11-27T17:07:37Z</published>
    <updated>2020-11-27T17:07:37Z</updated>
    <link rel="alternate" type="text/html" href="https://www.githubstatus.com/incidents/vts24q167tc8"/>
    <title>Incident on 2020-11-27 16:04 UTC</title>
    <content type="html">&lt;p&gt;&lt;small&gt;Nov &lt;var data-var='date'&gt;27&lt;/var&gt;, &lt;var data-var='time'&gt;17:07&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Resolved&lt;/strong&gt; - This incident has been resolved.&lt;/p&gt;&lt;p&gt;&lt;small&gt;Nov &lt;var data-var='date'&gt;27&lt;/var&gt;, &lt;var data-var='time'&gt;16:35&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Update&lt;/strong&gt; - GitHub Actions is  now  experiencing degraded performance. We are continuing to investigate.&lt;/p&gt;&lt;p&gt;&lt;small&gt;Nov &lt;var data-var='date'&gt;27&lt;/var&gt;, &lt;var data-var='time'&gt;16:04&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Investigating&lt;/strong&gt; - We are investigating reports of degraded availability for GitHub Actions.&lt;/p&gt;</content>
  </entry>
  <entry>
    <id>tag:www.githubstatus.com,2005:Incident/5597741</id>
    <published>2020-11-18T23:38:07Z</published>
    <updated>2020-11-18T23:38:07Z</updated>
    <link rel="alternate" type="text/html" href="https://www.githubstatus.com/incidents/j4vrg2ljjb5c"/>
    <title>Incident on 2020-11-18 23:02 UTC</title>
    <content type="html">&lt;p&gt;&lt;small&gt;Nov &lt;var data-var='date'&gt;18&lt;/var&gt;, &lt;var data-var='time'&gt;23:38&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Resolved&lt;/strong&gt; - This incident has been resolved.&lt;/p&gt;&lt;p&gt;&lt;small&gt;Nov &lt;var data-var='date'&gt;18&lt;/var&gt;, &lt;var data-var='time'&gt;23:36&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Update&lt;/strong&gt; - Issues is  operating normally.&lt;/p&gt;&lt;p&gt;&lt;small&gt;Nov &lt;var data-var='date'&gt;18&lt;/var&gt;, &lt;var data-var='time'&gt;23:36&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Update&lt;/strong&gt; - Pull Requests is  operating normally.&lt;/p&gt;&lt;p&gt;&lt;small&gt;Nov &lt;var data-var='date'&gt;18&lt;/var&gt;, &lt;var data-var='time'&gt;23:26&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Update&lt;/strong&gt; - Pull Requests is  experiencing degraded performance. We are continuing to investigate.&lt;/p&gt;&lt;p&gt;&lt;small&gt;Nov &lt;var data-var='date'&gt;18&lt;/var&gt;, &lt;var data-var='time'&gt;23:24&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Update&lt;/strong&gt; - Issues is  experiencing degraded performance. We are continuing to investigate.&lt;/p&gt;&lt;p&gt;&lt;small&gt;Nov &lt;var data-var='date'&gt;18&lt;/var&gt;, &lt;var data-var='time'&gt;23:02&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Investigating&lt;/strong&gt; - We are investigating reports of degraded performance for Webhooks.&lt;/p&gt;</content>
  </entry>
  <entry>
    <id>tag:www.githubstatus.com,2005:Incident/5576406</id>
    <published>2020-11-10T20:00:00Z</published>
    <updated>2020-11-16T09:55:07Z</updated>
    <link rel="alternate" type="text/html" href="https://www.githubstatus.com/incidents/nk9gdvl03ztg"/>
    <title>Incident on 2020-11-02 12:14 UTC</title>
    <content type="html">&lt;p&gt;&lt;small&gt;Nov &lt;var data-var='date'&gt;10&lt;/var&gt;, &lt;var data-var='time'&gt;20:00&lt;/var&gt; UTC&lt;/small&gt;&lt;br&gt;&lt;strong&gt;Resolved&lt;/strong&gt; - We are investigating reports of degraded performance for GitHub Packages.&lt;/p&gt;</content>
  </entry>
</feed>
Huawei devices using Kirin processors have a different partitioning method from most common devices. Magisk is usually installed to the `boot` partition of the device, however Huawei devices do not have this partition.

Generally, follow [Patching Images](#patching-images) with some differences from the original instructions:

- After downloading your firmware zip (you may find a lot in [Huawei Firmware Database](http://pro-teammt.ru/firmware-database/)), you have to extract images from `UPDATE.APP` in the zip with [Huawei Update Extractor](https://forum.xda-developers.com/showthread.php?t=2433454) (Windows only!)
- Regarding patching images:
  - If your device has boot ramdisk, patch `RAMDISK.img` instead of `boot.img`
  - If your device does **NOT** have boot ramdisk, patch `RECOVERY_RAMDIS.img` (this is not a typo) instead of `recovery.img`
- When flashing the image back with `fastboot`
  - If you patched `RAMDISK.img`, flash with command `fastboot flash ramdisk magisk_patched.img`
  - If you patched `RECOVERY_RAMDIS.img`, flash with command `fastboot flash recovery_ramdisk magisk_patched.img`
