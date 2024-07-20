#!/usr/bin/env bash

cvd_args='-daemon -enable_sandbox=false -report_anonymous_usage_stats=n'
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

install_bazel() {
  sudo apt-get install -y apt-transport-https curl gnupg
  curl -fsSL https://bazel.build/bazel-release.pub.gpg | gpg --dearmor >bazel-archive-keyring.gpg
  sudo mv bazel-archive-keyring.gpg /usr/share/keyrings
  echo "deb [arch=amd64 signed-by=/usr/share/keyrings/bazel-archive-keyring.gpg] https://storage.googleapis.com/bazel-apt stable jdk1.8" | sudo tee /etc/apt/sources.list.d/bazel.list
  sudo apt-get update && sudo apt-get install bazel zip unzip
}

build_cf() {
  git clone https://github.com/google/android-cuttlefish
  cd android-cuttlefish
  # We only want to build the base package
  sed -i '$ d' tools/buildutils/build_packages.sh
  tools/buildutils/build_packages.sh
  sudo dpkg -i ./cuttlefish-base_*_*64.deb || sudo apt-get install -f
  cd ../
  rm -rf android-cuttlefish
}

setup_env() {
  echo 'KERNEL=="kvm", GROUP="kvm", MODE="0666", OPTIONS+="static_node=kvm"' | sudo tee /etc/udev/rules.d/99-kvm4all.rules
  sudo udevadm control --reload-rules
  sudo udevadm trigger
  sudo usermod -aG kvm,cvdnetwork,render $USER
  yes | "$sdk" --licenses > /dev/null
  "$sdk" --channel=3 tools platform-tools
}

download_cf() {
  local build_id=$(curl -sL https://ci.android.com/builds/branches/aosp-main/status.json | \
    jq -r '.targets[] | select(.name == "aosp_cf_x86_64_phone-trunk_staging-userdebug") | .last_known_good_build')
  local sys_img_url="https://ci.android.com/builds/submitted/${build_id}/aosp_cf_x86_64_phone-trunk_staging-userdebug/latest/raw/aosp_cf_x86_64_phone-img-${build_id}.zip"
  local host_pkg_url="https://ci.android.com/builds/submitted/${build_id}/aosp_cf_x86_64_phone-trunk_staging-userdebug/latest/raw/cvd-host_package.tar.gz"

  print_title "* Download aosp-main ($build_id) system images"
  curl -L $sys_img_url -o aosp_cf_x86_64_phone-img.zip
  curl -LO $host_pkg_url
  rm -rf $CF_HOME
  mkdir -p $CF_HOME
  tar xvf cvd-host_package.tar.gz -C $CF_HOME
  unzip aosp_cf_x86_64_phone-img.zip -d $CF_HOME
  rm -f cvd-host_package.tar.gz aosp_cf_x86_64_phone-img.zip
}

test_cf() {
  local variant=$1

  run_cvd_bin stop_cvd || true

  print_title "* Testing $variant builds"
  timeout $boot_timeout bash -c "run_cvd_bin launch_cvd $cvd_args $magisk_args -resume=false"
  adb wait-for-device
  test_setup $variant

  adb reboot
  sleep 5
  run_cvd_bin stop_cvd || true

  timeout $boot_timeout bash -c "run_cvd_bin launch_cvd $cvd_args $magisk_args"
  adb wait-for-device
  test_app
}

run_test() {
  # Launch stock cuttlefish
  run_cvd_bin launch_cvd $cvd_args -resume=false
  adb wait-for-device

  # Patch and test debug build
  ./build.py avd_patch -s "$CF_HOME/init_boot.img" magisk_patched.img
  test_cf debug

  # Patch and test release build
  ./build.py -r avd_patch -s "$CF_HOME/init_boot.img" magisk_patched.img
  test_cf release

  # Cleanup
  run_cvd_bin stop_cvd || true
  rm -f magisk_patched.img*
}

set -xe
. scripts/test_common.sh

if [ -z $CF_HOME ]; then
  print_error "! Environment variable CF_HOME is required"
  exit 1
fi

case "$1" in
  setup )
    install_bazel
    build_cf
    setup_env
    ;;
  download )
    download_cf
    ;;
  test )
    trap cleanup EXIT
    export -f run_cvd_bin
    run_test
    trap - EXIT
    ;;
  * )
    exit 1
    ;;
esac
