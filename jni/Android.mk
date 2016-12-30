LOCAL_PATH := $(call my-dir)

include jni/bootimgtools/Android.mk
include jni/magiskhide/Android.mk
include jni/resetprop/Android.mk
include jni/sepolicy-inject/Android.mk
include jni/su/Android.mk

include jni/selinux/libsepol/Android.mk
include jni/selinux/libselinux/Android.mk
