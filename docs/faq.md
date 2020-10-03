# Frequently Asked Questions

### Q: Why is X app detecting root?

Manually enable MagiskHide in settings (MagiskHide is no longer enabled by default). Also, there are known methods to detect Magisk, so your mileage may vary.

### Q: I installed a module and it bootlooped my device. Help!

If you have USB debugging enabled in developer options, connect your phone to the PC. If your device is detected (check by `adb devices`), enter ADB shell and run the command `magisk --remove-modules`. This will remove all your modules and automatically reboot the device.

If unfortunately you do not have USB debugging enabled, reboot into Safe Mode. Most modern Android devices support pressing a special key combo at boot to enter Safe Mode as an emergency option. Magisk will detect Safe Mode being activated, and all modules will be disabled. Then reboot back to normal mode (the module disable state persists) and manage your modules through Magisk Manager.
