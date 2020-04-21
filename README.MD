![](docs/images/logo.png)

![ZIP Downloads](https://img.shields.io/badge/dynamic/json?color=blue&label=ZIP%20Downloads&query=magisk&url=https%3A%2F%2Fraw.githubusercontent.com%2Ftopjohnwu%2Fmagisk_files%2Fcount%2Fcount.json&cacheSeconds=1800)
![APK Downloads](https://img.shields.io/badge/dynamic/json?color=green&label=APK%20Downloads&query=manager&url=https%3A%2F%2Fraw.githubusercontent.com%2Ftopjohnwu%2Fmagisk_files%2Fcount%2Fcount.json&cacheSeconds=1800)

## Introduction

Magisk is a suite of open source tools for customizing Android, supporting devices higher than Android 4.2. It covers fundamental parts of Android customization: root, boot scripts, SELinux patches, AVB2.0 / dm-verity / forceencrypt removals etc.

Here are some feature highlights:

- **MagiskSU**: Provide root access to your device
- **Magisk Modules**: Modify read-only partitions by installing modules
- **MagiskHide**: Hide Magisk from root detections / system integrity checks

## Download

[![](https://img.shields.io/badge/Magisk%20Manager-v7.5.1-green)](https://github.com/topjohnwu/Magisk/releases/download/manager-v7.5.1/MagiskManager-v7.5.1.apk)
[![](https://img.shields.io/badge/Magisk-v20.4-blue)](https://github.com/topjohnwu/Magisk/releases/download/v20.4/Magisk-v20.4.zip)
[![](https://img.shields.io/badge/Magisk%20Beta-v20.4-blue)](https://github.com/topjohnwu/Magisk/releases/download/v20.4/Magisk-v20.4.zip)
<br>
[![](https://img.shields.io/badge/Magisk%20Manager-Canary-red)](https://raw.githubusercontent.com/topjohnwu/magisk_files/canary/app-release.apk)
[![](https://img.shields.io/badge/Magisk%20Manager-Canary%20Debug-red)](https://raw.githubusercontent.com/topjohnwu/magisk_files/canary/app-debug.apk)

## Useful Links

- [Installation Instruction](https://topjohnwu.github.io/Magisk/install.html)
- [OTA Upgrade Guide](https://topjohnwu.github.io/Magisk/ota.html)
- [Full Official Docs](https://topjohnwu.github.io/Magisk/)
- [Magisk Troubleshoot Wiki](https://www.didgeridoohan.com/magisk/HomePage) (by [@Didgeridoohan](https://github.com/Didgeridoohan))

## Android Version Support

- Android 4.2+: MagiskSU and Magisk Modules Only
- Android 4.4+: All core features available
- Android 6.0+: Guaranteed MagiskHide support
- Android 7.0+: Full MagiskHide protection
- Android 9.0+: Magisk Manager stealth mode

## Bug Reports

Canary Channels are cutting edge builds for those adventurous. To access canary builds, install either Canary Magisk Manager, switch to a Canary Channel in settings and upgrade.

**Only bug reports from Canary DEBUG builds will be accepted.**

For installation issues, upload both boot image and install logs.<br>
For Magisk issues, upload boot logcat or dmesg.<br>
For Magisk Manager crashes, record and upload the logcat when the crash occurs.

## Building and Development

- Magisk builds on any OS Android Studio supports. Install Android Studio and do the initial setups.
- Clone sources: `git clone --recurse-submodules https://github.com/topjohnwu/Magisk.git`
- Install Python 3.6+. For Windows, select 'Add Python to PATH' in installer, and run `pip install colorama` after install.
- Configure to use the JDK bundled in Android Studio:
	- macOS: `export JAVA_HOME="/Applications/Android Studio.app/Contents/jre/jdk/Contents/Home"`
	- Linux: `export PATH="/path/to/androidstudio/jre/bin:$PATH"`
	- Windows: Add `C:\Path\To\Android Studio\jre\bin` to environment variable `PATH`
- Set environment variable `ANDROID_HOME` to the Android SDK folder (can be found in Android Studio settings)
- Run `./build.py ndk` to let the script download and install NDK for you
- Set configurations in `config.prop`. A sample `config.prop.sample` is provided.
- To start building, run `build.py` to see your options. For each action, use `-h` to access help (e.g. `./build.py all -h`)
- To start development, open the project in Android Studio. Both app (Kotlin/Java) and native (C++/C) source code can be properly developed using the IDE, but *always* use `build.py` for building.
- `build.py` builds in debug mode by default. If you want release builds (with `-r, --release`), you need a Java Keystore to sign APKs and zips. For more information, check [Google's Documentation](https://developer.android.com/studio/publish/app-signing.html#generate-key).

## Translation Contributions

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
