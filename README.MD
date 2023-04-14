![](docs/images/logo.png)

#### This is not an officially supported Google product and is not endorsed by topjohnwu or HuskyDG.

## Introduction

Some highlight features:

- **MagiskSU**: Provide root access for applications
- **Magisk Modules**: Modify read-only partitions by installing modules
- **MagiskBoot**: The most complete tool for unpacking and repacking Android boot images
- **Zygisk**: Run code in every Android applications' processes

Added with [Magisk 32-bit](https://github.com/Namelesswonder/Magisk/tree/32bit) is:

- **Zygote64_32**: Enables running the secondary Zygote process to allow 32-bit applications to start

Further added with [Magisk Delta 32-bit](https://github.com/Namelesswonder/Magisk/tree/delta-32bit) is:

- **early-mount.d**: Mount files before the init process is ran
- **init.rc injection**: Inject rc scripts without having to rebuild the initrd
- **Systemless file removal**: Remove file and folders systemlessly
- **SuList**: Hides Magisk binaries by default and only whitelisted processes can use Magisk
- **Maru**: Zygisk implementation using Native Bridge

Currently only the Pixel 7 and Pixel 7 Pro are officially supported by this project.

Currently only Magisk 32-bit has full support, with Magisk Delta 32-bit being offered _as-is_.

## Sources

The sources for [Magisk 32-bit are available on the page you are reading this from right now](https://github.com/Namelesswonder/Magisk/tree/32bit).

The sources for [Magisk Delta 32-bit are available in the delta-32bit branch](https://github.com/Namelesswonder/Magisk/tree/delta-32bit).

## Downloads

[My GitHub download repository](https://github.com/Namelesswonder/magisk-files) has the downloads of the Magisk app for both Magisk 32-bit and Magisk Delta 32-bit.

**NOTE: Magisk Delta 32-bit is offered _as-is_, as I do not run it daily.**

[![](https://img.shields.io/badge/Magisk%2032bit-25210-blue)](https://raw.githubusercontent.com/Namelesswonder/magisk-files/main/magisk-32bit-release.apk)
[![](https://img.shields.io/badge/Magisk%20Delta%2032bit-25206-red)](https://raw.githubusercontent.com/Namelesswonder/magisk-files/main/magisk-delta-32bit-release.apk)

## Installation

- [**Recommended instructions are on the XDA thread**](https://forum.xda-developers.com/t/4521029/)

## Useful Links

- [XDA-Developers Post](https://forum.xda-developers.com/t/4521029/)

## Bug Reports

**Only bug reports from Magisk 32-bit builds will be accepted, as I am not running Magisk Delta 32-bit**

For installation issues, upload both boot image and install logs.<br>
For bootloops or Magisk issues, upload boot logcat or dmesg.<br>

## Building and Development

- Magisk builds on any OS Android Studio supports. Install Android Studio and do the initial setups.
- Clone sources: `git clone --recurse-submodules https://github.com/Namelesswonder/Magisk.git`
- Switch branch to appropriate version: `git switch 32bit` or `git switch delta-32bit`
- Install Python 3.8+ \
  (Windows only: select **'Add Python to PATH'** in installer, and run `pip install colorama` after install)
- Configure to use the JDK bundled in Android Studio:
  - macOS: `export JAVA_HOME="/Applications/Android Studio.app/Contents/jre/Contents/Home"`
  - Linux: `export PATH="/path/to/androidstudio/jre/bin:$PATH"`
  - Windows: Add `C:\Path\To\Android Studio\jre\bin` to environment variable `PATH`
- Set environment variable `ANDROID_SDK_ROOT` to the Android SDK folder (can be found in Android Studio settings)
- Run `./build.py ndk` to let the script download and install NDK for you
- To start building, run `build.py` to see your options. \
  For each action, use `-h` to access help (e.g. `./build.py all -h`)
- To start development, open the project with Android Studio. The IDE can be used for both app (Kotlin/Java) and native sources.
- Optionally, set custom configs with `config.prop`. A sample `config.prop.sample` is provided.

## Signing and Distribution

- The certificate of the key used to sign the final Magisk APK product is also directly embedded into some executables. In release builds, Magisk's root daemon will enforce this certificate check and reject and forcefully uninstall any non-matching Magisk apps to protect users from malicious and unverified Magisk APKs.
- To do any development on Magisk itself, switch to an **official debug build and reinstall Magisk** to bypass the signature check.
- To distribute your own Magisk builds signed with your own keys, set your signing configs in `config.prop`.
- Check [Google's Documentation](https://developer.android.com/studio/publish/app-signing.html#generate-key) for more details on generating your own key.

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
