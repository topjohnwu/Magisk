#!/usr/bin/env bash
set -e

# On macOS, gsed is required (brew install gnu-sed)
# Required tools: gh
# The GitHub cli (gh) has to be properly authenticated

# These variables can be modified as needed
CONFIG=config.prop
NOTES=notes.md
APK_CERT=b4cb83b4dad99f997dbe872f013aa16c14eec41d167021f371f7e1330f273ee6

# These are constants, do not modify
GCONFIG=app/gradle.properties
BUILDCMD="./build.py -c $CONFIG"
CWD=$(pwd)

grep_prop() {
  local REGEX="s/^$1=//p"
  shift
  local FILES=$@
  sed -n "$REGEX" $FILES | head -n 1
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

  local out=$(grep_prop outdir $CONFIG)
  if [ -z "$out" ]; then
    out=out
  fi

  local apks=("$out"/*.apk)
  if [ ! -e "${apks[0]}" ]; then
    echo "Error: No APKs found in $out" >&2
    exit 1
  fi

  local expected_cert
  expected_cert=$(echo "$APK_CERT" | tr '[:upper:]' '[:lower:]')

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
      if [ "$cert" != "$expected_cert" ]; then
        echo "Error: $apk signature mismatch!" >&2
        echo "  Expected: $expected_cert" >&2
        echo "  Found:    $cert" >&2
        exit 1
      fi
    done
    echo "  -> Verified: $expected_cert"
  done
  echo "All APKs successfully verified!"
}

ensure_config() {
  # Make sure version is not commented out and exists
  sed -i "s:^# version=:version=:g" $CONFIG
  if ! grep -qE '^version=' $CONFIG; then
    echo 'version=' >> $CONFIG
  fi
  # Make sure abiList is not set when building for release
  sed -i "s:^abiList=:# abiList=:g" $CONFIG
}

disable_version_config() {
  # Comment out version config
  sed -i "s:^version=:# version=:g" $CONFIG
}

# $1 = ver
set_version() {
  local ver=$1
  local code=$(echo - | awk "{ print $ver * 1000 }")
  local tag="v$ver"

  sed -i "s:versionCode=.*:versionCode=${code}:g" $GCONFIG
  sed -i "s:version=.*:version=${ver}:g" $CONFIG

  # Commit version code changes
  git add -u .
  git status
  git commit -m "Release Magisk v$ver" -m "[skip ci]"
}

# $1 = ver
build() {
  [ -z $1 ] && exit 1
  local ver=$1
  git pull
  set_version $ver
  $BUILDCMD clean
  $BUILDCMD all
  $BUILDCMD -r all
  $BUILDCMD -r app-legacy
}

upload() {
  gh auth status
  verify_apks

  local code=$(grep_prop magisk.versionCode $GCONFIG)
  local ver=$(echo - | awk "{ print $code / 1000 }")
  local tag="v$ver"
  local title="Magisk v$ver"

  local out=$(grep_prop outdir $CONFIG)
  if [ -z $out ]; then
    out=out
  fi

  git tag $tag
  git push origin master
  git push --tags

  # Publish release
  local release_apk="Magisk-v${ver}.apk"
  local legacy_apk="Magisk-v${ver}-legacy.apk"
  cp $out/app-release.apk $release_apk
  cp $out/apk-legacy-release.apk $legacy_apk
  gh release create --verify-tag $tag -d -t "$title" -F $NOTES $release_apk $legacy_apk $out/app-debug.apk

  rm -f $release_apk $legacy_apk
}

# Use GNU sed on macOS
if command -v gsed >/dev/null; then
  function sed() { gsed "$@"; }
  export -f sed
fi

case $1 in
  build )
    trap disable_version_config EXIT
    ensure_config
    build $2
    ;;
  upload )
    upload
    ;;
  verify )
    verify_apks
    ;;
  * ) exit 1 ;;
esac
