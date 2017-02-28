LOCAL_PATH := $(call my-dir)

include jni/magiskboot/Android.mk
include jni/magiskhide/Android.mk
include jni/resetprop/Android.mk
include jni/sepolicy-inject/Android.mk
include jni/su/Android.mk

# Libraries
include jni/selinux/libsepol/Android.mk
include jni/selinux/libselinux/Android.mk
include jni/ndk-compression/xz/src/liblzma/Android.mk
