### 5.9.1
- Prevent upgrading database before upgrading to v17.0

### v5.9.0
- No more on boot notifications
- Support new mechanism for installing to inactive slot for OTAs on A/B devices
- Fix restore Magisk Manager settings on Android P
- Verify existing file checksums to prevent unnecessary re-downloads
- Update SNET extension to use new Google API, fix "Invalid Response" errors
- Move fingerprint settings to magisk database to prevent the settings to be easily removed
- Fingerprint settings are now guarded with fingerprint authentications before it can get changed
- Prevent any files to be downloaded to `/sdcard/MagiskManager`
