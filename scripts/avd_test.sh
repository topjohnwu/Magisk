#!/usr/bin/env bash

emu="$ANDROID_SDK_ROOT/emulator/emulator"
avd="$ANDROID_SDK_ROOT/cmdline-tools/latest/bin/avdmanager"
sdk="$ANDROID_SDK_ROOT/cmdline-tools/latest/bin/sdkmanager"
emu_args='-no-window -gpu swiftshader_indirect -read-only -no-snapshot -noaudio -no-boot-anim -show-kernel'
boot_timeout=300

# Should be either 'google_apis' or 'default'
type='google_apis'

# We test these API levels for the following reason

# API 23: legacy rootfs w/o Treble
# API 26: legacy rootfs with Treble
# API 28: legacy system-as-root
# API 29: 2 Stage Init
# API 34: latest Android

api_list='23 26 28 29 34'

cleanup() {
  echo -e '\n\033[41;39m! An error occurred\033[0m\n'

  for api in $api_list; do
    set_api_env $api
    restore_avd
  done

  "$avd" delete avd -n test
  pkill -INT -P $$
  wait
}

wait_for_boot() {
  adb wait-for-device
  while true; do
    if [ "1" = "$(adb exec-out getprop sys.boot_completed)" ]; then
      break
    fi
    sleep 2
  done
}

set_api_env() {
  pkg="system-images;android-$1;$type;$arch"
  local img_dir="$ANDROID_SDK_ROOT/system-images/android-$1/$type/$arch"
  ramdisk="$img_dir/ramdisk.img"
  features="$img_dir/advancedFeatures.ini"
}

restore_avd() {
  if [ -f "${ramdisk}.bak" ]; then
    cp "${ramdisk}.bak" "$ramdisk"
  fi
  if [ -f "${features}.bak" ]; then
    cp "${features}.bak" "$features"
  fi
}

run_test() {
  local pid
  local api=$1

  set_api_env $api

  # Setup emulator
  echo -e "\n\033[44;39m* Testing $pkg\033[0m\n"
  "$sdk" $pkg
  echo no | "$avd" create avd -f -n test -k $pkg

  # Launch emulator and patch
  restore_avd
  "$emu" @test $emu_args &
  pid=$!
  timeout $boot_timeout bash -c wait_for_boot

  ./build.py avd_patch -s "$ramdisk"
  kill -INT $pid
  wait $pid

  # Test if it boots properly
  "$emu" @test $emu_args &
  pid=$!
  timeout $boot_timeout bash -c wait_for_boot

  adb shell magisk -v
  kill -INT $pid
  wait $pid

  restore_avd
}

trap cleanup EXIT

export -f wait_for_boot

set -xe

case $(uname -m) in
  'arm64'|'aarch64')
    arch=arm64-v8a
    ;;
  *)
    arch=x86_64
    ;;
esac

yes | "$sdk" --licenses
"$sdk" --channel=3 --update

if [ -n "$1" ]; then
  run_test $1
else
  for api in $api_list; do
    run_test $api
  done
fi

"$avd" delete avd -n test

trap - EXIT
