# Frequently Asked Questions

### Q: I installed a module and it bootlooped my device. Help!

If you have USB debugging enabled in developer options, connect your phone to the PC. If your device is detected (check by `adb devices`), enter ADB shell and run the command `magisk --remove-modules`. This will remove all your modules and automatically reboot the device.

If unfortunately you do not have USB debugging enabled you can boot using the Safe Mode key combo to cause Magisk to create an empty file named 'disable' in modules directories which disables modules when next booted with Magisk. Most modern Android devices support such a special key combo at boot to enter system Safe Mode as an emergency option, but **please note** that Magisk's key combo detection occurs _earlier_ than system detection so the key combo timing indicated by many online guides may need to be altered to activate Magisk's Safe Mode. (It's possible to activate system Safe Mode but not Magisk Safe Mode and vice versa.)

The following details should ensure that modules are properly disabled:

1. Many online guides for entering Safe Mode say 'When the animated logo appears, press and hold the volume down button until the system boots' or similar. This may actually be _too late_ for Magisk detection however and result in activating system Safe Mode but modules are not disabled.
2. By pressing the volume down button some seconds before the animation and releasing it as soon as the boot animation appears, Magisk's Safe Mode should be activated without activating system Safe Mode (thus avoiding disabling other device and app settings) and the device should then simply boot to normal system with modules disabled.
3. By pressing the volume down button some seconds before the animation and holding it until the system boots, both Magisk's Safe Mode and system Safe Mode should be activated. Next, after booting back to normal system, modules will be disabled.

### Q: Why is X app detecting root?

Magisk no longer handles root hiding. There are plenty of Magisk/Zygisk modules available that specifically provide these functionalities, please search around 😉

### Q: After I hidden the Magisk app, the app icon is broken.

When hiding the Magisk app, it will install a "stub" APK that has nothing in it. The only functionality this stub app has is downloading the full Magisk app APK into its internal storage and dynamically loading it. Due to the fact that the APK is literally _empty_, it does not contain the image resource for the app icon.

When you open the hidden Magisk app, it will offer you the option to create a shortcut in the homescreen (which has both the correct app name and icon) for your convenience. You can also manually ask the app to create the icon in app settings.

### Q: How to use Magisk in the emulator?

With the emulator running and accessible via ADB, run `./build.py emulator <path to Magisk APK>` to temporarily install Magisk on to the emulator. The patch is not persistent, meaning Magisk will be lost after a reboot, so re-execute the script to emulate a reboot if required.

The script is only tested on the official Android Virtual Device (AVD) shipped alongside Android Studio; other emulators may work, but the emulator must have SELinux enabled.
