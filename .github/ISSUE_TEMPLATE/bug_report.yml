name: Bug report
description: Create a report to help us improve
body:
  - type: markdown
    attributes:
      value: |
        ## READ BEFORE OPENING ISSUES
        All bug reports require you to **USE CANARY BUILDS**. Please include the version name and version code in the bug report.
        If you experience a bootloop, attach a `dmesg` (kernel logs) when the device refuse to boot. This may very likely require a custom kernel on some devices as            `last_kmsg` or `pstore ramoops` are usually not enabled by default. In addition, please also upload the result of `cat /proc/mounts` when your device is working          correctly **WITHOUT ROOT**.
        If you experience issues during installation, in recovery, upload the recovery logs, or in Magisk, upload the install logs. Please also upload the `boot.img` or        `recovery.img` that you are using for patching.
        If you experience a crash of Magisk app, dump the full `logcat` **when the crash happens**.
        If you experience other issues related to Magisk, upload `magisk.log`, and preferably also include a boot `logcat` (start dumping `logcat` when the device boots         up)
        **DO NOT** open issues regarding root detection.
        **DO NOT** ask for instructions.
        **DO NOT** report issues if you have any modules installed.
        Without following the rules above, your issue will be closed without explanation.


 - type: input
    id: device_model
    attributes:
      label: Device model
      description: What is your device model? This can usually be found in your device's settings in the "About" section.
      placeholder: (e.g. `OnePlus 8 (IN2010)`, or `SM-G960UZKABST`, etc.)
- type: input
    id: Device
    attributes:
      label: Device
      description: Enter the Device model including the name of the manufacturer
    validations:
      required: true
- type: dropdown
  id: android_version
  attributes:
    label: Android version
    multiple: false
    description: What version of Android are you running?
    options:
      - 12
      - 11
      - 10
      - 9
      - 8.1
      - 8.0
      - 7.1
      - 7.0
      - 6
      - 5.1
      - 5.0
      - 4.4
      - 4.3
      - 4.2
      - 4.1
      - 4.0
  validations:
    required: true
    
Magisk version name: 
Magisk version code: 

