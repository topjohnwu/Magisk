![](docs/images/logo.png)

[![下载](https://img.shields.io/badge/dynamic/json?color=green&label=Downloads&query=totalString&url=https%3A%2F%2Fraw.githubusercontent.com%2Ftopjohnwu%2Fmagisk-files%2Fcount%2Fcount.json&cacheSeconds=1800)](https://raw.githubusercontent.com/topjohnwu/magisk-files/count/count.json)

#### 这不是一个谷歌官方支持的产品

## 介绍

Magisk 是一套用于定制化 Android 的开源软件，支持高于Android 6.0 的设备。<br>

一些亮点功能:

- **MagiskSU**: 为应用程序提供 ROOT 访问权限
- **Magisk模块**: 通过安装模块的方式修改只读分区
- **MagiskBoot**: 用于解包、重新打包 Android 启动镜像的最完整工具
- **Zygisk**: 使代码运行在所有 Android 程序的进程中

## 下载

[Github](https://github.com/topjohnwu/Magisk/) 是您可以获得 Magisk 官方信息和下载官方 Magisk 的唯一来源。

[![](https://img.shields.io/badge/Magisk-v26.1-blue)](https://github.com/topjohnwu/Magisk/releases/tag/v26.1)
[![](https://img.shields.io/badge/Magisk%20Beta-v26.1-blue)](https://github.com/topjohnwu/Magisk/releases/tag/v26.1)
[![](https://img.shields.io/badge/Magisk-Canary-red)](https://raw.githubusercontent.com/topjohnwu/magisk-files/canary/app-release.apk)
[![](https://img.shields.io/badge/Magisk-Debug-red)](https://raw.githubusercontent.com/topjohnwu/magisk-files/canary/app-debug.apk)

## 有用的链接

- [安装说明](https://topjohnwu.github.io/Magisk/install.html)
- [构建与开发](https://topjohnwu.github.io/Magisk/build.html)
- [Magisk文件](https://topjohnwu.github.io/Magisk/)

## 漏洞（Bug）报告

**只接受构建于调试（Debug）版本生成的漏洞（Bug）报告**

对于安装问题，请同时上传启动镜像和安装日志。<br>
对于Magisk问题，请上传 boot logcat 或者是 dmesg 。<br>
对于Magisk应用闪退，请记录并上传闪退时的日志。

## 翻译贡献

Magisk 应用及其 APK 的默认根字符串资源位于此处：

- `app/src/main/res/values/strings.xml`
- `stub/src/main/res/values/strings.xml`

翻译每个信息并将其放入各自对应的位置 (`[module]/src/main/res/values-[lang]/strings.xml`).

## 许可证

    Magisk，包括其所有的 git 子模块都是免费软件：
    您可以根据自由软件基金会发布的GNU通用公共许可证条款第3版或由您自由选择的任何更高版本对其进行重新分发、修改或同时分发修改。

    分发此程序是希望它能有作用，但这没有任何的保障，甚至没有对适销性或特定用途适用性的隐含保证。有关更多详细信息，请参阅GNU通用公共许可证。

    您应当已经收到一份GNU通用公共许可证的副本以及这个程序。如果没有的话，详见<http://www.gnu.org/licenses/>.
