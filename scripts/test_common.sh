if [ -z $ANDROID_HOME ]; then
  export ANDROID_HOME=$ANDROID_SDK_ROOT
fi

# Make sure paths are consistent
export ANDROID_USER_HOME="$HOME/.android"
export ANDROID_EMULATOR_HOME="$ANDROID_USER_HOME"
export ANDROID_AVD_HOME="$ANDROID_EMULATOR_HOME/avd"
export PATH="$PATH:$ANDROID_HOME/platform-tools"

emu="$ANDROID_HOME/emulator/emulator"
sdk="$ANDROID_HOME/cmdline-tools/latest/bin/sdkmanager"
avd="$ANDROID_HOME/cmdline-tools/latest/bin/avdmanager"

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

# $1 = TestClass#method
# $2: boolean = isRepackaged
am_instrument() {
  local test_pkg
  if [ -n "$2" -a "$2" ]; then
    test_pkg="repackaged.com.topjohnwu.magisk.test"
  else
    test_pkg=com.topjohnwu.magisk.test
  fi
  local out=$(adb shell am instrument -w --user 0 -e class "$1" \
    "$test_pkg/com.topjohnwu.magisk.test.TestRunner")
  grep -q 'OK (' <<< "$out"
}

# $1 = pkg
wait_for_pm() {
  sleep 5
  adb shell pm uninstall $1 || true
}

run_setup() {
  local variant=$1
  adb shell 'PATH=$PATH:/debug_ramdisk magisk -v'

  # Install the Magisk app
  adb install -r -g out/app-${variant}.apk

  # Install the test app
  adb install -r -g out/test.apk

  # Run setup through the test app
  am_instrument '.Environment#setupMagisk'
  # Install LSPosed
  am_instrument '.Environment#setupLsposed'
}

run_tests() {
  # Run app tests
  am_instrument '.MagiskAppTest,.AdditionalTest'

  # Test shell su request
  am_instrument '.Environment#setupShellGrantTest'
  adb shell /system/xbin/su 2000 su -c id | tee /dev/fd/2 | grep -q 'uid=0'
  adb shell am force-stop com.topjohnwu.magisk

  # Test app hiding
  am_instrument '.Environment#setupAppHide'
  wait_for_pm com.topjohnwu.magisk

  # Make sure it still works
  am_instrument '.MagiskAppTest' true

  # Test shell su request
  am_instrument '.Environment#setupShellGrantTest' true
  adb shell /system/xbin/su 2000 su -c id | tee /dev/fd/2 | grep -q 'uid=0'
  adb shell am force-stop repackaged.com.topjohnwu.magisk

  # Test app restore
  am_instrument '.Environment#setupAppRestore' true
  wait_for_pm repackaged.com.topjohnwu.magisk

  # Make sure it still works
  am_instrument '.MagiskAppTest'
}
