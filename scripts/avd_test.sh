#!/usr/bin/env bash

set -xe

cleanup() {
  echo -e '\n\033[41m! An error occurred\033[0m\n'
  pkill -INT -P $$
  wait
}

trap cleanup EXIT

emu="$ANDROID_SDK_ROOT/emulator/emulator"
avd="$ANDROID_SDK_ROOT/cmdline-tools/latest/bin/avdmanager"
sdk="$ANDROID_SDK_ROOT/cmdline-tools/latest/bin/sdkmanager"
emu_args='-no-window -gpu swiftshader_indirect -no-snapshot -noaudio -no-boot-anim'

case $(uname -m) in
  'arm64'|'aarch64')
    arch=arm64-v8a
    ;;
  *)
    arch=x86_64
    ;;
esac

# Should be either 'google_apis' or 'default'
type='google_apis'

wait_for_boot() {
  while true; do
    if [ -n "$(adb shell getprop sys.boot_completed)" ]; then
      break
    fi
    sleep 2
  done
}

export -f wait_for_boot

test_api() {
  local pkg pid img_dir ramdisk features

  # Setup emulator
  pkg="system-images;android-$1;$type;$arch"

  echo -e "\n\033[44m* Testing $pkg\033[0m\n"

  "$sdk" $pkg
  echo no | "$avd" create avd -f -n test -k $pkg

  # Launch emulator and patch
  img_dir="$ANDROID_SDK_ROOT/system-images/android-$1/$type/$arch"
  ramdisk="$img_dir/ramdisk.img"
  features="$img_dir/advancedFeatures.ini"
  if [ -f "${ramdisk}.bak" ]; then
    cp "${ramdisk}.bak" "$ramdisk"
  fi
  if [ -f "${features}.bak" ]; then
    cp "${features}.bak" "$features"
  fi
  "$emu" @test $emu_args &
  pid=$!
  timeout 60 adb wait-for-device
  ./build.py avd_patch -s "$ramdisk"
  kill -INT $pid
  wait $pid

  # Test if it boots properly
  "$emu" @test $emu_args &
  pid=$!
  timeout 60 adb wait-for-device
  timeout 60 bash -c wait_for_boot

  adb shell magisk -v
  kill -INT $pid
  wait $pid

  # Restore patches
  if [ -f "${ramdisk}.bak" ]; then
    cp "${ramdisk}.bak" "$ramdisk"
  fi
  if [ -f "${features}.bak" ]; then
    cp "${features}.bak" "$features"
  fi
}

# Build our executables
./build.py binary
./build.py app

# We test these API levels for the following reason

# API 23: legacy rootfs w/o Treble
# API 26: legacy rootfs with Treble
# API 28: legacy system-as-root
# API 29: 2 Stage Init
# API 33: latest Android

for api in 23 26 28 29 33; do
  test_api $api
done

"$avd" delete avd -n test

trap - EXIT
