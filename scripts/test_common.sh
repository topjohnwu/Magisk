if [ -z $ANDROID_HOME ]; then
  export ANDROID_HOME=$ANDROID_SDK_ROOT
fi

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

run_content_cmd() {
  while true; do
    local out=$(adb shell /system/xbin/su 0 content call --uri content://com.topjohnwu.magisk.provider --method $1 | tee /dev/fd/2)
    if ! grep -q 'Bundle\[' <<< "$out"; then
      # The call failed, wait a while and retry later
      sleep 30
    else
      grep -q 'result=true' <<< "$out"
      return $?
    fi
  done
}

test_setup() {
  local variant=$1
  adb shell 'PATH=$PATH:/debug_ramdisk magisk -v'

  # Install the Magisk app
  adb install -r -g out/app-${variant}.apk

  # Use the app to run setup and reboot
  run_content_cmd setup
}

test_app() {
  # Run app tests
  run_content_cmd test
  adb shell /system/xbin/su 2000 su -c id | tee /dev/fd/2 | grep -q 'uid=0'
}
