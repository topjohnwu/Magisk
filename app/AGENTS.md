# AGENTS.md (app subproject)

Guidelines for AI models operating inside the `app/` subproject. Always include and follow the top-level [`AGENTS.md`](../AGENTS.md).

## 1. Environment & Gradle Setup

- **General Guidelines:** Always follow the top-level [`AGENTS.md`](../AGENTS.md) for general repository rules, environment execution setup, and commit control policies.
- **Working Directory:** Set working directory to `app/` when working on app code.
- **Environment Wrapper:** Standalone `./gradlew` commands MUST be prefixed with `../scripts/env.py` (e.g., `../scripts/env.py ./gradlew assembleDebug`), or run `./build.py app` from root.

## 2. Architecture & Submodules

Multi-module Gradle project structure:
- **`:apk`** (`apk/`): Legacy app APK. **Maintenance mode:** No new features should be added here.
- **`:apk-ng`** (`apk-ng/`): Next-gen app variant. Primary target for new UI/app features.
- **`:core`** (`core/`): Core domain logic, resources, Room DB, services. Primary target for core feature development.
- **`:shared`** (`shared/`): Shared utilities and common data structures.
- **`:stub`** (`stub/`): Lightweight stub app loader for hidden installs.
- **`:stub-res`** (`stub-res/`): Stub-specific Android resources.
- **`:test`** (`test/`): Application testing target.
- **`:build-logic`** (`build-logic/`): Custom Gradle plugins and build logic.

## 3. Development Guidelines

- **Feature Development:** `:apk` is in maintenance mode. All new development MUST occur in `:core` and `:apk-ng`.
- **Language & UI:** Written in Kotlin/Java. **Prefer Kotlin for all new code.** Uses Jetpack Compose for UI (prefer over View XML).
- **String Resources:** Default strings in `core/src/main/res/values/strings.xml` and `stub-res/src/main/res/values/strings.xml`. Translations go in `values-[lang]/strings.xml`.
- **Data Stack:** Room, KSP, Wire (Protocol Buffers), Moshi.

## 4. Workflows & Verification (from `app/`)

Prefix commands with `../scripts/env.py`:
- **Build Main APK (Debug):** `../scripts/env.py ./gradlew :apk:assembleDebug`
- **Build All Variants:** `../scripts/env.py ./gradlew assembleDebug`
- **Build Stub APK:** `../scripts/env.py ./gradlew :stub:assembleDebug`
- **Run Lint:** `../scripts/env.py ./gradlew lint`
- **Run Unit Tests:** `../scripts/env.py ./gradlew test`
- **Clean Artifacts:** `../scripts/env.py ./gradlew clean`
