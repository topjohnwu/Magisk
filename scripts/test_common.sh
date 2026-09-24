if [ -z $ANDROID_HOME ]; then
  export ANDROID_HOME=$ANDROID_SDK_ROOT
fi

# Make sure paths are consistent
export ANDROID_USER_HOME="$HOME/.android"
export ANDROID_EMULATOR_HOME="$ANDROID_USER_HOME"
export ANDROID_AVD_HOME="$ANDROID_EMULATOR_HOME/avd"
export PATH="$PATH:$ANDROID_HOME/platform-tools"

cmdline_tools="$ANDROID_HOME/cmdline-tools/latest"
android="$cmdline_tools/bin/android"

boot_timeout=100

core_count=$(nproc)
if [ $core_count -gt 8 ]; then
  core_count=8
fi

print_title() {
  echo -e "\n\033[44;39m${1}\033[0m\n"
}

print_error() {
  echo -e "\n\033[41;39m${1}\033[0m\n" >&2
}

ensure_android_cli() {
  local sdk="$cmdline_tools/bin/sdkmanager"
  if [ ! -x "$android" ]; then
    # Update to the latest cmdline-tools
    yes | "$sdk" --licenses > /dev/null 2>&1
    "$sdk" 'cmdline-tools;latest'
    # Rename cmdline-tools if updated
    if [ -e "${cmdline_tools}-2" ]; then
      rm -rf "$cmdline_tools"
      mv "${cmdline_tools}-2" "$cmdline_tools"
    fi
  fi
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

run_setup() {
  local apk=$1
  adb shell 'PATH=$PATH:/debug_ramdisk magisk -v'

  # Install the Magisk app
  adb install -r -g $apk

  # Install the test app
  adb install -r -g out/test.apk

  local app='com.topjohnwu.magisk.test/com.topjohnwu.magisk.test.AppTestRunner'

  # Run setup through the test app
  am_instrument '.Environment#setupEnvironment' $app
}

print_apks() {
  if [ "$#" -eq 0 ]; then
    find out -maxdepth 1 -type f -name "app-*.apk" -or -name "apk-*.apk"
  else
    echo "$@"
  fi
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
