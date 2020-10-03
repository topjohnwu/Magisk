# 2019.11.2 Magisk v20.1
Lots of bug fixes from v20.0, and some cool new features!

### Updated Magisk Manager Hiding
Starting with Magisk v20.1 paired with Magisk Manager v7.4.0, a new hiding mode is introduced for Android 9.0+. On supported devices, Magisk Manager will download and customize a heavily obfuscated stub APK and use it as a replacement. The stub app will then download the full app into its private internal data, then dynamically load and run the actual full Magisk Manager.

Note, not all Android 9.0+ devices will be able to use this feature. To use an obfuscated stub as Magisk Manager, the Magisk daemon will have to rely on a special way to communicate with the app, and some OEMs (most likely Chinese manufacturers) block certain broadcasts, breaking the communication channel.

Magisk Manager will verify compatibility before it uses stubs to hide itself on Android 9.0+. **The verification relies on Magisk v20.1+, which means you have to fully upgrade and reboot in order to opt in this feature.** If you are already running a hidden Magisk Manager, **restore and upgrade Magisk Manager, upgrade Magisk and reboot, then re-hide the app**.

For those incompatible with the hiding-with-stub feature, there are also a few updates that everyone, regardless whether using stubs or not, can enjoy:

- You can now customize the app name of the repackaged Magisk Manager
- Magisk Manager will generate new keys to sign the repackaged APK to prevent signature detection

### Full Changelog: [here](https://forum.xda-developers.com/showpost.php?p=68966755&postcount=2)
