#!/usr/bin/env bash

emu="$ANDROID_SDK_ROOT/emulator/emulator"
avd="$ANDROID_SDK_ROOT/cmdline-tools/latest/bin/avdmanager"
sdk="$ANDROID_SDK_ROOT/cmdline-tools/latest/bin/sdkmanager"
emu_args_base='-no-window -no-audio -no-boot-anim -gpu swiftshader_indirect -read-only -no-snapshot -show-kernel'
lsposed_url='https://github.com/LSPosed/LSPosed/releases/download/v1.9.2/LSPosed-v1.9.2-7024-zygisk-release.zip'
boot_timeout=600
emu_pid=

export PATH="$PATH:$ANDROID_SDK_ROOT/platform-tools"

atd_min_api=30
atd_max_api=34
lsposed_min_api=27
huge_ram_min_api=26

print_title() {
  echo -e "\n\033[44;39m${1}\033[0m\n"
}

print_error() {
  echo -e "\n\033[41;39m${1}\033[0m\n"
}

cleanup() {
  print_error "! An error occurred when testing $pkg"

  find $ANDROID_SDK_ROOT/system-images -name 'ramdisk.img' -exec cp -v {}.bak {} \; 2>/dev/null
  find $ANDROID_SDK_ROOT/system-images -name 'advancedFeatures.ini' -exec cp -v {}.bak {} \; 2>/dev/null

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

restore_backup() {
  if [ -f "${1}.bak" ]; then
    cp "${1}.bak" "$1"
  fi
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

run_content_cmd() {
  while true; do
    local out=$(adb shell echo "'content call --uri content://com.topjohnwu.magisk.provider --method $1'" \| /system/xbin/su | tee /dev/fd/2)
    if ! grep -q 'Bundle\[' <<< "$out"; then
      # The call failed, wait a while and retry later
      sleep 30
    else
      grep -q 'result=true' <<< "$out"
      return $?
    fi
  done
}

test_emu() {
  local variant=$1
  local api=$2

  print_title "* Testing $pkg ($variant)"

  "$emu" @test $emu_args &
  emu_pid=$!
  wait_emu wait_for_boot

  adb shell 'PATH=$PATH:/debug_ramdisk magisk -v'

  # Install the Magisk app
  adb install -r -g out/app-${variant}.apk

  # Use the app to run setup and reboot
  run_content_cmd setup

  # Install LSPosed
  if [ $api -ge $lsposed_min_api -a $api -le $atd_max_api ]; then
    adb push out/lsposed.zip /data/local/tmp/lsposed.zip
    adb shell echo 'PATH=$PATH:/debug_ramdisk magisk --install-module /data/local/tmp/lsposed.zip' \| /system/xbin/su
  fi

  adb reboot
  wait_emu wait_for_boot

  # Run app tests
  run_content_cmd test
  adb shell echo 'su -c id' \| /system/xbin/su 2000 | tee /dev/fd/2 | grep -q 'uid=0'

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
  local img_dir="$ANDROID_SDK_ROOT/system-images/android-$ver/$type/$arch"
  local ramdisk="$img_dir/ramdisk.img"
  local features="$img_dir/advancedFeatures.ini"

  # Old Linux kernels will not boot with memory larger than 3GB
  local memory
  if [ $api -lt $huge_ram_min_api ]; then
    memory=3072
  else
    memory=8192
  fi

  local emu_args="$emu_args_base -memory $memory"

  # Setup emulator
  "$sdk" --channel=3 $pkg
  echo no | "$avd" create avd -f -n test -k $pkg

  # Launch stock emulator
  print_title "* Launching $pkg"
  restore_backup $ramdisk
  restore_backup $features
  "$emu" @test $emu_args &
  emu_pid=$!
  wait_emu wait_for_bootanim

  # Patch and test debug build
  ./build.py avd_patch -s "$ramdisk"
  kill -INT $emu_pid
  wait $emu_pid
  test_emu debug $api

  # Re-patch and test release build
  ./build.py -r avd_patch -s "$ramdisk"
  kill -INT $emu_pid
  wait $emu_pid
  test_emu release $api

  # Cleanup
  kill -INT $emu_pid
  wait $emu_pid
  restore_backup $ramdisk
  restore_backup $features
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
"$sdk" --channel=3 tools platform-tools emulator

if [ -n "$1" ]; then
  run_test $1 $2
else
  for api in $(seq 23 34); do
    run_test $api
  done
  # Android 15 Beta
  run_test 35 google_apis
  # Run 16k page tests
  run_test VanillaIceCream google_apis_ps16k
fi

"$avd" delete avd -n test

trap - EXIT
