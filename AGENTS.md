# AGENTS.md

Guidelines and instructions for AI models and automated agents operating in the Magisk repository.

## 1. Environment & Execution Setup

- **`build.py` Invocations:** `./build.py` imports `scripts/env.py` internally and configures the environment automatically. It does **not** need to be prefixed with `scripts/env.py` (e.g., `./build.py all` or `./build.py native`).
- **Direct Tool Executions:** Standalone commands outside of `build.py` (such as `./gradlew`, `cargo`, `rustc`, `ndk-build`, etc.) **MUST** be prefixed with `scripts/env.py` (e.g., `scripts/env.py ./gradlew assembleDebug`).
- **NDK Toolchain Setup:** If NDK toolchain dependencies are missing or outdated, run `./build.py ndk`.

## 2. Codebase Structure & Architecture

- **`app/` (Android Application):** Multi-module Gradle project for the Android app. Refer to [`app/AGENTS.md`](app/AGENTS.md) for app architecture, submodules, Kotlin conventions, and Gradle workflows.
- **`native/` (Native Core Binaries):** C, C++, and Rust codebase for building native components (`magisk`, `magiskinit`, `magiskboot`, `magiskpolicy`, `resetprop`). Refer to [`native/AGENTS.md`](native/AGENTS.md) for native architecture, submodules, FFI compilation, and Rust conventions.
- **`build.py` (Primary Build Orchestrator):** Python script managing native builds, app packaging, Clippy runs, Cargo invocations, and cleanup.
- **`tools/` & `scripts/`:** Build scripts, helper utilities (e.g., `elf-cleaner`), and environment setup tools.

## 3. Build & Verification Workflows

- **Build Complete Project (APK + Native):** `./build.py all`
- **Build Native Components:** `./build.py native [target...]`
- **Build Application Targets:** `./build.py app`, `./build.py stub`, `./build.py test`
- **Rust Verification & Linting:** `./build.py clippy`, `./build.py cargo <cargo-commands...>`
- **Clean Build Artifacts:** `./build.py clean [native|cpp|rust|java|app]`

## 4. Guidelines for AI Models

1. **Git / Commit Control:** NEVER commit changes or amend an existing git commit without the user's explicit request or approval. When explicitly requested to commit changes, follow the 50/72 rule for commit messages (subject line <= 50 characters, blank line before body, wrap body lines at 72 characters) and include an `Assisted-by: <ModelVersion>` trailer in the commit message body (using a pretty name, e.g., `Assisted-by: Gemini 3.6 Flash`).
2. **Build Invocation:** Execute `./build.py <command>` directly without `scripts/env.py`. Prefix standalone tool executions (like `./gradlew` or raw `cargo`) with `scripts/env.py`.
3. **Pre-build Native Code:** Before modifying code in `native/`, build native binaries at least once with `./build.py native` to generate required bindings and header files.
4. **App Subproject Context:** Refer to [`app/AGENTS.md`](app/AGENTS.md) when working inside the `app/` subproject.
5. **Verification Loop:** After making changes, verify compilation and run linting/clippy checks for affected modules before concluding tasks.
