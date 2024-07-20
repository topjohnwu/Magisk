export PATH="$PATH:$ANDROID_SDK_ROOT/platform-tools"

sdk="$ANDROID_SDK_ROOT/cmdline-tools/latest/bin/sdkmanager"
boot_timeout=600

print_title() {
  echo -e "\n\033[44;39m${1}\033[0m\n"
}

print_error() {
  echo -e "\n\033[41;39m${1}\033[0m\n"
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
  adb shell echo 'su -c id' \| /system/xbin/su 2000 | tee /dev/fd/2 | grep -q 'uid=0'
}
