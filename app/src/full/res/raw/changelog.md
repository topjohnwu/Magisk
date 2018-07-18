### v5.8.1
- Fix a bug that cause the root shell initializer not running in BusyBox.
This is the cause of freezes on some older devices and DTBO being patched unconditionally.

### Note
If your device has a separate DTBO, and you do NOT want it patched but it was patched due to the bug,
follow these instructions to fix it:

1. Uninstall → Restore Images. Do **NOT** reboot afterwards!
2. Check **Preserve AVB 2.0/dm-verity** in Advanced Settings
3. Install → Install → Direct Install to install correctly with proper settings
