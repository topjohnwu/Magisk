#!/bin/bash
# Script to update Android make-files for HAL modules.

set -e

if [ -z "$ANDROID_BUILD_TOP" ]; then
    echo "Missing ANDROID_BUILD_TOP env variable. Run 'lunch' first."
    exit 1
fi

source $ANDROID_BUILD_TOP/system/tools/hidl/update-makefiles-helper.sh

do_makefiles_update \
  "android.hidl:system/libhidl/transport"
