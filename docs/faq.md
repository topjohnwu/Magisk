# Frequently Asked Questions

### Q: I installed a module and it bootlooped my device. Help!

If you have USB debugging enabled in developer options, connect your phone to the PC. If your device is detected (check by `adb devices`), enter ADB shell and run the command `magisk --remove-modules`. This will remove all your modules and automatically reboot the device.

If unfortunately you do not have USB debugging enabled, reboot into Safe Mode. Most modern Android devices support a special key combo at boot to enter Safe Mode as an emergency option.

Magisk can detect Safe Mode key combo in a similar way to system and activate 'Magisk Safe Mode' which disables all modules.

Please Note: Magisk detects the key combo earlier than system, so it's possible to activate Magisk Safe Mode but not system Safe Mode, and vice versa... Key combo detection can result in the following:

(1) Many online guides for entering Safe Mode say 'when the animated logo appears, press and hold the volume down button until the system boots' or similar... This may actually be to late for Magisk detection however and result in activating system Safe Mode but modules aren't disabled... (2) If user presses vol. down button some seconds before the animation but releases it when boot animation appears, activating Magisk Safe Mode may be achieved without activating system Safe Mode and device should then simply boot to normal system with modules disabled. Next, manage modules through the Magisk app... (3) If user presses vol. down button some seconds before the animation and holds it until the system boots, both Magisk Safe Mode and system Safe Mode can be activated. Next, reboot back to normal system mode and manage disabled modules through the Magisk app.

It can be seen that varying prescribed timing (eg. pwr button may need to be held for a few seconds (varies), then vol. down pressed some seconds before animated logo appears) for activating Safe Mode is often required to activate Magisk 'Safe Mode' (which really just adds an empty directory named 'disable' to modules directory to disable modules when booting with Magisk) and thereby bypass module related bootloop conditions... Further, achieving timing for (2) above may be preferable to (3) since system Safe Mode will also reset many other user preferences.

### Q: Why is X app detecting root?

Magisk no longer handles root hiding. There are plenty of Magisk/Zygisk modules available that specifically provide these functionalities, please search around 😉

### Q: After I hidden the Magisk app, the app icon is broken.

When hiding the Magisk app, it will install a "stub" APK that has nothing in it. The only functionality this stub app has is downloading the full Magisk app APK into its internal storage and dynamically loading it. Due to the fact that the APK is literally _empty_, it does not contain the image resource for the app icon.

When you open the hidden Magisk app, it will offer you the option to create a shortcut in the homescreen (which has both the correct app name and icon) for your convenience. You can also manually ask the app to create the icon in app settings.
