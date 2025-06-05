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
GCONFIG=app/gradle.properties
README=README.MD
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

# $1 = tag
update_readme_canary() {
  sed -i "s:badge/Magisk-Canary.*:badge/Magisk-Canary-red)](https\://github.com/topjohnwu/Magisk/releases/tag/$1):g" $README
}

# $1 = tag
update_readme_beta() {
  update_readme_canary $1
  sed -i "s:badge/Magisk%20Beta.*:badge/Magisk%20Beta-${1}-blue)](https\://github.com/topjohnwu/Magisk/releases/tag/$1):g" $README
}

# $1 = tag
update_readme_stable() {
  update_readme_beta $1
  sed -i "s:badge/Magisk-v.*:badge/Magisk-${1}-blue)](https\://github.com/topjohnwu/Magisk/releases/tag/$1):g" $README
}

# $1 = tag
# $2 = file name
gen_link() {
  echo "https://github.com/topjohnwu/Magisk/releases/download/$1/$2"
}

# $1 = version code
is_canary() {
  [ $(($1 % 100)) -ne 0 ]
}

# $1 = json path
# $2 = apk name
update_json() {
  local json=$1
  local apk=$2
  local ver=$(grep_prop version $CONFIG)
  local code=$(grep_prop magisk.versionCode $GCONFIG)

  local tag
  if is_canary $code; then
    tag="canary-$code"
  else
    tag="v$ver"
  fi

  jq ".magisk.version=\"$ver\"|.magisk.versionCode=\"$code\"|
  .magisk.link=\"$(gen_link $tag $apk)\"|
  .magisk.note=\"$(gen_link $tag notes.md)\"" $json > ${json}.tmp
  mv ${json}.tmp $json
}

update_canary_json() {
  update_json $MAGISK_FILES/canary.json app-release.apk
  update_json $MAGISK_FILES/debug.json app-debug.apk
}

update_beta_json() {
  update_json $MAGISK_FILES/canary.json Magisk-v${ver}.apk
  update_json $MAGISK_FILES/debug.json app-debug.apk
  cp -vf $MAGISK_FILES/canary.json $MAGISK_FILES/beta.json
}

update_stable_json() {
  update_beta_json
  cp -vf $MAGISK_FILES/beta.json $MAGISK_FILES/stable.json
}

bump_canary_version() {
  # Update version code
  local code=$(grep_prop magisk.versionCode $GCONFIG)
  code=$((code + 1))
  local tag="canary-$code"
  sed -i "s:versionCode=.*:versionCode=${code}:g" $GCONFIG
  update_readme_canary $tag

  # Commit version code changes
  git add -u .
  git status
  git commit -m "Release new canary build" -m "[skip ci]"
  git tag $tag

  # Update version name
  local ver=$(git rev-parse --short=8 HEAD)
  sed -i "s:version=.*:version=${ver}:g" $CONFIG
  sed -i "1s:.*:## Magisk (${ver}) (${code}):" $NOTES
  update_canary_json

  # Commit json files
  cd $MAGISK_FILES
  git add -u .
  git status
  git commit -m "Update Canary Channel: Upstream to $ver"
  cd $CWD
}

# $1 = ver, $2 = stable?
set_version() {
  local ver=$1
  local stable=$2
  local code=$(echo - | awk "{ print $ver * 1000 }")
  local tag="v$ver"

  sed -i "s:versionCode=.*:versionCode=${code}:g" $GCONFIG
  sed -i "s:version=.*:version=${ver}:g" $CONFIG
  sed -i "1s:.*:## $(date +'%Y.%-m.%-d') Magisk v$ver:" $NOTES

  if $stable; then
    update_readme_stable $tag
    update_stable_json
  else
    update_readme_beta $tag
    update_beta_json
  fi

  # Commit version code changes
  git add -u .
  git status
  git commit -m "Release Magisk v$ver" -m "[skip ci]"
  git tag $tag

  # Commit json files
  cd $MAGISK_FILES
  git add -u .
  git status
  git commit -m "Release Magisk v$ver"
  cd $CWD
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
build_beta() {
  [ -z $1 ] && exit 1
  local ver=$1
  set_version $ver false
  build_apk
}

# $1 = ver
build_stable() {
  [ -z $1 ] && exit 1
  local ver=$1
  set_version $ver true
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
    "canary" )
      tag="canary-$code"
      title="Magisk ($ver) ($code)"

      # Assert tag format
      [ $latest_tag = $tag ]

      # Publish release
      gh release create --verify-tag $tag -p -t "$title" -F release.md $out/app-release.apk $out/app-debug.apk $NOTES
      ;;
    "beta|stable" )
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

  # Finally upload jsons
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
ensure_config
case $1 in
  "canary" ) build_canary ;;
  "beta" ) build_beta $2 ;;
  "stable" ) build_stable $2 ;;
  "upload" ) upload $2 ;;
  "revert" ) revert ;;
  * ) exit 1 ;;
esac
