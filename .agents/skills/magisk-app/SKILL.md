---
name: magisk-app
description: Guidelines, architecture, development conventions, and build workflows for the Magisk Android application (app/ subproject). Use when working on, modifying, building, or testing code in app/ or any of its submodules (:core, :apk, :test, :build-logic).
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
- **`:test`** (`test/`): Application testing target.
- **`:build-logic`** (`build-logic/`): Custom Gradle plugins and build logic.

## 3. Development Guidelines

- **Target Modules:** All active app development occurs in `:core` and `:apk`.
- **Language & UI:** Written in Kotlin/Java. **Prefer Kotlin for all new code.** Uses Jetpack Compose for UI (prefer over View XML).
- **String Resources:**
  - Default strings live in `core/src/main/res/values/strings.xml`. Translations live in `values-[lang]/strings.xml`.
  - Whenever updating an existing string in a way that significantly changes its meaning, **always create a new string resource key** (and remove the old key across all `values*/strings.xml` files if unused) so existing translations do not go out of sync; falling back to untranslated default text is preferred over displaying an incorrect translation.
- **Data Stack:** Room, KSP, Wire (Protocol Buffers), Moshi.

## 4. Workflows & Verification

### From `app/` directory (prefixed with `../scripts/env.py`):
- **Build App (`:apk` Debug):** `../scripts/env.py ./gradlew :apk:assembleDebug`
- **Run Lint:** `../scripts/env.py ./gradlew lint` (or `../scripts/env.py ./gradlew :apk:lintDebug`)
- **Run Unit Tests:** `../scripts/env.py ./gradlew test`
- **Clean Artifacts:** `../scripts/env.py ./gradlew clean`

### From repository root (via `build.py`):
- **Build App (`:apk`):** `./build.py app`
- **Run App Tests:** `./build.py test`
