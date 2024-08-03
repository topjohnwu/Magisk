#!/usr/bin/env bash
set -e

# On macOS, gsed is required (brew install gnu-sed)
# Required tools: jq, gh
# The GitHub cli (gh) has to be properly authenticated

# These variables can be modified as needed
MAGISK_FILES=../magisk-files
CONFIG=config.prop
NOTES=notes.md

# These are constants, do not modify
GCONFIG=gradle.properties
README=README.MD
BUILDCMD="./build.py -c $CONFIG"
CWD=$(pwd)

grep_prop() {
  local REGEX="s/^$1=//p"
  shift
  local FILES=$@
  sed -n "$REGEX" $FILES | head -n 1
}

enable_version_config() {
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

# $1 = tag
update_readme_stable() {
  sed -i "s:badge/Magisk-v.*:badge/Magisk-${1}-blue)](https\://github.com/topjohnwu/Magisk/releases/tag/$1):g" $README
}

# $1 = tag
update_readme_beta() {
  sed -i "s:badge/Magisk%20Beta.*:badge/Magisk%20Beta-${1}-blue)](https\://github.com/topjohnwu/Magisk/releases/tag/$1):g" $README
}

# $1 = tag
update_readme_canary() {
  sed -i "s:badge/Magisk-Canary.*:badge/Magisk-Canary-red)](https\://github.com/topjohnwu/Magisk/releases/tag/$1):g" $README
}

gen_link() {
  echo "https://github.com/topjohnwu/Magisk/releases/download/$1/$2"
}

update_canary_json() {
  local ver=$(grep_prop version $CONFIG)
  local code=$(grep_prop magisk.versionCode $GCONFIG)
  local tag="canary-$code"
  local json

  json=$MAGISK_FILES/canary.json
  jq ".magisk.version=\"$ver\"|.magisk.versionCode=\"$code\"|
  .magisk.link=\"$(gen_link $tag app-release.apk)\"|
  .magisk.note=\"$(gen_link $tag notes.md)\"" $json > ${json}.tmp
  mv ${json}.tmp $json

  json=$MAGISK_FILES/debug.json
  jq ".magisk.version=\"$ver\"|.magisk.versionCode=\"$code\"|
  .magisk.link=\"$(gen_link $tag app-debug.apk)\"|
  .magisk.note=\"$(gen_link $tag notes.md)\"" $json > ${json}.tmp
  mv ${json}.tmp $json
}

# $1 = json path
update_release_json() {
  local json=$1
  local ver=$(grep_prop version $CONFIG)
  local code=$(grep_prop magisk.versionCode $GCONFIG)

  jq ".magisk.version=\"$ver\"|.magisk.versionCode=\"$code\"|
  .magisk.link=\"$(gen_link v${ver} Magisk-v${ver}.apk)\"|
  .magisk.note=\"https://topjohnwu.github.io/Magisk/releases/${code}.md\"" $json > ${json}.tmp
  mv ${json}.tmp $json
}

build_canary() {
  # Update version code
  local code=$(grep_prop magisk.versionCode $GCONFIG)
  code=$((code + 1))
  local tag="canary-$code"
  sed -i "s:versionCode=.*:versionCode=${code}:g" $GCONFIG
  update_readme_canary $tag

  # Commit version code changes
  git add -u .
  git status
  git commit -m "Release new canary build"
  git tag $tag

  # Update version name
  local ver=$(git rev-parse --short=8 HEAD)
  sed -i "s:version=.*:version=${ver}:g" $CONFIG
  sed -i "s:## Magisk (.*:## Magisk (${ver}) (${code}):g" $NOTES

  # Update and commit JSON
  update_canary_json
  cd $MAGISK_FILES
  git add -u .
  git status
  git commit -m "Update Canary Channel: Upstream to $ver"
  cd $CWD

  # Build
  $BUILDCMD clean
  $BUILDCMD all
  $BUILDCMD -r all
}

# $1 = ver, $2 = stable?
build_release() {
  # Update version configs
  local ver=$1
  local stable=$2
  local code=$(echo - | awk "{ print $ver * 1000 }")
  local tag="v$ver"
  sed -i "s:versionCode=.*:versionCode=${code}:g" $GCONFIG
  sed -i "s:version=.*:version=${ver}:g" $CONFIG

  # Update and commit JSON
  if $stable; then
    update_readme_stable $tag
    update_readme_beta $tag
    update_release_json $MAGISK_FILES/stable.json
    cp -vf $MAGISK_FILES/stable.json $MAGISK_FILES/beta.json
  else
    update_readme_beta $tag
    update_release_json $MAGISK_FILES/beta.json
  fi
  cd $MAGISK_FILES
  git add -u .
  git status
  git commit -m "Release Magisk v$ver"
  cd $CWD

  # Commit version code changes
  git add -u .
  git status
  git commit -m "Release Magisk v$ver"
  git tag $tag

  # Build
  $BUILDCMD clean
  $BUILDCMD -r all
}

stable() {
  [ -z $1 ] && exit 1
  local ver=$1
  build_release $ver true
}

beta() {
  [ -z $1 ] && exit 1
  local ver=$1
  build_release $ver false
}

pub() {
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

  if [ $(($code % 100)) -ne 0 ]; then
    tag="canary-$code"
    title="Magisk ($ver) ($code)"

    # Assert tag format
    [ $latest_tag = $tag ]

    # Publish release
    tail -n +3 $NOTES > release.md
    gh release create --verify-tag $tag -p -t "$title" -F release.md $out/app-release.apk $out/app-debug.apk $NOTES
  else
    tag="v$ver"
    title="Magisk v$ver"

    # Assert tag format
    [ $latest_tag = $tag ]

    # Publish release
    tail -n +3 docs/releases/$code.md > release.md
    gh release create --verify-tag $tag -t "$title" -F release.md "$out/app-release.apk#Magisk-v${ver}.apk"
  fi

  rm -f release.md
  cd $MAGISK_FILES
  git push origin master
  cd $CWD
}

revert() {
  local latest_tag=$(git describe --abbrev=0 --tags)

  git tag -d $latest_tag
  git reset --hard HEAD~
  cd $MAGISK_FILES
  git reset --hard HEAD~
  cd $CWD
}

# Use GNU sed on macOS
if command -v gsed >/dev/null; then
  function sed() { gsed "$@"; }
  export -f sed
fi

git pull

trap disable_version_config EXIT
enable_version_config
case $1 in
  "canary" ) build_canary ;;
  "stable" ) stable $2 ;;
  "beta" ) beta $2 ;;
  "pub" ) pub ;;
  "revert" ) revert ;;
  * ) exit 1 ;;
esac
