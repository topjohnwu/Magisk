---
name: magisk-app
description: Guidelines, architecture, development conventions, and build workflows for the Magisk Android application (app/ subproject). Use when working on, modifying, building, or testing code in app/ or any of its submodules (:core, :apk, :shared, :stub, :stub-res, :test, :build-logic, :apk-legacy).
---

# Magisk App Subproject Guidelines

Guidelines and workflows for developing and building the Android application in the `app/` subproject.

## 1. Environment & Gradle Setup

- **General Guidelines:** Always follow the top-level [`AGENTS.md`](../../../AGENTS.md) and [`magisk-git`](../magisk-git/SKILL.md) for repository rules, environment execution setup, and commit control policies.
- **Working Directory:** Set working directory to `app/` when running standalone Gradle commands, or execute build commands via `./build.py` from the repository root.
- **Environment Wrapper:** Standalone `./gradlew` commands MUST be prefixed with `../scripts/env.py` (e.g., `../scripts/env.py ./gradlew :apk:assembleDebug`). Alternatively, run `./build.py app` from the repository root (which configures the environment automatically and builds `:apk`).

## 2. Architecture & Submodules

Multi-module Gradle project structure:
- **`:apk`** (`apk/`): Main Magisk app (modern Jetpack Compose UI). Primary target for UI and application features.
- **`:core`** (`core/`): Core domain logic, resources, Room DB, services. Primary target for core feature development.
- **`:shared`** (`shared/`): Shared utilities and common data structures.
- **`:stub`** (`stub/`): Lightweight stub app loader for hidden installs.
- **`:stub-res`** (`stub-res/`): Stub-specific Android resources.
- **`:test`** (`test/`): Application testing target.
- **`:build-logic`** (`build-logic/`): Custom Gradle plugins and build logic.
- **`:apk-legacy`** (`apk-legacy/`): Legacy app APK (View-based UI). **Maintenance mode:** Do NOT modify unless explicitly requested.

## 3. Development Guidelines

- **Target Modules:** All active app development occurs in `:core` and `:apk`. Do NOT make changes to `:apk-legacy` unless explicitly requested.
- **Legacy Compatibility:** When modifying `:core` and `:shared`, make sure the `:apk-legacy` module is still buildable.
- **Language & UI:** Written in Kotlin/Java. **Prefer Kotlin for all new code.** Uses Jetpack Compose for UI (prefer over View XML).
- **String Resources:** Default strings in `core/src/main/res/values/strings.xml` and `stub-res/src/main/res/values/strings.xml`. Translations go in `values-[lang]/strings.xml`.
- **Data Stack:** Room, KSP, Wire (Protocol Buffers), Moshi.

## 4. Workflows & Verification

### From `app/` directory (prefixed with `../scripts/env.py`):
- **Build App (`:apk` Debug):** `../scripts/env.py ./gradlew :apk:assembleDebug`
- **Build Stub APK:** `../scripts/env.py ./gradlew :stub:assembleDebug`
- **Build Legacy APK (`:apk-legacy` Debug):** `../scripts/env.py ./gradlew :apk-legacy:assembleDebug`
- **Run Lint:** `../scripts/env.py ./gradlew lint` (or `../scripts/env.py ./gradlew :apk:lintDebug`)
- **Run Unit Tests:** `../scripts/env.py ./gradlew test`
- **Clean Artifacts:** `../scripts/env.py ./gradlew clean`

### From repository root (via `build.py`):
- **Build App (`:apk`):** `./build.py app`
- **Build Stub App:** `./build.py stub`
- **Build Legacy App (`:apk-legacy`):** `./build.py app-legacy`
- **Run App Tests:** `./build.py test`
