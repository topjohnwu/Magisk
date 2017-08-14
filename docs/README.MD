# Magisk Documentations
#### (Updated on 2017.8.15 for Magisk v13.5+)

- Magisk Details
    - [Introduction](#introduction)
    - **(Important!)** [Procedure Diagram](https://cdn.rawgit.com/topjohnwu/Magisk/cc1809688299f1f8b5db494a234850852712c0c9/docs/procedures.html)
    - [Available Applets](applets.md)
        - [magisk](applets.md#magisk)
        - [su](applets.md#su)
        - [resetprop](applets.md#resetprop)
        - [magiskpolicy](applets.md#magiskpolicy)
        - [magiskhide](applets.md#magiskhide)
- [Modules and Repos](module_repo.md)
    - [Modules and Templates](module_repo.md#magisk-module-format)
    - [Submit Modules to Repo](module_repo.md#submit-your-module-to-magisk-modules-repo)

    
## Introduction
Magisk is a suite of open source tools for devices running Android version higher than 5.0 Lollipop (API 21). It provides an environment which covers most Android aftermarket customization like root, boot scripts, SELinux patches, and also acts as a **Systemless Interface** to alter the system (or vendor) arbitrarily while the actual partitions stay completely intact, all of which accomplished by only patching the boot image, and adding some files into `/data`.

## Document Changelog
- 2017.8.15
    - Initial version


