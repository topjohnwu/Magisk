if [ -z $ANDROID_HOME ]; then
  export ANDROID_HOME=$ANDROID_SDK_ROOT
fi

export PATH="$PATH:$ANDROID_HOME/platform-tools"

emu="$ANDROID_HOME/emulator/emulator"
sdk="$ANDROID_HOME/cmdline-tools/latest/bin/sdkmanager"
avd="$ANDROID_HOME/cmdline-tools/latest/bin/avdmanager"
test_pkg='com.topjohnwu.magisk.test'

boot_timeout=600

core_count=$(nproc)
if [ $core_count -gt 8 ]; then
  core_count=8
fi

print_title() {
  echo -e "\n\033[44;39m${1}\033[0m\n"
}

print_error() {
  echo -e "\n\033[41;39m${1}\033[0m\n"
}

run_instrument_tests() {
  local out=$(adb shell am instrument -w \
    --user 0 \
    -e class "$1" \
    com.topjohnwu.magisk.test/androidx.test.runner.AndroidJUnitRunner)
  grep -q 'OK (' <<< "$out"
}

test_setup() {
  local variant=$1
  adb shell 'PATH=$PATH:/debug_ramdisk magisk -v'

  # Install the Magisk app
  adb install -r -g out/app-${variant}.apk

  # Install the test app
  adb install -r -g out/test-${variant}.apk

  # Run setup through the test app
  run_instrument_tests "$test_pkg.Environment#setupMagisk"
}

test_app() {
  # Run app tests
  run_instrument_tests "$test_pkg.MagiskAppTest"

  # Test shell su request
  run_instrument_tests "$test_pkg.Environment#setupShellGrantTest"
  adb shell /system/xbin/su 2000 su -c id | tee /dev/fd/2 | grep -q 'uid=0'
}
