# AGENTS.md

Guidelines and instructions for AI models and automated agents operating in the Magisk repository.

## 1. Core Guidelines for AI Models

1. **Git Control:** NEVER commit changes or amend an existing git commit without the user's explicit request or approval. Refer to the [`magisk-git`](.agents/skills/magisk-git/SKILL.md) skill for commit policies and formatting rules.
2. **Environment & Tool Invocations:** `./build.py` configures the build environment internally. Standalone tool executions (such as `./gradlew`, `cargo`, `rustc`, `ndk-build`, etc.) **MUST** be prefixed with `scripts/env.py` (e.g., `scripts/env.py ./gradlew assembleDebug`).
3. **Subproject Contexts:** Activate and follow subproject skills when working on specific codebases:
   - `native/`: Refer to the [`magisk-native`](.agents/skills/magisk-native/SKILL.md) skill for architecture, FFI bindings, Rust conventions, and native workflows.
   - `app/`: Refer to the [`magisk-app`](.agents/skills/magisk-app/SKILL.md) skill for app architecture, Compose/Kotlin conventions, and Gradle workflows.
4. **Verification Loop:** After making changes, verify compilation and run linting/checks for affected modules before concluding tasks.

## 2. Repository Structure

- **`app/`**: Android application multi-module Gradle project.
- **`native/`**: C, C++, and Rust native binaries and libraries (`magisk`, `magiskinit`, `magiskboot`, `magiskpolicy`, `resetprop`).
- **`build.py`**: Primary build orchestrator managing toolchains, native builds, and app packaging.
- **`tools/` & `scripts/`**: Helper utilities (e.g., `elf-cleaner`) and environment setup (`scripts/env.py`).

## 3. Global Build Commands

- **Build Complete Project:** `./build.py all`
- **Clean All Artifacts:** `./build.py clean`
- **NDK Toolchain Setup:** `./build.py ndk`
