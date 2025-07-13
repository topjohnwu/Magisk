#!/usr/bin/env bash

set -xe
. scripts/test_common.sh

cvd_args="-daemon -enable_sandbox=false -memory_mb=8192 -report_anonymous_usage_stats=n -cpus=$core_count"
magisk_args='-init_boot_image=magisk_patched.img'

cleanup() {
  print_error "! An error occurred"
  run_cvd_bin stop_cvd || true
  rm -f magisk_patched.img*
}

run_cvd_bin() {
  local exe=$1
  shift
  HOME=$CF_HOME $CF_HOME/bin/$exe "$@"
}

setup_env() {
  curl -LO https://github.com/topjohnwu/magisk-files/releases/download/files/cuttlefish-base_1.2.0_amd64.deb
  sudo apt-get update
  sudo dpkg -i ./cuttlefish-base_*_*64.deb || sudo apt-get install -f
  rm cuttlefish-base_*_*64.deb
  echo 'KERNEL=="kvm", GROUP="kvm", MODE="0666", OPTIONS+="static_node=kvm"' | sudo tee /etc/udev/rules.d/99-kvm4all.rules
  sudo udevadm control --reload-rules
  sudo udevadm trigger
  sudo usermod -aG kvm,cvdnetwork,render $USER
  yes | "$sdk" --licenses > /dev/null
  "$sdk" --channel=3 platform-tools
  adb kill-server
  adb start-server
}

download_cf() {
  local branch=$1
  local device=$2

  if [ -z $branch ]; then
    branch='aosp-android-latest-release'
  fi
  if [ -z $device ]; then
    device='aosp_cf_x86_64_only_phone'
  fi
  local target="${device}-userdebug"

  local build_id=$(curl -sL https://ci.android.com/builds/branches/${branch}/status.json | \
    jq -r ".targets[] | select(.name == \"$target\") | .last_known_good_build")
  local sys_img_url="https://ci.android.com/builds/submitted/${build_id}/${target}/latest/raw/${device}-img-${build_id}.zip"
  local host_pkg_url="https://ci.android.com/builds/submitted/${build_id}/${target}/latest/raw/cvd-host_package.tar.gz"

  print_title "* Download $target ($build_id) images"
  curl -L $sys_img_url -o aosp_cf_phone-img.zip
  curl -LO $host_pkg_url
  rm -rf $CF_HOME
  mkdir -p $CF_HOME
  tar xvf cvd-host_package.tar.gz -C $CF_HOME
  unzip aosp_cf_phone-img.zip -d $CF_HOME
  rm -f cvd-host_package.tar.gz aosp_cf_phone-img.zip
}

test_cf() {
  local variant=$1

  run_cvd_bin stop_cvd || true

  print_title "* Testing $variant builds"
  timeout $boot_timeout bash -c "run_cvd_bin launch_cvd $cvd_args $magisk_args -resume=false"
  adb wait-for-device
  run_setup $variant

  adb reboot
  sleep 5
  run_cvd_bin stop_cvd || true

  timeout $boot_timeout bash -c "run_cvd_bin launch_cvd $cvd_args $magisk_args"
  adb wait-for-device
  run_tests
}

test_main() {
  # Launch stock cuttlefish
  run_cvd_bin launch_cvd $cvd_args -resume=false
  adb wait-for-device

  # Patch and test debug build
  ./build.py -v avd_patch "$CF_HOME/init_boot.img" magisk_patched.img
  test_cf debug

  # Patch and test release build
  ./build.py -vr avd_patch "$CF_HOME/init_boot.img" magisk_patched.img
  test_cf release

  # Cleanup
  run_cvd_bin stop_cvd || true
  rm -f magisk_patched.img*
}

if [ -z $CF_HOME ]; then
  print_error "! Environment variable CF_HOME is required"
  exit 1
fi

case "$1" in
  setup )
    setup_env
    ;;
  download )
    download_cf $2 $3
    ;;
  test )
    trap cleanup EXIT
    export -f run_cvd_bin
    test_main
    trap - EXIT
    ;;
  * )
    exit 1
    ;;
esac
