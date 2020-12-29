---
name: Bug report
about: Create a report to help us improve
title: ''
labels: ''
assignees: ''

---

## READ BEFORE OPENING ISSUES

All bug reports require you to **USE CANARY BUILDS**. Please include the version name and version code in the bug report.

If you experience a bootloop, please attach a dmesg (kernel logs) when the device refuse to boot. This may very likely require a custom kernel on some devices as `last_kmsg` or `pstore ramoops` are usually not enabled by default.

If you experience a crash of Magisk Manager, dump the full `logcat` **when the crash happens**. **DO NOT** upload `magisk.log`.

If you experience other issues related to Magisk, upload `magisk.log`, and preferably also include a boot `logcat` (`logcat` starting when the device boots up)

**DO NOT** open issues regarding root detection.

**DO NOT** ask for instructions.

**DO NOT** report issues if you have any modules installed.

Without following the rules above, your issue will be closed without explanation.
