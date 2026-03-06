#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$ROOT_DIR"

WITH_AVD=1
WITH_CUTTLEFISH=1
SKIP_SETUP=0
SKIP_BUILD=0
QUICK_AVD=0

AVD_64_API="${AVD_64_API:-34}"
AVD_32_API="${AVD_32_API:-30}"
CF_BRANCH="${CF_BRANCH:-aosp-android-latest-release}"
CF_DEVICE="${CF_DEVICE:-aosp_cf_x86_64_only_phone}"
CF_HOME_DEFAULT="$HOME/aosp_cf_phone"
CF_HOME="${CF_HOME:-$CF_HOME_DEFAULT}"
CI_OUTDIR="${CI_OUTDIR:-out-ci}"

PYTHON_BIN=
SDKMANAGER=
FAILURES=()

log() {
  echo "[run_all_linux] $*"
}

warn() {
  echo "[run_all_linux] WARN: $*" >&2
}

die() {
  echo "[run_all_linux] ERROR: $*" >&2
  exit 1
}

usage() {
  cat <<'EOF'
Usage: ./run_all_linux.sh [options]

Options:
  --no-avd             Skip AVD tests
  --quick-avd          Run only AVD_64_API and AVD_32_API (not full CI matrix)
  --no-cuttlefish      Skip Cuttlefish test
  --with-cuttlefish    Force Cuttlefish test (default)
  --skip-setup         Skip SDK/NDK setup
  --skip-build         Skip build steps, run only tests
  --help               Show this help

Env overrides:
  AVD_64_API=34
  AVD_32_API=30
  AVD_PATCH_APK=/path/to/magisk.apk
  AVD_PATCH_APK_DEBUG=/path/to/debug.apk
  AVD_PATCH_APK_RELEASE=/path/to/release.apk
  CI_OUTDIR=out-ci
  AVD_MEMORY_MB=4096
  AVD_TEST_LOG=1
  AVD_ALLOW_SOFT_ACCEL=1
  CF_HOME=$HOME/aosp_cf_phone
  CF_BRANCH=aosp-android-latest-release
  CF_DEVICE=aosp_cf_x86_64_only_phone
EOF
}

require_command() {
  local cmd="$1"
  command -v "$cmd" >/dev/null 2>&1 || die "Required command not found: $cmd"
}

require_file() {
  local f="$1"
  [[ -f "$f" ]] || die "Required file missing: $f"
}

prepare_python() {
  if command -v python3 >/dev/null 2>&1; then
    PYTHON_BIN=python3
  elif command -v python >/dev/null 2>&1; then
    PYTHON_BIN=python
  else
    die "Python 3.8+ is required"
  fi
}

prepare_android_env() {
  if [[ -z "${ANDROID_HOME:-}" && -n "${ANDROID_SDK_ROOT:-}" ]]; then
    export ANDROID_HOME="$ANDROID_SDK_ROOT"
  fi
  if [[ -z "${ANDROID_HOME:-}" ]]; then
    export ANDROID_HOME="$HOME/Android/Sdk"
  fi
  export ANDROID_SDK_ROOT="$ANDROID_HOME"
  export PATH="$ANDROID_HOME/platform-tools:$ANDROID_HOME/emulator:$ANDROID_HOME/cmdline-tools/latest/bin:$PATH"

  SDKMANAGER="$ANDROID_HOME/cmdline-tools/latest/bin/sdkmanager"
  [[ -x "$SDKMANAGER" ]] || die "sdkmanager not found at $SDKMANAGER"
}

record_failure() {
  local job="$1"
  FAILURES+=("$job")
}

apk_has_entry() {
  local apk="$1"
  local entry="$2"
  "$PYTHON_BIN" - "$apk" "$entry" <<'PY' >/dev/null 2>&1
import sys, zipfile
apk, entry = sys.argv[1], sys.argv[2]
with zipfile.ZipFile(apk, "r") as zf:
    if entry not in zf.namelist():
        raise SystemExit(1)
PY
}

validate_pipeline_artifacts() {
  require_file out/app-debug.apk
  require_file out/app-release.apk
  require_file out/test.apk

  # AVD/CF patching needs busybox for emulator ABIs.
  local abi
  for abi in x86 x86_64; do
    apk_has_entry out/app-debug.apk "lib/${abi}/libbusybox.so" || \
      die "out/app-debug.apk missing lib/${abi}/libbusybox.so"
    apk_has_entry out/app-release.apk "lib/${abi}/libbusybox.so" || \
      die "out/app-release.apk missing lib/${abi}/libbusybox.so"
  done

  if [[ -n "${AVD_PATCH_APK:-}" ]]; then
    require_file "$AVD_PATCH_APK"
    for abi in x86 x86_64; do
      apk_has_entry "$AVD_PATCH_APK" "lib/${abi}/libbusybox.so" || \
        die "AVD_PATCH_APK missing lib/${abi}/libbusybox.so: $AVD_PATCH_APK"
    done
  fi
  if [[ -n "${AVD_PATCH_APK_DEBUG:-}" ]]; then
    require_file "$AVD_PATCH_APK_DEBUG"
    for abi in x86 x86_64; do
      apk_has_entry "$AVD_PATCH_APK_DEBUG" "lib/${abi}/libbusybox.so" || \
        die "AVD_PATCH_APK_DEBUG missing lib/${abi}/libbusybox.so: $AVD_PATCH_APK_DEBUG"
    done
  fi
  if [[ -n "${AVD_PATCH_APK_RELEASE:-}" ]]; then
    require_file "$AVD_PATCH_APK_RELEASE"
    for abi in x86 x86_64; do
      apk_has_entry "$AVD_PATCH_APK_RELEASE" "lib/${abi}/libbusybox.so" || \
        die "AVD_PATCH_APK_RELEASE missing lib/${abi}/libbusybox.so: $AVD_PATCH_APK_RELEASE"
    done
  fi
}

run_optional_job() {
  local label="$1"
  shift

  log "START: $label"
  if "$@"; then
    log "PASS:  $label"
  else
    warn "FAIL:  $label"
    record_failure "$label"
  fi
}

setup_environment() {
  if [[ "$SKIP_SETUP" -eq 1 ]]; then
    log "Setup skipped (--skip-setup)"
    return
  fi

  log "Accepting Android SDK licenses"
  yes | "$SDKMANAGER" --licenses >/dev/null || true

  log "Installing Android SDK packages"
  "$SDKMANAGER" --channel=3 \
    "platform-tools" \
    "emulator" \
    "cmdline-tools;latest" \
    "platforms;android-36" \
    "build-tools;36.1.0"

  log "Syncing git submodules"
  git submodule update --init --recursive

  log "Setting up Magisk NDK"
  "$PYTHON_BIN" build.py -v ndk
}

run_builds() {
  if [[ "$SKIP_BUILD" -eq 1 ]]; then
    log "Build steps skipped (--skip-build)"
    return
  fi

  log "Build release (CI: ./build.py -vr all)"
  "$PYTHON_BIN" build.py -vr all

  log "Build debug (CI: ./build.py -v all)"
  "$PYTHON_BIN" build.py -v all
  validate_pipeline_artifacts

  log "Stop Gradle daemon"
  bash ./app/gradlew --stop || true

  # Keep CI profile outputs separated so we do not overwrite out/app-debug.apk
  # and out/test.apk that are required by AVD/Cuttlefish tests.
  log "Test build profile (CI: ./build.py -v -c .github/ci.prop all) in $CI_OUTDIR"
  local ci_cfg
  ci_cfg="$(mktemp)"
  cp .github/ci.prop "$ci_cfg"
  echo "outdir=$CI_OUTDIR" >> "$ci_cfg"
  "$PYTHON_BIN" build.py -v -c "$ci_cfg" all
  rm -f "$ci_cfg"
  validate_pipeline_artifacts

  log "Stop Gradle daemon"
  bash ./app/gradlew --stop || true
}

run_avd_job_x64() {
  local version="$1"
  local type="${2:-}"
  if [[ -n "$type" ]]; then
    bash scripts/avd.sh test "$version" "$type"
  else
    bash scripts/avd.sh test "$version"
  fi
}

run_avd_job_x86() {
  local version="$1"
  FORCE_32_BIT=1 bash scripts/avd.sh test "$version"
}

run_avd_matrix() {
  [[ "$WITH_AVD" -eq 1 ]] || {
    log "AVD tests skipped (--no-avd)"
    return
  }

  export AVD_TEST_LOG="${AVD_TEST_LOG:-1}"
  export AVD_PATCH_APK="${AVD_PATCH_APK:-}"
  export AVD_PATCH_APK_DEBUG="${AVD_PATCH_APK_DEBUG:-}"
  export AVD_PATCH_APK_RELEASE="${AVD_PATCH_APK_RELEASE:-}"
  validate_pipeline_artifacts

  if [[ -n "${AVD_ALLOW_SOFT_ACCEL:-}" ]]; then
    warn "Software acceleration enabled (AVD_ALLOW_SOFT_ACCEL=1). Tests may be very slow."
  fi

  if [[ "$QUICK_AVD" -eq 1 ]]; then
    run_optional_job "Test API ${AVD_64_API} (x86_64)" run_avd_job_x64 "$AVD_64_API"
    run_optional_job "Test API ${AVD_32_API} (x86)" run_avd_job_x86 "$AVD_32_API"
    return
  fi

  local x64_versions=(23 24 25 26 27 28 29 30 31 32 33 34 35 36 36.1 CANARY)
  local x86_versions=(23 24 25 26 27 28 29 30)
  local v

  for v in "${x64_versions[@]}"; do
    run_optional_job "Test API ${v} (x86_64)" run_avd_job_x64 "$v"
  done
  run_optional_job "Test API CinnamonBun (x86_64)" run_avd_job_x64 "CinnamonBun" "google_apis_ps16k"

  for v in "${x86_versions[@]}"; do
    run_optional_job "Test API ${v} (x86)" run_avd_job_x86 "$v"
  done
}

run_cuttlefish_job() {
  export CF_HOME
  bash scripts/cuttlefish.sh setup
  bash scripts/cuttlefish.sh download "$CF_BRANCH" "$CF_DEVICE"
  bash scripts/cuttlefish.sh test
}

run_cuttlefish() {
  [[ "$WITH_CUTTLEFISH" -eq 1 ]] || {
    log "Cuttlefish test skipped (--no-cuttlefish)"
    return
  }

  run_optional_job "Test ${CF_DEVICE}" run_cuttlefish_job
}

print_summary() {
  if [[ "${#FAILURES[@]}" -eq 0 ]]; then
    log "Pipeline completed successfully."
    return
  fi

  warn "Pipeline completed with failures (${#FAILURES[@]}):"
  local f
  for f in "${FAILURES[@]}"; do
    echo "  - $f" >&2
  done
  exit 1
}

parse_args() {
  while [[ $# -gt 0 ]]; do
    case "$1" in
      --no-avd)
        WITH_AVD=0
        ;;
      --quick-avd)
        QUICK_AVD=1
        ;;
      --no-cuttlefish)
        WITH_CUTTLEFISH=0
        ;;
      --with-cuttlefish)
        WITH_CUTTLEFISH=1
        ;;
      --skip-setup)
        SKIP_SETUP=1
        ;;
      --skip-build)
        SKIP_BUILD=1
        ;;
      --help|-h)
        usage
        exit 0
        ;;
      *)
        die "Unknown argument: $1"
        ;;
    esac
    shift
  done
}

main() {
  parse_args "$@"

  [[ "$(uname -s)" == "Linux" ]] || die "This script is intended for Linux only."
  [[ -f build.py ]] || die "build.py not found. Run this from Magisk repo root."

  require_command git
  prepare_python
  prepare_android_env

  if [[ "$WITH_AVD" -eq 1 || "$WITH_CUTTLEFISH" -eq 1 ]]; then
    if [[ ! -e /dev/kvm ]]; then
      warn "/dev/kvm not found. Emulator/Cuttlefish tests may fail."
    fi
  fi

  setup_environment
  run_builds
  run_avd_matrix
  run_cuttlefish
  print_summary
}

main "$@"
