#!/usr/bin/env bash

set -xe
. scripts/test_common.sh

emu_args_base="-no-window -no-audio -no-boot-anim -gpu swiftshader_indirect -read-only -no-snapshot -cores $core_count"
lsposed_url='https://github.com/LSPosed/LSPosed/releases/download/v1.9.2/LSPosed-v1.9.2-7024-zygisk-release.zip'
emu_pid=

atd_min_api=30
atd_max_api=34
lsposed_min_api=27
lsposed_max_api=34
huge_ram_min_api=26

cleanup() {
  print_error "! An error occurred when testing $pkg"

  rm -f magisk_patched.img
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

  print_title "* Testing $pkg ($variant)"

  if [ -n "$AVD_TEST_LOG" ]; then
    "$emu" @test $emu_args > kernel.log 2>&1 &
  else
    "$emu" @test $emu_args > /dev/null 2>&1 &
  fi

  emu_pid=$!
  wait_emu wait_for_boot

  test_setup $variant

  # Install LSPosed
  if [ $api -ge $lsposed_min_api -a $api -le $lsposed_max_api ]; then
    adb push out/lsposed.zip /data/local/tmp/lsposed.zip
    echo 'PATH=$PATH:/debug_ramdisk magisk --install-module /data/local/tmp/lsposed.zip' | adb shell /system/xbin/su
  fi

  adb reboot
  wait_emu wait_for_boot

  test_app

  # Try to launch LSPosed
  if [ $api -ge $lsposed_min_api -a $api -le $atd_max_api ]; then
    adb shell rm -f /data/local/tmp/window_dump.xml
    adb shell am start -c org.lsposed.manager.LAUNCH_MANAGER com.android.shell/.BugreportWarningActivity
    while adb shell '[ ! -f /data/local/tmp/window_dump.xml ]'; do
      sleep 10
      adb shell uiautomator dump /data/local/tmp/window_dump.xml
    done
    adb shell grep -q org.lsposed.manager /data/local/tmp/window_dump.xml
  fi
}

run_test() {
  local ver=$1
  local type=$2

  # Determine API level
  local api
  case $ver in
    [0-9]*) api=$ver ;;
    TiramisuPrivacySandbox) api=33 ;;
    UpsideDownCakePrivacySandbox) api=34 ;;
    VanillaIceCream) api=35 ;;
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
  local pkg="system-images;android-$ver;$type;$arch"
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
  "$sdk" --channel=3 $pkg
  echo no | "$avd" create avd -f -n test -k $pkg

  # Launch stock emulator
  print_title "* Launching $pkg"
  "$emu" @test $emu_args >/dev/null 2>&1 &
  emu_pid=$!
  wait_emu wait_for_bootanim

  # Update arguments for Magisk runs
  emu_args="$emu_args -ramdisk magisk_patched.img -feature -SystemAsRoot"
  if [ -n "$AVD_TEST_LOG" ]; then
    emu_args="$emu_args -show-kernel -logcat '' -logcat-output logcat.log"
  fi

  # Patch and test debug build
  ./build.py avd_patch -s "$ramdisk" magisk_patched.img
  kill -INT $emu_pid
  wait $emu_pid
  test_emu debug $api

  # Patch and test release build
  ./build.py -r avd_patch -s "$ramdisk" magisk_patched.img
  kill -INT $emu_pid
  wait $emu_pid
  test_emu release $api

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
curl -L $lsposed_url -o out/lsposed.zip
"$sdk" --channel=3 platform-tools emulator

if [ -n "$1" ]; then
  run_test $1 $2
else
  for api in $(seq 23 34); do
    run_test $api
  done
  # Android 15 Beta
  run_test 35 google_apis
  # Run 16k page tests
  run_test 35 google_apis_ps16k
fi

"$avd" delete avd -n test

trap - EXIT
