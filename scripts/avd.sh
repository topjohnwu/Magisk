#!/usr/bin/env bash

set -e
shopt -s extglob
. scripts/test_common.sh

emu_args_base="-no-window -no-audio -no-boot-anim -gpu software -read-only -no-snapshot -cores $core_count"
log_args="-show-kernel -logcat '' -logcat-output logcat.log"
emu_args=

atd_min_api=30
atd_max_api=36
huge_ram_min_api=26

case $(uname -m) in
  'arm64'|'aarch64')
    if [ -n "$FORCE_32_BIT" ]; then
      echo "! ARM32 is not supported"
      exit 1
    fi
    arch=arm64-v8a
    ;;
  *)
    if [ -n "$FORCE_32_BIT" ]; then
      arch=x86
    else
      arch=x86_64
    fi

    ;;
esac

cleanup() {
  rm -f magisk-*.img
  "$avd" delete avd -n test > /dev/null 2>&1
}

test_error() {
  trap - EXIT
  print_error "! An error occurred"
  pkill -INT -P $$
  wait
  cleanup
  exit 1
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

dump_vars() {
  local val
  for name in $@; do
    eval val=\$$name
    echo local $name=\"$val\"\;
  done
}

parse_args() {
  set +x
  local return_vals="$1"
  shift

  local ver=
  local type=

  while getopts ":v:t:l" opt; do
    case $opt in
      v )
        ver="$OPTARG"
        ;;
      t )
        type="$OPTARG"
        ;;
      l )
        export AVD_TEST_LOG=1
        ;;
      \? )
        echo "Error: Invalid option: -$OPTARG" 1>&2
        exit 1
        ;;
      : )
        # Missing a required argument is fine as we perform validations later
        ;;
    esac
  done

  if [ -z $ver ]; then
    print_error "! No system image version specified"
    exit 1
  fi

  # Determine API level
  local api
  case $ver in
    +([0-9\.])) api=$ver ;;
    TiramisuPrivacySandbox) api=33 ;;
    UpsideDownCakePrivacySandbox) api=34 ;;
    VanillaIceCream) api=35 ;;
    Baklava) api=36 ;;
    CinnamonBun) api=37 ;;
    *CANARY) api=10000 ;;
    *)
      print_error "! Unknown system image version '$ver'"
      exit 1
      ;;
  esac

  # Determine default image type
  if [ -z $type ]; then
    if [ $(bc <<< "$api >= $atd_min_api && $api <= $atd_max_api") = 1 ]; then
      # Use the lightweight ATD images if possible
      type='aosp_atd'
    elif [ $(bc <<< "$api > $atd_max_api") = 1 ]; then
      # Preview/beta release, no AOSP version available
      type='google_apis'
    else
      type='default'
    fi
  fi

  # Old Linux kernels will not boot with memory larger than 3GB
  local memory
  if [ $(bc <<< "$api < $huge_ram_min_api") = 1 ]; then
    memory=3072
  else
    memory=8192
  fi

  emu_args="$emu_args_base -memory $memory"

  # System image variable and paths
  local avd_pkg="system-images;android-$ver;$type;$arch"
  local sys_img_dir="$ANDROID_HOME/system-images/android-$ver/$type/$arch"
  local ramdisk="$sys_img_dir/ramdisk.img"

  # Dump global variables
  echo emu_args=\"$emu_args\"
  echo OPTIND=$OPTIND

  # Dump local variables
  dump_vars $return_vals
}

dl_emu() {
  local avd_pkg=$1
  yes | "$sdk" --licenses > /dev/null 2>&1
  "$sdk" --channel=3 'cmdline-tools;latest' platform-tools emulator "$avd_pkg"
  # It's possible cmdline-tools updated
  if [ -e "${cli_tools}-2" ]; then
    rm -rf "$cli_tools"
    mv "${cli_tools}-2" "$cli_tools"
  fi
}

setup_emu() {
  local avd_pkg=$1
  local ver=$2
  dl_emu $avd_pkg
  echo no | "$avd" create avd -f -n test -k $avd_pkg
}

test_emu() {
  local apk=$1
  local image=$2

  local magisk_args="-ramdisk $image -feature -SystemAsRoot"

  if [ -n "$AVD_TEST_LOG" ]; then
    rm -f logcat.log
    "$emu" @test $emu_args $log_args $magisk_args > kernel.log 2>&1 &
  else
    "$emu" @test $emu_args $magisk_args > /dev/null 2>&1 &
  fi
  timeout $boot_timeout bash -c wait_for_boot

  run_setup $apk

  adb reboot
  timeout $boot_timeout bash -c wait_for_boot

  run_tests

  adb emu kill
  wait
}

test_main() {
  local vars
  vars=$(parse_args "ver avd_pkg ramdisk" "$@")
  eval "$vars"

  # Shift off the options we just parsed so that "$@" only contains the remaining positional arguments
  shift $((OPTIND - 1))
  local apks=($(print_apks "$@"))

  # Specify an explicit port so that tests can run with other emulators running at the same time
  local emu_port=5682
  emu_args="$emu_args -port $emu_port"
  export ANDROID_SERIAL="emulator-$emu_port"

  setup_emu "$avd_pkg" $ver

  # Restart ADB daemon just in case
  adb kill-server
  adb start-server

  # Launch stock emulator
  print_title "* Launching $avd_pkg"
  "$emu" @test $emu_args > /dev/null 2>&1 &
  timeout $boot_timeout bash -c wait_for_boot

  # Patch images
  local images=()
  for apk in "${apks[@]}"; do
    images+=("magisk-$(basename $apk .apk).img")
    ./build.py -v avd_patch --apk "$apk" "$ramdisk" "${images[-1]}"
  done

  adb emu kill
  wait

  for i in "${!apks[@]}"; do
    print_title "* Testing $avd_pkg ($(basename ${apks[i]}))"
    test_emu ${apks[i]} ${images[i]}
  done

  cleanup
}

run_main() {
  local vars
  vars=$(parse_args "ver avd_pkg" "$@")
  eval "$vars"

  setup_emu "$avd_pkg" $ver
  print_title "* Launching $avd_pkg"
  "$emu" @test $emu_args > /dev/null 2>&1 &
  wait_for_boot
  cleanup
}

dl_main() {
  local vars
  vars=$(parse_args "avd_pkg" "$@")
  eval "$vars"

  print_title "* Downloading $avd_pkg"
  dl_emu "$avd_pkg"
}

case "$1" in
  test )
    shift
    trap test_error EXIT
    export -f wait_for_boot
    set -x
    test_main "$@"
    ;;
  run )
    shift
    trap cleanup EXIT
    run_main "$@"
    ;;
  dl )
    shift
    dl_main "$@"
    ;;
  * )
    print_error "Unknown argument '$1'"
    exit 1
    ;;
esac

# Exit normally, don't run through cleanup again
trap - EXIT
