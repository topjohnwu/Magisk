## 2020.10.3 Magisk v21.0

Long time no see! v21.0 is the largest release in Magisk's history. It comes with full Android 11 support (tons of stuff had to be rewritten from scratch!), and a completely redesigned Magisk Manager. These are the reasons why this particular public release took me over half a year to wrap up.

To the end user, not much has changed other than the fact that Magisk Manager has completely changed its appearance. However developers should pay attention to some changes due to adjustments for Android 11. Full changelogs are too massive to fit, so here I'll point out the main changes and links to updated documentations.

### Highlights

- Android 11 support 🎉
- Completely redesigned Magisk Manager
- Safe Mode detection: if you installed a module that bootloops your device, reboot into Safe Mode and all modules will be disabled. More instructions on how to deal with broken modules is linked [here](https://topjohnwu.github.io/Magisk/faq.html).

The following are for advanced users/developer:

- On Android 8.0+, Magisk now uses a new SELinux setup that keeps Android sandbox less compromised. This provides better security to rooted users, and also separates Magisk rules from original rules. Details [here](https://topjohnwu.github.io/Magisk/details.html#selinux-policies).
- On Android 11, `/sbin` may no longer exist. For developers, this means the Magisk's internal `tmpfs` directory is no longer always `/sbin`, and instead randomly created every boot. To get the `tmpfs` path, use the command `magisk --path` (more details [here](https://topjohnwu.github.io/Magisk/details.html)). For custom kernel developers that uses `overlay.d`, updated docs are [here](https://topjohnwu.github.io/Magisk/guides.html#root-directory-overlay-system).
- `magiskpolicy` gained more features and some minor syntax changes, details [here](https://topjohnwu.github.io/Magisk/tools.html#magiskpolicy).

### Full Changelog: [here](https://topjohnwu.github.io/Magisk/changes.html)
