#!/usr/bin/env bash

emu="$ANDROID_SDK_ROOT/emulator/emulator"
avd="$ANDROID_SDK_ROOT/cmdline-tools/latest/bin/avdmanager"
sdk="$ANDROID_SDK_ROOT/cmdline-tools/latest/bin/sdkmanager"
emu_args='-no-window -gpu swiftshader_indirect -read-only -no-snapshot -no-audio -no-boot-anim -show-kernel'
boot_timeout=600
emu_pid=

# Should be either 'google_apis' or 'default'
type='default'

# We test these API levels for the following reason

# API 23: legacy rootfs w/o Treble
# API 26: legacy rootfs with Treble
# API 28: legacy system-as-root
# API 29: 2 Stage Init
# API 34: latest Android

api_list='23 26 28 29 34'

print_title() {
  echo -e "\n\033[44;39m${1}\033[0m\n"
}

print_error() {
  echo -e "\n\033[41;39m${1}\033[0m\n"
}

cleanup() {
  print_error "! An error occurred when testing $pkg"

  for api in $api_list; do
    set_api_env $api
    restore_avd
  done

  "$avd" delete avd -n test
  pkill -INT -P $$
  wait
  trap - EXIT
  exit 1
}

wait_for_bootanim() {
  adb wait-for-device
  while true; do
    local result="$(adb exec-out getprop init.svc.bootanim)"
    if [ $? -ne 0 ]; then
      exit 1
    elif [ "$result" = "stopped" ]; then
      break
    fi
    sleep 2
  done
}

wait_for_boot() {
  adb wait-for-device
  while true; do
    local result="$(adb exec-out getprop sys.boot_completed)"
    if [ $? -ne 0 ]; then
      exit 1
    elif [ "$result" = "1" ]; then
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

wait_emu() {
  local wait_fn=$1
  local which_pid

  timeout $boot_timeout bash -c $wait_fn &
  local wait_pid=$!

  # Handle the case when emulator dies earlier than wait
  wait -p which_pid -n $emu_pid $wait_pid
  [ $which_pid -eq $wait_pid ]
}

test_emu() {
  local variant=$1

  print_title "* Testing $pkg ($variant)"

  "$emu" @test $emu_args &
  emu_pid=$!
  wait_emu wait_for_boot

  adb shell magisk -v

  # Install the Magisk app
  adb install -r -g out/app-${variant}.apk
  adb shell appops set com.topjohnwu.magisk REQUEST_INSTALL_PACKAGES allow

  # Use the app to run setup and reboot
  adb shell echo "'content call --uri content://com.topjohnwu.magisk.provider --method setup'" \| /system/xbin/su \
    | tee /dev/fd/2 | grep -q 'result=true'
  adb reboot
  wait_emu wait_for_boot

  # Run app tests
  adb shell echo "'content call --uri content://com.topjohnwu.magisk.provider --method test'" \| /system/xbin/su \
    | tee /dev/fd/2 | grep -q 'result=true'
  adb shell echo "'su -c id'" \| /system/xbin/su 2000 | tee /dev/fd/2 | grep -q 'uid=0'
}


run_test() {
  local api=$1

  set_api_env $api

  # Setup emulator
  "$sdk" $pkg
  echo no | "$avd" create avd -f -n test -k $pkg

  # Launch stock emulator
  print_title "* Launching $pkg"
  restore_avd
  "$emu" @test $emu_args &
  emu_pid=$!
  wait_emu wait_for_bootanim

  # Patch and test debug build
  ./build.py avd_patch -s "$ramdisk"
  kill -INT $emu_pid
  wait $emu_pid
  test_emu debug

  # Re-patch and test release build
  ./build.py -r avd_patch -s "$ramdisk"
  kill -INT $emu_pid
  wait $emu_pid
  test_emu release

  # Cleanup
  kill -INT $emu_pid
  wait $emu_pid
  restore_avd
}

trap cleanup EXIT

export -f wait_for_boot
export -f wait_for_bootanim

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
