# AGENTS.md (native subproject)

Guidelines for AI models operating inside the `native/` subproject.

## 1. Environment & Build Requirements

- **Working Directory:** Execute commands from repo root via `./build.py`.
- **Pre-build Requirement:** ALWAYS run `./build.py native` at least once before editing native sources to generate FFI bindings and headers (`flags.h`, `flags.rs`, generated headers/sources).

## 2. Component Architecture

Native C, C++, and Rust source modules under `native/src/`:
- **`base/`**: System wrappers, logging, and common utilities (C++/Rust).
- **`boot/`**: Boot image parsing, unpacking, and patching logic (Rust).
- **`core/`**: Magisk daemon, Zygisk engine, `su`, applets, system properties (C++/Rust).
- **`init/`**: Early boot `magiskinit`, ramdisk patching, SELinux pre-init (C/C++/Rust).
- **`sepolicy/`**: SELinux policy engine and `magiskpolicy` CLI (C++/Rust).
- **`external/`**: Embedded dependencies (`cxx-rs`, `selinux`, `crt0`, `system_properties`, `lz4-sys`).

## 3. Build Targets & Commands (from Root)

- **Build All Binaries:** `./build.py native`
- **Build Specific Target(s):** `./build.py native [magisk|magiskinit|magiskboot|magiskpolicy|resetprop]`
- **Rust Clippy Lint:** `./build.py clippy`
- **Cargo Commands:** `./build.py cargo check`, `./build.py cargo test`
- **Generate IDE Database:** `./build.py gen`
- **Clean Native Artifacts:** `./build.py clean native`

## 4. Rust & C++ Conventions

- **Clippy Rules:** `unwrap_used = "deny"` in workspace. **Do NOT use `.unwrap()` in Rust code.** Use `?`, `unwrap_or`, or pattern matching.
- **FFI & Toolchain:** C++/Rust FFI uses `cxx` (auto-generated headers). Rust Edition 2024. Configured with `panic = "immediate-abort"`.
