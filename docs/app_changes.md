# Magisk Manager Changelog

### v8.0.7

- Fix sepolicy rule migration when upgrading

### v8.0.6

- Minor UI changes
- Update internal scripts

### v8.0.5

- Fix sepolicy rule copying

### v8.0.4

- A lot of stability changes and minor bug fixes
- Collect device properties, app logcat, and Magisk logs when saving logs in the logs menu

### v8.0.3

- Switch to the new Magisk Module Repo setup in preparation to allow 3rd party repos
- Add tapjacking protection on Superuser request dialog
- Stability changes and bug fixes

### v8.0.2

- Fix an issue with requesting permission on devices older than Android 10
- Make more files download through CDN

### v8.0.1

- Fix `vbmeta.img` patching for Samsung `AP.tar` files. This fixes bootloops on devices like Galaxy S10 after flashing updated AP files.
- Properly truncate existing files before writing to prevent corrupted files
- Prevent a possible UI loop when device ran into very low memory
- Switch to use JSDelivr CDN for several files

### v8.0.0

- 100% full app rewrite! Will highlight functional changes below.
- Add detailed device info in home screen to assist user installation
- Support Magisk v21.0 communication protocol
- Support patching modern Samsung `AP.tar`

### v7.5.1

- Fix toggling app components in MagiskHide screen
- Update translations

### v7.5.0

- Support new MagiskSU communication method (ContentProvider)
- Fix several issues with hidden stub APK
- Support using BiometricPrompt (face unlock)

### v7.4.0

- Hide Magisk Manager with stub APKs on Android 9.0+
- Allow customizing app name when hiding Magisk Manager
- Generate random keys to sign the hidden Magisk Manager to prevent signature detections
- Fix fingerprint UI infinite loop

### v7.3.5

- Sort installed modules by name
- Better pre-5.0 support
- Fix potential issues when patching tar files

### v7.3.4

- App is now fully written in Kotlin!
- New downloading system
- Add new "Recovery Mode" to Advanced Settings

### v7.3.0/1/2

- HUGE code base modernization, thanks @diareuse!
- More sweet changes coming in the future!
- Reboot device using proper API (no more abrupt reboot)
- New floating button in Magisk logs to go to bottom

### v7.2.0

- Huge UI overhaul
- More sweet changes coming in the future!

### v7.1.2

- Support patching Samsung AP firmware
- Much better module downloading mechanism

### v7.1.1

- Fix a bug that causes some modules using new format not showing up

### v7.1.0

- Support the new module format
- Support per-application component granularity MagiskHide targets (only on v19+)
- Ask for fingerprint before deleting rules if enabled
- Fix the bug that causes repackaging to lose settings
- Several UI fixes

### v7.0.0

- Major UI redesign!
- Render Markdown natively (no more buggy WebView!)
- Support down to Android 4.1 (native Magisk only support Android 4.2 though)
- Significantly improve Magisk log display performance
- Fix post OTA scripts for A/B devices
- Reduce memory usages when verifying and signing boot image
- Drop support for Magisk lower than v18.0

### v6.1.0

- Introduce new downloading methods: no longer uses buggy system Download Manager
- Introduce many new notifications for better user experience
- Add support for Magisk v18.0
- Change application name to "Manager" after hiding(repackaging) to prevent app name detection
- Add built-in systemless hosts module (access in settings)
- Auto launch the newly installed app after hiding(repackaging) and restoring Magisk Manager
- Fix bug causing incomplete module.prop in modules to have improper UI

### v6.0.1

- Update to use new online module's organizing method
- When fingerprint authentication is enabled, toggling root permissions in "Superuser" section now requires fingerprint beforehand
- Fix crashes when entering MagiskHide section on some devices
- Remove support to Magisk version lower than v15.0
- Ask storage permissions before patching stock boot image
- Update dark theme CardView color

### v6.0.0

- Update to latest AndroidX support library
- Fix crashes when online repos contain incomplete metadata
- Optimize BootSigner to use as little memory as possible, prevent OutOfMemoryError
- Support new communication scheme between Magisk v17.2 and Magisk Manager
- Enable excessive obfuscation to prevent APK analysis root detections (still not 100% obfuscated due to backwards compatibility with stable channel)

### v5.9.0/v5.9.1

- No more on boot notifications
- Support new mechanism for installing to inactive slot for OTAs on A/B devices
- Fix restore Magisk Manager settings on Android P
- Verify existing file checksums to prevent unnecessary re-downloads
- Update SNET extension to use new Google API, fix "Invalid Response" errors
- Move fingerprint settings to magisk database to prevent the settings to be easily removed
- Fingerprint settings are now guarded with fingerprint authentications before it can get changed
- Prevent any files to be downloaded to /sdcard/MagiskManager

### v5.8.3

- Prevent invalid modules in the online repo crashing the app
- Update Stable and Beta channel URLs

### v5.8.1

- Fix a bug that cause the root shell initializer not running in BusyBox environment

### v5.8.0

- Remain hidden when upgrading within repackaged Magisk Manager
- New feature: support reconstructing a proper Magisk environment if error detected (e.g. after factory reset)
- New uninstall method: download uninstaller and completely remove Magisk + Magisk Manager, following with a reboot
- Hidden apps are now shown on the top of the list in MagiskHide fragment
- Tons of under-the-hood bug fixes and improvements

### v5.7.0

- Add app shortcuts for Android 7.1+
- Bump minimal module minMagisk requirement to 1500
- Adjustments for new sepolicies on v16.4+
- Fix crashes when refreshing the online repo

### v5.6.4

- Remove the blacklisted apps using SafetyNet (e.g. Pokemon GO)

### v5.6.3

- Fix repo loading UI logic

### v5.6.2

- Cleanup folders if installation failed
- Add support for Android P

### v5.6.1

- Fix database crashes on F2FS with SQLite 3.21.0+
- Optimize several settings options
- Use native XML for settings migration

### v5.6.0

- Remove JNI requirement, Magisk Manager is now pure Java
- Update the method of handling su database, may fix the issue that root requests won't save
- Add the option to restore Magisk Manager after repackaging with random package name
- Massive under-the-hood

### v5.5.5

- Fix crashes on Lollipop and some devices not following AOSP standards

### v5.5.4

- Fix dtbo on-boot detection, should follow configured dtbo patching behavior on Pixel 2 devices
- Add fingerprint authentication for Superuser requests

### v5.5.3

- Update translations
- Update internal scripts (in sync with Magisk)
- Minor adjustments

### v5.5.2

- Support sorting online repos with last update
- Fix issue that advanced installation settings won't stick
- Prevent sudb crashing Magisk Manager

### v5.5.1
- Fix an issue in setting up superuser database, which causes some users to experience tons of root issues

### v5.5.0

- Fix dynamic resource loading, prevent crashes when checking SafetyNet
- Update SignAPK to use very little RAM for supporting old devices
- Support settings migration after hiding Magisk Manager
- Add reboot menu in modules section
- Add dark theme to superuser request dialogs
- Properly handle new HIGHCOMP and add recommended KEEPVERITY and KEEPFORCEENCRYPT flags for installation
- Support new paths for v14.6
- Massive improvements in repackaging Magisk Manager

### v5.4.3

- Add flags to intent to prevent crashes
- Update translations

### v5.4.2

- Support new paths and setup of v14.5
- Support repackaging Magisk Manager for hiding (only works on v14.5+)
- Support hardlinking global su database into app data
- Support signing boot images (AVB 1.0)
- Update app icon to adaptive icons
- Remove app from MagiskHide list if uninstalled
- Add support to save detailed logs when installing Magisk or modules
- Fix download progress error if module is larger than 20MB
- Changed the way how downloaded repos are processed, should be rock stable
- Prevent crashes when database is corrupted - clear db instead
- Fix saving wrong UID issue on multiuser mode
- Add custom update channel support - you can now switch to your own update server!
- Some UI adjustments and asynchronous UI performance improvements

### v5.4.0

- SafetyNet checks now require external code extension (for 100% FOSS)
- Repo loading will now show real-time progress instead of blank screen
- Show progress when downloading an online module
- Allow secondary users to access superuser settings if allowed
- Fix several places where external storage is needed but forgot to request
- Fetching online repo info from sever is significantly faster thanks to multithreading
- Pulling down Download page will now force a full refresh, thanks to the faster loading speed
- Using new resetprop tool to properly detect MagiskHide status

### v5.3.5

- Fix error when MagiskManager folder doesn't exist
- Offload many logic to scripts: script fixes will also be picked up in the app
- Add installing Magisk to second slot on A/B partition devices
- Support file based encryption: store necessary files into DE storage
- Update uninstall method to self remove app and prompt user to manually reboot

### v5.3.0

- Add hide Magisk Manager feature - hide the app from detection
- Add update channel settings - you can now receive beta updates through the app
- Proper runtime permission implementation - request storage permission only when needed
- Add boot image file patch feature - you can patch boot images without root!
- Rewrite Magisk direct install method - merge with boot image file patch mode
- Add feature to restore stock boot image - convenient for applying OTAs

### v5.2.0

- Fix force close which occurs when failure in flashing zips
- Remove several external dependencies and rewrite a large portion of components
- Improve MarkDown support: showing README.MD is much faster and will properly render Unicode characters (e.g. Chinese characters)
- Add language settings: you can now switch to languages other than system default
- Remove busybox included within APK; download through Internet if needed
- Use Magisk internal busybox if detected
- Busybox is added to the highest priority in PATH to create reliable shell environment
- Always use global namespace for internal shell if possible

### v5.1.1

- Fix Magisk Manager hanging when reading files with no end newline
- Massive rewrite AsyncTasks to prevent potential memory leak
- Fix some minor issues with notifications
- Improve update notification and popup behavior
- Update internal uninstaller script

### v5.1.0

- Introduce a new flash log activity, so you know what is actually happening, just like flashing in custom recoveries!
- Rewritten Java native shall interface: merged root shell and normal shell
- Cleaned up implementation of repo recyclerview and adapters

### v5.0.6

- Fix crash when installing modules downloading from repos

### v5.0.5

- Fix update notifications on Android O
- Fix crash when trying to install Magisk Manager update
- Update translations

### v5.0.4

- Fix bug in su timeout

### v5.0.3

- Fix FC on boot on Android O
- Adapt to Android O broadcast limitations: re-authenticate app when update is disabled on Android O

### v5.0.2
- Rewrite zip signing part, zips downloaded from repo will be properly signed and adjusted for custom recoveries

### v5.0.1

- Add namespace mode options
- Fix a bug in Manager OTA system

### v5.0.0

- Support the new Magisk unified binary
- Properly handle application install / uninstall root management issues
- Add multiuser mode support
- Add application upgrade re-authentication feature
- Add basic integrity check for SafetyNet
- Merged install fragment and status fragment into Magisk fragment
- Fix theme switching glitch
- Update translations

### v4.3.3

- Re-build APK with stable build tools

### v4.3.2

- Improve usage of Github API to support unlimited amount of online repos
- Update translations (thanks to all contributors!!)

### v4.3.1
- Update proper Magisk busybox detection, will not be confused by busybox installed by default in custom roms

### v4.3.0

- Add Core Only Mode option
- Fix crashes when selecting release note on Samsung devices
- Hide modules using template lower than version 3

### v4.2.7

- Update translations
- Update uninstall scripts

### v4.2.6

- Samsung crashes finally fixed (confirmed!)
- Add settings to disable update notifications
- Adjust Dark theme colors
- Refined download section, now support download only when root is not detected
- Fix crashes in boot image selection

### v4.2

- Change Repo cache to database
- Dark theme refined
- Alert Dialog buttons now properly aligned
- Support very large online modules' zip processing
- You can now download online modules without installing
- Add notifications when new Magisk version is available
- Removed changelog, donation link, support link in download cards
- Read and display README.md for online modules

### v4.1

- Change MagiskHide startup
- Reduce static data (= less memory leaks/issues)
- Translation updates

### v4.0

- Whole new Superuser section for MagiskSU management!
- Add Superuser tab in Logs section
- Add lots of Superuser settings
- Handle MagiskSU requests, logging, notifications
- Controls MagiskHide initialization
- Add disable button
- Add uninstall button
- Tons of improvements, not practical to list all :)

### v3.1

- Fix online repo inaccessible issue
- Fix repo list card expanding issues
- Change SafetyNet check to manually triggered
- Update translations
- Tons of bug fixes preventing potential crashes

### v3.0

- Now on Play Store
- Add Status Section, you can check Safety Net, root status, and Magisk status in one place
- Add Install Section, you can manually choose the boot image location and advanced options

### v2.5

- Add Magisk Hide section, you can now add/remove apps from Magisk Hide list
- Support custom Magisk Version names, any string is now accepted (for custom builds)
- Fixed modules and repos not sorted by name

### v2.1

- Add Magisk Hide settings
- Add search bar in "Downloads Sections"
- Fix crashes when no root is available
- Fix trash can icon not updated when removing module
- Prevent crash when Magisk Version is set incorrectly

### v2.0

- Massive refactor
- Material Design
- Module Management
- Download Section
- And much more....

### v1.0

- Initial release
