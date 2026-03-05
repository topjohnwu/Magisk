#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$ROOT_DIR"

WITH_AVD=1
WITH_CUTTLEFISH=0
SKIP_SETUP=0
AVD_64_API="${AVD_64_API:-34}"
AVD_32_API="${AVD_32_API:-30}"
CF_BRANCH="${CF_BRANCH:-aosp-android-latest-release}"
CF_DEVICE="${CF_DEVICE:-aosp_cf_x86_64_only_phone}"
CF_HOME_DEFAULT="$HOME/aosp_cf_phone"
CF_HOME="${CF_HOME:-$CF_HOME_DEFAULT}"

usage() {
  cat <<'EOF'
Usage: ./run_all_linux.sh [options]

Options:
  --no-avd             Skip AVD tests
  --with-cuttlefish    Run Cuttlefish tests too
  --skip-setup         Skip SDK/NDK setup
  --help               Show this help

Env overrides:
  AVD_64_API=34
  AVD_32_API=30
  CF_HOME=$HOME/aosp_cf_phone
  CF_BRANCH=aosp-android-latest-release
  CF_DEVICE=aosp_cf_x86_64_only_phone
EOF
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --no-avd)
      WITH_AVD=0
      shift
      ;;
    --with-cuttlefish)
      WITH_CUTTLEFISH=1
      shift
      ;;
    --skip-setup)
      SKIP_SETUP=1
      shift
      ;;
    --help|-h)
      usage
      exit 0
      ;;
    *)
      echo "[ERROR] Unknown argument: $1" >&2
      usage
      exit 1
      ;;
  esac
done

if [[ ! -f build.py ]]; then
  echo "[ERROR] build.py not found. Run this script from Magisk repo root." >&2
  exit 1
fi

if ! command -v python3 >/dev/null 2>&1; then
  echo "[ERROR] python3 not found in PATH." >&2
  exit 1
fi

if [[ "$SKIP_SETUP" -eq 0 ]]; then
  if [[ -z "${ANDROID_HOME:-}" && -n "${ANDROID_SDK_ROOT:-}" ]]; then
    export ANDROID_HOME="$ANDROID_SDK_ROOT"
  fi
  if [[ -z "${ANDROID_HOME:-}" ]]; then
    export ANDROID_HOME="$HOME/Android/Sdk"
  fi
  export ANDROID_SDK_ROOT="$ANDROID_HOME"
  export PATH="$ANDROID_HOME/platform-tools:$ANDROID_HOME/emulator:$ANDROID_HOME/cmdline-tools/latest/bin:$PATH"

  if [[ -x "$ANDROID_HOME/cmdline-tools/latest/bin/sdkmanager" ]]; then
    SDKMANAGER="$ANDROID_HOME/cmdline-tools/latest/bin/sdkmanager"
  elif command -v sdkmanager >/dev/null 2>&1; then
    SDKMANAGER="$(command -v sdkmanager)"
  else
    echo "[ERROR] sdkmanager not found. Install Android command-line tools (latest)." >&2
    exit 1
  fi

  echo "[1/10] Setup Android SDK licenses"
  yes | "$SDKMANAGER" --licenses >/dev/null || true

  echo "[2/10] Setup Android SDK packages"
  "$SDKMANAGER" --channel=3 "platform-tools" "emulator" "cmdline-tools;latest"

  echo "[3/10] Setup NDK (CI: build.py -v ndk)"
  python3 build.py -v ndk
else
  echo "[1/10] Setup skipped (--skip-setup)"
  echo "[2/10] Setup skipped (--skip-setup)"
  echo "[3/10] Setup skipped (--skip-setup)"
fi

echo "[4/10] Build release (CI: build.py -vr all)"
python3 build.py -vr all

echo "[5/10] Build debug (CI: build.py -v all)"
python3 build.py -v all

echo "[6/10] Stop Gradle daemon"
./app/gradlew --stop || true

echo "[7/10] Test-build profile (CI: build.py -v -c .github/ci.prop all)"
python3 build.py -v -c .github/ci.prop all

echo "[8/10] Stop Gradle daemon"
./app/gradlew --stop || true

if [[ "$WITH_AVD" -eq 1 ]]; then
  echo "[9/10] AVD test x86_64 API ${AVD_64_API}"
  scripts/avd.sh test "$AVD_64_API"

  echo "[10/10] AVD test x86 API ${AVD_32_API}"
  FORCE_32_BIT=1 scripts/avd.sh test "$AVD_32_API"
else
  echo "[9/10] AVD tests skipped (--no-avd)"
  echo "[10/10] AVD tests skipped (--no-avd)"
fi

if [[ "$WITH_CUTTLEFISH" -eq 1 ]]; then
  echo "[CF] Running Cuttlefish setup + download + test"
  export CF_HOME
  scripts/cuttlefish.sh setup
  scripts/cuttlefish.sh download "$CF_BRANCH" "$CF_DEVICE"
  scripts/cuttlefish.sh test
fi

echo
echo "[OK] Local Linux pipeline completed."
