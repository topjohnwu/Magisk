# Magisk Documentations
(Updated on 2018.1.8) ([Changelog](changelog.md))

## Table of Contents

- [Introduction](#introduction)
- [Tips and Tricks](tips.md)
    - [OTA Installation Tips](tips.md#ota-installation-tips)

The following are for developers

- [Procedure Diagram](https://cdn.rawgit.com/topjohnwu/Magisk/f927c1b997f5590d86b22217988361325de04745/docs/procedures.html)
- [Magisk Details](details.md)
    - [Boot Stages](details.md#boot-stages)
    - [Magic Mount Details](details.md#magic-mount-details)
    - [Simple Mount Details](details.md#simple-mount-details)
- [Available Tools](tools.md)
    - [magiskboot](tools.md#magiskboot)
    - [magiskinit](tools.md#magiskinit)
    - [magiskpolicy](tools.md#magiskpolicy)
    - [magisk](tools.md#magisk)
    - [su](tools.md#su)
    - [resetprop](tools.md#resetprop)
    - [magiskhide](tools.md#magiskhide)
- [Modules](modules.md)
    - [Modules and Templates](modules.md#magisk-module-format)
    - [Submit Modules to Repo](https://github.com/topjohnwu/Magisk_Repo_Submissions)
- [Tips and Tricks](tips.md)
    - [Remove Files](tips.md#remove-files)
    - [Remove Folders](tips.md#remove-folders)

    
## Introduction
Magisk is a suite of open source tools for devices running Android version higher than 5.0 Lollipop (API 21). It establishes an environment which covers most stuffs you need for Android customization, such as root, boot scripts, SELinux patches, AVB2.0 / dm-verity / forceencrypt patches etc..

Furthermore, Magisk provides a **Systemless Interface** to alter the system (or vendor) arbitrarily while the actual partitions stay completely intact, all of which accomplished by only patching the boot image. With its systemless nature along with several other hacks, Magisk can hide modifications from nearly any existing system integrity verifications, one of the main target is to hide from [Google's SafetyNet API](https://developer.android.com/training/safetynet/index.html).
