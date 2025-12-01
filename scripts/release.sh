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
}

upload() {
  gh auth status

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

  # Prepare release notes
  tail -n +3 $NOTES > release.md

  # Publish release
  local release_apk="Magisk-v${ver}.apk"
  cp $out/app-release.apk $release_apk
  gh release create --verify-tag $tag -p -t "$title" -F release.md $release_apk $out/app-debug.apk $NOTES

  rm -f $release_apk release.md
}

# Use GNU sed on macOS
if command -v gsed >/dev/null; then
  function sed() { gsed "$@"; }
  export -f sed
fi

trap disable_version_config EXIT
ensure_config
case $1 in
  build ) build $2 ;;
  upload ) upload ;;
  * ) exit 1 ;;
esac
