# Frequently Asked Questions

### Q: I installed a module and it bootlooped my device. Help!

If you have USB debugging enabled in developer options, connect your phone to the PC. If your device is detected (check by `adb devices`), enter ADB shell and run the command `magisk --remove-modules`. This will remove all your modules and automatically reboot the device.

If unfortunately you do not have USB debugging enabled, reboot into Safe Mode. Most modern Android devices support a special key combo at boot to enter Safe Mode as an emergency option and Magisk can detect the Safe Mode key combo in a similar way to system to disable Magisk modules.

Please Note: Magisk detects the key combo earlier than system, so it's possible to activate Magisk Safe Mode but not system Safe Mode and vice versa. Key combo detection can therefore be inconsistent but the following details should ensure that modules are properly disabled:
1) Many online guides for entering Safe Mode say 'When the animated logo appears, press and hold the volume down button until the system boots' or similar. This may actually be too late for Magisk detection however and result in activating system Safe Mode but modules aren't disabled.
2) By pressing the volume down button some seconds before the animation and releasing it when the boot animation appears, Magisk Safe Mode should be activated without activating system Safe Mode and the device should then simply boot to normal system with modules disabled.
3) By pressing the volume down button some seconds before the animation and holding it until the system boots, both Magisk Safe Mode and system Safe Mode should be activated. Next, after rebooting back to normal system modules will be disabled.

Varying the key combo timing that online guides give for entering system Safe Mode is often needed to activate Magisk 'Safe Mode' as described above, but this will add an empty directory named 'disable' to the modules directory to disable modules when next booted with Magisk thereby bypassing module related bootloop conditions.

Further, achieving timing for scenario 2. above may be preferable to 3. since system Safe Mode will also reset many other user preferences. Disabled modules can be managed through the Magisk app.

### Q: Why is X app detecting root?

Magisk no longer handles root hiding. There are plenty of Magisk/Zygisk modules available that specifically provide these functionalities, please search around 😉

### Q: After I hidden the Magisk app, the app icon is broken.

When hiding the Magisk app, it will install a "stub" APK that has nothing in it. The only functionality this stub app has is downloading the full Magisk app APK into its internal storage and dynamically loading it. Due to the fact that the APK is literally _empty_, it does not contain the image resource for the app icon.

When you open the hidden Magisk app, it will offer you the option to create a shortcut in the homescreen (which has both the correct app name and icon) for your convenience. You can also manually ask the app to create the icon in app settings.
