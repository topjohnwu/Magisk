# Native Development

## Prerequisite

Install the NDK required to build and develop Magisk with `./build.py ndk`. The NDK will be installed to `$ANDROID_SDK_ROOT/ndk/magisk`. You don't need to manually install a Rust toolchain with `rustup`, as the NDK installed already has a Rust toolchain bundled.

## Code Paths

- `jni`: Magisk's code in C++
- `jni/external`: external dependencies, mostly submodules
- `rust`: Magisk's code in Rust
- `src`: irrelevant, only exists to setup a native Android Studio project

## Build Configs

All C/C++ code and its dependencies are built with [`ndk-build`](https://developer.android.com/ndk/guides/ndk-build) and configured with several `*.mk` files scatterred in many places.

The `rust` folder is a proper Cargo workspace, and all Rust code is built with `cargo` just like any other Rust projects.

## Rust + C/C++

To reduce complexity involved in linking, all Rust code is built as `staticlib` and linked to C++ targets to ensure our final product is built with an officially supported NDK build system. Each C++ target can at most link to **one** Rust `staticlib` or else multiple definitions error will occur.

We use the [`cxx`](https://cxx.rs) project for interop between Rust and C++. Although cxx supports interop in both directions, for Magisk, it is strongly advised to avoid calling C++ functions in Rust; if some functionality required in Rust is already implemented in C++, the desired solution is to port the C++ implementation into Rust and export the migrated function back to C++.

## Development / IDE

All C++ code should be recognized and properly indexed by Android Studio out of the box. For Rust:

- Install the [Rust plugin](https://www.jetbrains.com/rust/) in Android Studio
- In Preferences > Languages & Frameworks > Rust, set `$ANDROID_SDK_ROOT/ndk/magisk/toolchains/rust/bin` as the toolchain location
- Open `native/rust/Cargo.toml`, and select "Attach" in the "No Cargo projects found" banner

Note: run `./build.py binary` before developing to make sure generated code is created.
