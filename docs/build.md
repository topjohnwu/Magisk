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
- Set environment variable `ANDROID_HOME` to the Android SDK folder. This path can be found in Android Studio settings.
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

- Kotlin, Java, C++, and C code in the project should be supported in Android Studio out of the box. This repository can be directly opened with Android Studio as a project.
- For Rust development, see the next section.
- Before working on any native code, build all native code first with `./build.py binary`, as some generated code is only created during the build process.

### Developing Rust

First, install [rustup](https://www.rust-lang.org/tools/install), the official Rust toolchain manager. The Magisk NDK package [ONDK](https://github.com/topjohnwu/ondk) (the one installed with `./build.py ndk`) bundles a complete Rust toolchain, so _building_ the Magisk project itself does not require any further configuration.

However, if you'd like to work on the Rust codebase, it'll be easier if you link ONDK's Rust toolchain in `rustup` and set it as default so several development tools and IDEs will work properly:

```bash
# Link the ONDK toolchain with the name "magisk"
rustup toolchain link magisk "$ANDROID_HOME/ndk/magisk/toolchains/rust"
# Set magisk as default
rustup default magisk
```

If you plan to use VSCode, you can then install the [rust-analyzer](https://marketplace.visualstudio.com/items?itemName=rust-lang.rust-analyzer) plugin and everything should be good to go. If you plan to use Jetbrain IDEs (e.g. [Rustrover](https://www.jetbrains.com/rust/), or its Rust Plugin), we need some additional setup:

- Install the official nightly toolchain and add some components. We won't actually use the nightly toolchain for anything other than tricking the IDE to cooperate; the magic happens in the wrapper we setup in the next step.

```bash
rustup toolchain install nightly
# Add some components that is also included in ONDK
rustup +nightly component add rust-src clippy
```

- Create a wrapper cargo bin directory to workaround `rustup` limitations

```bash
# We choose ~/.cargo/wrapper here as an example (and a good recommendation)
# Pick any path you like, you just need to use this path in the next step
./build.py rustup ~/.cargo/wrapper
```

- In Settings > Rust > Toolchain location, set this to the path of the wrapper directory we just created.
- The IDE should now be fully functional, and you are able to enable `rustfmt` and use `Clippy` as the external linter.

## Signing and Distribution

- In release builds, the certificate of the key signing the Magisk APK will be used by Magisk's root daemon as a reference to reject and forcefully uninstall any non-matching Magisk apps to protect users from malicious and unverified Magisk APKs.
- To do any development on Magisk itself, switch to an **official debug build and reinstall Magisk** to turn off the signature check.
- To distribute your own Magisk builds signed with your own keys, set your signing configs in `config.prop`.
- Check [Google's Documentation](https://developer.android.com/studio/publish/app-signing.html#generate-key) for more details on generating your own key.
