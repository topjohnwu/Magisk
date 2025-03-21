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

### Q: Magisk App shows Magisk Installed = N/A after an update but magisk su is still working.

If upgrading with App hidden (ie. you took the 'Hide the Magisk app' option), the stub app (for hiding Magisk) may remain while a full Magisk app is also installed. This creates a conflict and the full app fails to see or access root... Uninstalling and reinstalling the full app can fix this, but not if a hidden app (stub) still exists.

The solution is to check for a hidden stub app and remove it. It may not show up normally in your launcher homescreen any longer, but should be visible from general settings, Apps. The hidden app will be named 'Settings' (default) or whatever you named it during the hiding process. Note that it is possible to have multiple obfuscated apps present. Uninstall any iterations of the hidden app you find and try opening the full app again. If necessary, uninstall it and reinstall the full app matching the binaries installed. Typing magisk -c in a terminal emulator app will show the version and version code for Magisk binaries installed (despite Installed = N/A showing).

Additionally, if a 'second space', eg. Workspace, Parallel Space etc, or another sandboxed environment, eg. a Multiple User additional profile, Island app or similar, is set up, check that no iterations of Magisk (either hidden or full apps) are running within these environments.

### Q: After I take the 'Hide the Magisk app' option the app icon is broken.

When hiding the Magisk app, it will install a "stub" APK that has nothing in it. The only functionality this stub app has is to download the full Magisk app APK data into its internal storage and dynamically load it. Due to the fact that the stub APK is literally empty, it does not contain the image resource for the app icon.

When you open the hidden Magisk app, it will offer you the option to create a shortcut in the homescreen (which has both the correct app name and icon) for your convenience. You can also manually ask the app to create the icon in app settings.

### Q: How to use Magisk in the emulator?

With the emulator running and accessible via ADB, run `./build.py emulator <path to Magisk APK>` to temporarily install Magisk on to the emulator. The patch is not persistent, meaning Magisk will be lost after a reboot, so re-execute the script to emulate a reboot if required.

The script is only tested on the official Android Virtual Device (AVD) shipped alongside Android Studio; other emulators may work, but the emulator must have SELinux enabled.
