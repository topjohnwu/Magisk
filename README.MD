![](docs/images/logo.png)

[![Downloads](https://img.shields.io/badge/dynamic/json?color=green&label=Downloads&query=totalString&url=https%3A%2F%2Fraw.githubusercontent.com%2Ftopjohnwu%2Fmagisk-files%2Fcount%2Fcount.json&cacheSeconds=1800)](https://raw.githubusercontent.com/topjohnwu/magisk-files/count/count.json)

#### Disclaimer: This is not an officially supported Google product.

## Introduction

Magisk is a set of open-source tools that let you customize Android devices running Android 6.0 or higher.<br>
Key Features::

- **MagiskSU**: Provide root access for applications
- **Magisk Modules**: Modify read-only partitions by installing modules
- **MagiskBoot**: The most complete tool for unpacking and repacking Android boot images
- **Zygisk**: Run code in every Android applications' processes

## Downloads

[Github](https://github.com/topjohnwu/Magisk/) is the only source where you can get official Magisk information and downloads.

[![](https://img.shields.io/badge/Magisk-v26.1-blue)](https://github.com/topjohnwu/Magisk/releases/tag/v26.1)
[![](https://img.shields.io/badge/Magisk%20Beta-v26.2-blue)](https://github.com/topjohnwu/Magisk/releases/tag/v26.2)
[![](https://img.shields.io/badge/Magisk-Canary-red)](https://raw.githubusercontent.com/topjohnwu/magisk-files/canary/app-release.apk)
[![](https://img.shields.io/badge/Magisk-Debug-red)](https://raw.githubusercontent.com/topjohnwu/magisk-files/canary/app-debug.apk)

## Useful Links

- [Installation Instruction](https://topjohnwu.github.io/Magisk/install.html)
- [Building and Development](https://topjohnwu.github.io/Magisk/build.html)
- [Magisk Documentation](https://topjohnwu.github.io/Magisk/)

## Bug Reports

**Please note: We only accept bug reports from Debug builds.**

For installation issues, provide both the boot image and installation logs.<br>
For Magisk issues, include the boot logcat or dmesg.<br>
If the Magisk app crashes, record and upload the logcat when it happens.

## Translation Contributions

Translate strings in the Magisk app and its stub APK located here:

- `app/src/main/res/values/strings.xml`
- `stub/src/main/res/values/strings.xml`

Translate each and Place translations in the respective folders:  (`[module]/src/main/res/values-[lang]/strings.xml`).

## License

    Magisk, including all Git submodules, is distributed as free software under the terms of the 
    GNU General Public License, as published by the Free Software Foundation, 
    either version 3 of the License, or any later version you choose.

    This program is distributed with the hope that it will be beneficial, 
    but it comes with NO WARRANTY, not even the implied warranties of MERCHANTABILITY
    or FITNESS FOR A PARTICULAR PURPOSE. For more details, please refer to the
    GNU General Public License.

    If you have not received a copy of the GNU General Public License, 
    you can find it at http://www.gnu.org/licenses/.


