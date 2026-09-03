#!/usr/bin/env bash
set -e
set -o pipefail

# Required tools: gh, apksigner
# The GitHub cli (gh) has to be properly authenticated

# These variables can be modified as needed
CONFIG=config.prop
NOTES=notes.md
APK_CERT=b4cb83b4dad99f997dbe872f013aa16c14eec41d167021f371f7e1330f273ee6

# These are constants, do not modify
GCONFIG=app/gradle.properties

grep_prop() {
  local REGEX="s/^$1=//p"
  shift
  local FILES=$@
  sed -n "$REGEX" $FILES | head -n 1
}

ver_to_code() {
  echo "$1" | awk '{ print int($1 * 1000) }'
}

code_to_ver() {
  echo "$1" | awk '{ v = $1 / 1000; if (v == int(v)) printf "%.1f\n", v; else print v }'
}

find_apksigner() {
  local sdk="${ANDROID_HOME:-$ANDROID_SDK_ROOT}"
  if [ -n "$sdk" ] && [ -d "$sdk/build-tools" ]; then
    local exe
    exe=$(find "$sdk/build-tools" -mindepth 2 -maxdepth 2 -name apksigner 2>/dev/null | sort -V | tail -n 1)
    if [ -n "$exe" ] && [ -x "$exe" ]; then
      echo "$exe"
      return 0
    fi
  fi
  echo "Error: apksigner not found in Android SDK build-tools." >&2
  return 1
}

verify_apks() {
  local apksigner
  apksigner=$(find_apksigner) || exit 1

  local apks=(out/*.apk)
  if [ ! -e "${apks[0]}" ]; then
    echo "Error: No APKs found in out" >&2
    exit 1
  fi

  echo "* Verifying APK signatures"
  for apk in "${apks[@]}"; do
    echo "Verifying $(basename "$apk")..."
    local raw_output
    if ! raw_output=$("$apksigner" verify --print-certs "$apk" 2>&1); then
      echo "Error: $apk failed signature verification!" >&2
      echo "$raw_output" >&2
      exit 1
    fi

    local certs
    certs=$(echo "$raw_output" | sed -n 's/.*certificate SHA-256 digest: *\([a-fA-F0-9]*\).*/\1/p' | \
      tr '[:upper:]' '[:lower:]' | sort -u)

    if [ -z "$certs" ]; then
      echo "Error: Could not extract signing certificate from $apk!" >&2
      exit 1
    fi

    for cert in $certs; do
      if [ "$cert" != "$APK_CERT" ]; then
        echo "Error: $apk signature mismatch!" >&2
        echo "  Expected: $APK_CERT" >&2
        echo "  Found:    $cert" >&2
        exit 1
      fi
    done
    echo "  -> Verified: $APK_CERT"
  done
  echo "All APKs successfully verified!"
}

# Generate a clean config for release builds inheriting only signing properties
gen_release_config() {
  local ver=$1
  local cfg=$2

  echo "version=$ver" > "$cfg"

  if [ -f "$CONFIG" ]; then
    for prop in keyStore keyStorePass keyAlias keyPass; do
      local val
      val=$(grep_prop "$prop" "$CONFIG")
      if [ -n "$val" ]; then
        echo "$prop=$val" >> "$cfg"
      fi
    done
  fi
}

# $1 = ver
set_version() {
  local ver=$1
  local code=$(ver_to_code "$ver")

  sed "s:versionCode=.*:versionCode=${code}:g" "$GCONFIG" > "$GCONFIG.tmp"
  mv -f "$GCONFIG.tmp" "$GCONFIG"

  # Commit version code changes
  git add "$GCONFIG"
  git status
  git commit -m "Release Magisk v$ver" -m "[skip ci]"
}

# $1 = ver
build() {
  local ver=$1
  git pull
  set_version "$ver"

  local rel_config
  rel_config=$(mktemp 2>/dev/null || mktemp -t 'release_config')
  trap 'rm -f "$rel_config"' EXIT
  gen_release_config "$ver" "$rel_config"

  local build_cmd="./build.py -c $rel_config"
  $build_cmd clean
  $build_cmd all
  $build_cmd -r all
  $build_cmd -r app-legacy
  verify_apks
}

upload() {
  gh auth status

  if [ ! -f "$NOTES" ]; then
    echo "Error: Release notes file '$NOTES' not found!" >&2
    exit 1
  fi

  verify_apks

  if [ ! -f "out/app-release.apk" ] || [ ! -f "out/apk-legacy-release.apk" ] || [ ! -f "out/app-debug.apk" ]; then
    echo "Error: Required APKs missing in out" >&2
    exit 1
  fi

  local code=$(grep_prop magisk.versionCode $GCONFIG)
  local ver=$(code_to_ver "$code")
  local tag="v$ver"
  local title="Magisk v$ver"

  git tag "$tag"
  git push origin master
  git push --tags

  # Publish release
  local release_apk="Magisk-v${ver}.apk"
  local legacy_apk="Magisk-v${ver}-legacy.apk"
  cp "out/app-release.apk" "$release_apk"
  cp "out/apk-legacy-release.apk" "$legacy_apk"
  gh release create --verify-tag "$tag" -d -t "$title" -F "$NOTES" "$release_apk" "$legacy_apk" "out/app-debug.apk"

  rm -f "$release_apk" "$legacy_apk"
}

usage() {
  echo "Usage: $0 <build <version> | upload | verify>" >&2
  exit 1
}

case $1 in
  build )
    [ -z "$2" ] && usage
    build "$2"
    ;;
  upload )
    upload
    ;;
  verify )
    verify_apks
    ;;
  * )
    usage
    ;;
esac
