# Magisk Documentations
(Updated on 2017.8.16) ([Changelog](changelog.md))

## Table of Content

- [Introduction](#introduction)
- [Procedure Diagram](https://cdn.rawgit.com/topjohnwu/Magisk/cc1809688299f1f8b5db494a234850852712c0c9/docs/procedures.html)
- [Magisk Details](details.md)    
    - [Boot Stages](details.md#boot-stages)
    - [Magic Mount Details](details.md#magic-mount-details)
    - [Simple Mount Details](details.md#simple-mount-details)
- [Available Applets](applets.md)
    - [magisk](applets.md#magisk)
    - [su](applets.md#su)
    - [resetprop](applets.md#resetprop)
    - [magiskpolicy](applets.md#magiskpolicy)
    - [magiskhide](applets.md#magiskhide)
- [Modules and Repos](module_repo.md)
    - [Modules and Templates](module_repo.md#magisk-module-format)
    - [Submit Modules to Repo](module_repo.md#submit-your-module-to-magisk-modules-repo)
- [Tips and Tricks](tips.md)
    - [Remove Files](tips.md#remove-files)
    - [Remove Folders](tips.md#remove-folders)

    
## Introduction
Magisk is a suite of open source tools for devices running Android version higher than 5.0 Lollipop (API 21). It establishes an environment which covers most Android aftermarket customization needs, such as root, boot scripts, SELinux patches, dm-verity/forceencrypt patches etc.. Furthermore, Magisk also provides a **Systemless Interface** to alter the system (or vendor) arbitrarily while the actual partitions stay completely intact, all of which accomplished by only patching the boot image, and adding some files into `/data`. With its systemless nature along with several other hacks, Magisk can hide modifications from device integrity verifications, one of the main target is to hide from [Google's SafetyNet API](https://developer.android.com/training/safetynet/index.html).
