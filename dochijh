# Building and Development

## Setup Environment

- Supported platforms:
  - Linux x64
  - macOS x64 (Intel)
  - macOS arm64 (Apple Silicon)
  - Windows x64
- Windows only: Enable [developer mode](https://learn.microsoft.com/en-us/windows/apps/get-started/enable-your-device-for-development). This is required because we need symbolic link support.
- Install Python 3.8+:
  - On Unix, install python3 using your favorite package manager
  - On Windows, download and install the latest Python version on the [official website](https://www.python.org/downloads/windows/).<br>
    Make sure to select **"Add Python to PATH"** during installation.
  - (Optional on Windows): Run `pip install colorama` to install the `colorama` python package
- Install Git:
  - On Unix, install git with your favorite package manager
  - On Windows, download the install the latest Git version on the [official website](https://git-scm.com/download/win).<br>
    Make sure to **"Enable symbolic links"** during installation.
- Install Android Studio and follow the instructions and go through the initial setup.
- Set environment variable `ANDROID_SDK_ROOT` to the Android SDK folder. This path can be found in Android Studio settings.
- Setup JDK:
  - The recommended option is to set environment variable `ANDROID_STUDIO` to the path where your Android Studio is installed. The build script will automatically find and use the bundled JDK.
  - You can also setup JDK 17 yourself, but this guide will not cover the instructions.
- Clone sources: `git clone --recurse-submodules https://github.com/topjohnwu/Magisk.git`
- Run `./build.py ndk` to let the script download and install NDK for you

## Building

- To build everything and create the final Magisk APK, run `./build.py all`.
- You can also build specific sub-components; call `build.py` to see your options. \
  For each action, use `-h` to access help (e.g. `./build.py binary -h`)
- Configure the build by using `config.prop`. A sample `config.prop.sample` is provided.

## IDE Support

- The repository can be directly opened with Android Studio as a project.
- The Kotlin, Java, C++, and C code in the project should be properly supported in Android Studio out of the box.
- Run `./build.py binary` before working on native code, as some generated code is only created during the build process.

### Developing Rust in Android Studio

Because the Magisk NDK package, [ONDK](https://github.com/topjohnwu/ondk) (the one installed with `./build.py ndk`), contains a fully self contained Clang + Rust toolchain, building the Magisk project alone does not require configuring toolchains. However, due to the way the IntelliJ Rust plugin works, you'll have to go through some additional setup to make Android Studio work with Magisk's Rust codebase:

- Install [rustup](https://rustup.rs/), the official Rust toolchain manager
- Link the ONDK Rust toolchain and set it as default:

```bash
# Link the ONDK toolchain with the name "magisk"
rustup toolchain link magisk "$ANDROID_SDK_ROOT/ndk/magisk/toolchains/rust"
# Set as default
rustup default magisk
```

- Install the [Intellij Rust plugin](https://www.jetbrains.com/rust/) in Android Studio
- In Preferences > Languages & Frameworks > Rust, set `$ANDROID_SDK_ROOT/ndk/magisk/toolchains/rust/bin` as the toolchain location
- Open `native/src/Cargo.toml`, and select "Attach" in the "No Cargo projects found" banner

## Signing and Distribution

- In release builds, the certificate of the key signing the Magisk APK will be used by Magisk's root daemon as a reference to reject and forcefully uninstall any non-matching Magisk apps to protect users from malicious and unverified Magisk APKs.
- To do any development on Magisk itself, switch to an **official debug build and reinstall Magisk** to turn off the signature check.
- To distribute your own Magisk builds signed with your own keys, set your signing configs in `config.prop`.
- Check [Google's Documentation](https://developer.android.com/studio/publish/app-signing.html#generate-key) for more details on generating your own key.
