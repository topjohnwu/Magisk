#!/bin/bash

mydir="$(dirname $0)"

function freeze() {
  [[ $# == 1 ]] || {
    echo "usage: freeze.sh <level>"
    echo "e.g. To freeze framework manifest for Android R, run:"
    echo "  freeze.sh 5"
    return 1
  }
  local level="$1"
  [[ "${ANDROID_BUILD_TOP}" ]] || {
    echo "ANDROID_BUILD_TOP is not set; did you run envsetup.sh?"
    return 1
  }
  [[ "${ANDROID_HOST_OUT}" ]] || {
    echo "ANDROID_HOST_OUT is not set; did you run envsetup.sh?"
    return 1
  }

  local modules_to_build=check-vintf-all
  echo "Building ${modules_to_build}"
  "${ANDROID_BUILD_TOP}/build/soong/soong_ui.bash" --build-mode --all-modules --dir="$(pwd)" ${modules_to_build} || {
    echo "${modules_to_build} failed. Backwards compatibility might be broken."
    echo "Check framework manifest changes. If this is intentional, run "
    echo "  \`vintffm --update\` with appropriate options to update frozen files."
    return 1
  }

  echo "Updating level ${level}"
  "${ANDROID_HOST_OUT}/bin/vintffm" --update --level "${level}" --dirmap "/system:${ANDROID_PRODUCT_OUT}/system" "${mydir}/frozen" || return 1

  local files_to_diff="$(printf "${mydir}/frozen/%s\n" $(ls -1 -t -r ${mydir}/frozen | xargs -I{} basename {} | grep -B1 "${level}.xml"))"

  echo
  echo "Summary of changes:"
  echo diff ${files_to_diff}
  diff ${files_to_diff} || true
}

freeze $@
