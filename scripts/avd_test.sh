#!/usr/bin/env bash

set -xe
. scripts/test_common.sh

emu_args_base="-no-window -no-audio -no-boot-anim -gpu swiftshader_indirect -read-only -no-snapshot -cores $core_count"
emu_pid=

atd_min_api=30
atd_max_api=35
huge_ram_min_api=26

cleanup() {
  print_error "! An error occurred"

  rm -f magisk_patched.img
  "$avd" delete avd -n test
  pkill -INT -P $$
  wait
  trap - EXIT
  exit 1
}

wait_for_bootanim() {
  set -e
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
  set -e
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

wait_emu() {
  local wait_fn=$1
  local which_pid

  timeout $boot_timeout bash -c $wait_fn &
  local wait_pid=$!

  # Handle the case when emulator dies earlier than timeout
  wait -p which_pid -n $emu_pid $wait_pid
  [ $which_pid -eq $wait_pid ]
}

test_emu() {
  local variant=$1
  local api=$2

  print_title "* Testing $avd_pkg ($variant)"

  if [ -n "$AVD_TEST_LOG" ]; then
    "$emu" @test $emu_args > kernel.log 2>&1 &
  else
    "$emu" @test $emu_args > /dev/null 2>&1 &
  fi

  emu_pid=$!
  wait_emu wait_for_boot

  run_setup $variant

  adb reboot
  wait_emu wait_for_boot

  run_tests
}

test_main() {
  local ver=$1
  local type=$2

  # Determine API level
  local api
  case $ver in
    [0-9]*) api=$ver ;;
    TiramisuPrivacySandbox) api=33 ;;
    UpsideDownCakePrivacySandbox) api=34 ;;
    VanillaIceCream) api=35 ;;
    Baklava) api=36 ;;
    *)
      print_error "! Unknown system image version '$ver'"
      exit 1
      ;;
  esac

  # Determine image type
  if [ -z $type ]; then
    if [ $api -ge $atd_min_api -a $api -le $atd_max_api ]; then
      # Use the lightweight ATD images if possible
      type='aosp_atd'
    else
      type='default'
    fi
  fi

  # System image variable and paths
  local avd_pkg="system-images;android-$ver;$type;$arch"
  local sys_img_dir="$ANDROID_HOME/system-images/android-$ver/$type/$arch"
  local ramdisk="$sys_img_dir/ramdisk.img"

  # Old Linux kernels will not boot with memory larger than 3GB
  local memory
  if [ $api -lt $huge_ram_min_api ]; then
    memory=3072
  else
    memory=8192
  fi

  emu_args="$emu_args_base -memory $memory"

  # Setup emulator
  "$sdk" --channel=3 $avd_pkg
  echo no | "$avd" create avd -f -n test -k $avd_pkg

  # Launch stock emulator
  print_title "* Launching $avd_pkg"
  "$emu" @test $emu_args >/dev/null 2>&1 &
  emu_pid=$!
  wait_emu wait_for_bootanim

  # Update arguments for Magisk runs
  emu_args="$emu_args -ramdisk magisk_patched.img -feature -SystemAsRoot"
  if [ -n "$AVD_TEST_LOG" ]; then
    emu_args="$emu_args -show-kernel -logcat '' -logcat-output logcat.log"
  fi

  if [ -z "$AVD_TEST_SKIP_DEBUG" ]; then
    # Patch and test debug build
    ./build.py avd_patch -s "$ramdisk" magisk_patched.img
    kill -INT $emu_pid
    wait $emu_pid
    test_emu debug $api
  fi

  if [ -z "$AVD_TEST_SKIP_RELEASE" ]; then
    # Patch and test release build
    ./build.py -r avd_patch -s "$ramdisk" magisk_patched.img
    kill -INT $emu_pid
    wait $emu_pid
    test_emu release $api
  fi

  # Cleanup
  kill -INT $emu_pid
  wait $emu_pid
  rm -f magisk_patched.img
}

trap cleanup EXIT
export -f wait_for_boot
export -f wait_for_bootanim

case $(uname -m) in
  'arm64'|'aarch64')
    arch=arm64-v8a
    ;;
  *)
    arch=x86_64
    ;;
esac

if [ -n "$FORCE_32_BIT" ]; then
  case $arch in
    'arm64-v8a')
      echo "! ARM32 is not supported"
      exit 1
      ;;
    'x86_64')
      arch=x86
      max_api=$i386_max_api
      ;;
  esac
fi

yes | "$sdk" --licenses > /dev/null
"$sdk" --channel=3 platform-tools emulator

adb kill-server
adb start-server

if [ -n "$1" ]; then
  test_main $1 $2
else
  for api in $(seq 23 35); do
    test_main $api
  done
  # Android 16 Beta
  test_main Baklava google_apis
  # Run 16k page tests
  test_main Baklava google_apis_ps16k
fi

"$avd" delete avd -n test

trap - EXIT
