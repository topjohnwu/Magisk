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

boot_timeout=100

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
# $2 = component
am_instrument() {
  set +x
  local out=$(adb shell am instrument -w --user 0 -e class "$1" "$2")
  echo "$out"
  if grep -q 'OK (' <<< "$out"; then
    set -x
    return 0
  else
    set -x
    return 1
  fi
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

  local app='com.topjohnwu.magisk.test/com.topjohnwu.magisk.test.AppTestRunner'

  # Run setup through the test app
  am_instrument '.Environment#setupEnvironment' $app
}

run_tests() {
  local pkg='com.topjohnwu.magisk.test'
  local self="$pkg/$pkg.TestRunner"
  local app="$pkg/$pkg.AppTestRunner"
  local stub="repackaged.$pkg/$pkg.AppTestRunner"

  # Run app tests
  am_instrument '.MagiskAppTest,.AdditionalTest' $app

  # Test app hiding
  am_instrument '.AppMigrationTest#testAppHide' $self

  # Make sure it still works
  am_instrument '.MagiskAppTest' $stub

  # Test app restore
  am_instrument '.AppMigrationTest#testAppRestore' $self

  # Make sure it still works
  am_instrument '.MagiskAppTest' $app
}
