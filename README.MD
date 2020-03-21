# Magisk

[Downloads](https://github.com/topjohnwu/Magisk/releases) \| [Documentation](https://topjohnwu.github.io/Magisk/) \| [XDA Thread](https://forum.xda-developers.com/apps/magisk/official-magisk-v7-universal-systemless-t3473445)

## Introduction

Magisk is a suite of open source tools for customizing Android, supporting devices higher than Android 4.2 (API 17). It covers the fundamental parts for Android customization: root, boot scripts, SELinux patches, AVB2.0 / dm-verity / forceencrypt removals etc.

Furthermore, Magisk provides a **Systemless Interface** to alter the system (or vendor) arbitrarily while the actual partitions stay completely intact. With its systemless nature along with several other hacks, Magisk can almost perfectly hide modifications within userspace. Note that since 2020.3, the CTS check of [Google's SafetyNet API](https://developer.android.com/training/safetynet/index.html) will **NOT** pass.

## Bug Reports

**Only reports using debug canary builds will be accepted.** \
Access canary builds by upgrading to either canary Magisk Manager:
- [Canary Manager (Release)](https://raw.githubusercontent.com/topjohnwu/magisk_files/canary/app-release.apk)
- [Canary Manager (Debug)](https://raw.githubusercontent.com/topjohnwu/magisk_files/canary/app-debug.apk)

For installation issues, upload both boot image and install logs. \
For Magisk issues, upload boot logcat or dmesg. \
For Magisk Manager crashes, record and upload the logcat when the crash occurs.

## Building Environment Requirements

-  Python 3: run `build.py` script
-  Java Development Kit (JDK) 8: Compile Magisk Manager and sign zips
-  Latest Android SDK: set `ANDROID_HOME` environment variable to the path to Android SDK
-  Android NDK: Install NDK along with SDK (`$ANDROID_HOME/ndk-bundle`), or optionally specify a custom path `ANDROID_NDK_HOME`
-  (Windows Only) Python package Colorama: Install with `pip install colorama`, used for ANSI color codes

## Building Notes and Instructions

-  Clone sources with submodules: `git clone --recurse-submodules https://github.com/topjohnwu/Magisk.git`
-  Building is supported on macOS, Linux, and Windows. Official releases are built and tested with [FrankeNDK](https://github.com/topjohnwu/FrankeNDK); point `ANDROID_NDK_HOME` to FrankeNDK if you want to use it for compiling.
-  Set configurations in `config.prop`. A sample file `config.prop.sample` is provided as an example.
-  Run `build.py` with argument `-h` to see the built-in help message. The `-h` option also works for each supported actions, e.g. `./build.py binary -h`
-  By default, `build.py` build binaries and Magisk Manager in debug mode. If you want to build Magisk Manager in release mode (via the `-r, --release` flag), you need a Java Keystore file `release-key.jks` (only `JKS` format is supported) to sign APKs and zips. For more information, check out [Google's Official Documentation](https://developer.android.com/studio/publish/app-signing.html#signing-manually).

## Translations

Default string resources for Magisk Manager and its stub APK are located here:

- `app/src/main/res/values/strings.xml`
- `stub/src/main/res/values/strings.xml`

Translate each and place them in the respective locations (`[module]/src/main/res/values-[lang]/strings.xml`).

## License

    Magisk, including all git submodules are free software:
    you can redistribute it and/or modify it under the terms of the
    GNU General Public License as published by the Free Software Foundation,
    either version 3 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
