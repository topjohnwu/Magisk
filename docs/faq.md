# Frequently Asked Questions

### Q: I installed a module and it bootlooped my device. Help!

Magisk includes built-in bootloop protection: if your device fails to finish booting (`boot_completed`) **twice in a row**, Magisk will automatically enter **Safe Mode** on the next boot, disabling all modules (by creating a `disable` file in each module folder) and disabling Zygisk so your device can boot normally.

If your device is stuck before automatic bootloop detection can recover it, or you want to manually recover:

- If you have USB debugging enabled in developer options, connect your phone to the PC. If your device is detected (check by `adb devices`), enter ADB shell and run the command `magisk --remove-modules`. This will remove all your modules and automatically reboot the device.
- If your device can still boot into Android, you can also toggle **Safe Mode** directly from the Reboot menu (power icon) at the top of the Magisk app Home screen, then reboot.

If you do not have USB debugging enabled and the device cannot boot, you can trigger Safe Mode manually using the hardware key combo at boot. Most modern Android devices support a special key combo at boot to enter system Safe Mode as an emergency option, but **please note** that Magisk's key combo detection occurs _earlier_ than system detection so the key combo timing indicated by many online guides may need to be altered to activate Magisk's Safe Mode. (It's possible to activate system Safe Mode but not Magisk Safe Mode and vice versa.)

The following details should ensure that modules are properly disabled:

1. Many online guides for entering Safe Mode say 'When the animated logo appears, press and hold the volume down button until the system boots' or similar. This may actually be _too late_ for Magisk detection however and result in activating system Safe Mode but modules are not disabled.
2. By pressing the volume down button some seconds before the animation and releasing it as soon as the boot animation appears, Magisk's Safe Mode should be activated without activating system Safe Mode (thus avoiding disabling other device and app settings) and the device should then simply boot to normal system with modules disabled.
3. By pressing the volume down button some seconds before the animation and holding it until the system boots, both Magisk's Safe Mode and system Safe Mode should be activated. Next, after booting back to normal system, modules will be disabled.

### Q: Why is X app detecting root?

Magisk no longer handles root hiding. There are plenty of Magisk/Zygisk modules available that specifically provide these functionalities, please search around 😉

### Q: How to use Magisk in the emulator?

With the emulator running and accessible via ADB, run `./build.py emulator [path to Magisk APK]` (or `./build.py emulator -b` to build and install from source) to temporarily start Magisk on the running emulator. This runtime setup is not persistent across cold reboots, so re-execute the command to emulate a reboot if required.

For a persistent installation on an Android Virtual Device (AVD), you can patch its `ramdisk.img` directly on your host machine using `./build.py patch --avd <path/to/ramdisk.img> [output]`.

These commands are tested on the official Android Virtual Device (AVD) shipped alongside Android Studio; other emulators may work, but the emulator must have SELinux enabled.
