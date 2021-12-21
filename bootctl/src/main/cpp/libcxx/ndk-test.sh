#!/bin/bash

# This file should reside in test/, but it seems that if there is already one
# lit.site.cfg in the test/ directory it is impossible to tell LIT to use
# another. This will need to be fixed upstream before this can get a proper
# home. The downside of this is that there isn't a way to run a subset of the
# libc++ tests against the NDK.
if [ -z "$ANDROID_PRODUCT_OUT" ]; then
  >&2 echo "Error: ANDROID_PRODUCT_OUT is not set. Have you run lunch?"
  exit 1
fi

if [ ! -f $ANDROID_PRODUCT_OUT/system/lib/libc++_ndk.so ]; then
  >&2 echo "Error: libc++_ndk.so has not been built for this target."
  exit 1
fi

adb push $ANDROID_PRODUCT_OUT/system/lib/libc++_ndk.so /data/local/tmp
lit -sv $* .
