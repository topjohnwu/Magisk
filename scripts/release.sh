#!/usr/bin/env bash
set -e

# On macOS, gsed is required (brew install gnu-sed)
# Required tools: gh
# The GitHub cli (gh) has to be properly authenticated

# These variables can be modified as needed
CONFIG=config.prop
NOTES=notes.md

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

bump_canary_version() {
  # Update version code
  local code=$(grep_prop magisk.versionCode $GCONFIG)
  code=$((code + 1))
  local tag="canary-$code"
  sed -i "s:versionCode=.*:versionCode=${code}:g" $GCONFIG

  # Commit version code changes
  git add -u .
  git status
  git commit -m "Release new canary build" -m "[skip ci]"
  git tag $tag

  # Update version name
  local ver=$(git rev-parse --short=8 HEAD)
  sed -i "s:version=.*:version=${ver}:g" $CONFIG
  sed -i "1s:.*:## Magisk (${ver}) (${code}):" $NOTES
}

# $1 = ver
set_version() {
  local ver=$1
  local code=$(echo - | awk "{ print $ver * 1000 }")
  local tag="v$ver"

  sed -i "s:versionCode=.*:versionCode=${code}:g" $GCONFIG
  sed -i "s:version=.*:version=${ver}:g" $CONFIG
  sed -i "1s:.*:## $(date +'%Y.%-m.%-d') Magisk v$ver:" $NOTES

  # Commit version code changes
  git add -u .
  git status
  git commit -m "Release Magisk v$ver" -m "[skip ci]"
  git tag $tag
}

build_apk() {
  $BUILDCMD clean
  $BUILDCMD all
  $BUILDCMD -r all
}

build_canary() {
  bump_canary_version
  build_apk
}

# $1 = ver
build_public() {
  [ -z $1 ] && exit 1
  local ver=$1
  set_version $ver
  build_apk
}

upload() {
  # Verify pattern
  [[ "$1" =~ canary|beta|stable ]]
  local type=$1

  gh auth status

  local latest_tag=$(git describe --abbrev=0 --tags)
  local ver=$(grep_prop version $CONFIG)
  local code=$(grep_prop magisk.versionCode $GCONFIG)
  local out=$(grep_prop outdir $CONFIG)
  local tag title

  if [ -z $out ]; then
    out=out
  fi

  git push origin master
  git push --tags

  # Prepare release notes
  tail -n +3 $NOTES > release.md

  case $type in
    canary )
      tag="canary-$code"
      title="Magisk ($ver) ($code)"

      # Assert tag format
      [ $latest_tag = $tag ]

      # Publish release
      gh release create --verify-tag $tag -p -t "$title" -F release.md $out/app-release.apk $out/app-debug.apk $NOTES
      ;;
    beta|stable )
      tag="v$ver"
      title="Magisk v$ver"

      # Assert tag format
      [ $latest_tag = $tag ]

      # Publish release
      local release_apk="Magisk-v${ver}.apk"
      cp $out/app-release.apk $release_apk
      gh release create --verify-tag $tag -p -t "$title" -F release.md $release_apk $out/app-debug.apk $NOTES
      rm -f $release_apk
      ;;
  esac

  # If publishing stable, make it not prerelease and explicitly latest
  if [ $type = "stable" ]; then
    gh release edit $tag --prerelease=false --latest
  fi

  rm -f release.md
}

revert() {
  local latest_tag=$(git describe --abbrev=0 --tags)
  git tag -d $latest_tag
  git reset --hard HEAD~
}

# Use GNU sed on macOS
if command -v gsed >/dev/null; then
  function sed() { gsed "$@"; }
  export -f sed
fi

git pull

trap disable_version_config EXIT
ensure_config
case $1 in
  canary ) build_canary ;;
  public ) build_public $2 ;;
  upload ) upload $2 ;;
  revert ) revert ;;
  * ) exit 1 ;;
esac
