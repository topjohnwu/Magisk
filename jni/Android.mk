LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := magiskhide
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := magiskhide.c
LOCAL_CFLAGS += -std=gnu11
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE := bootimgtools
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := bootimgtools/main.c bootimgtools/extract.c bootimgtools/repack.c bootimgtools/hexpatch.c
LOCAL_CFLAGS += -std=gnu11
include $(BUILD_EXECUTABLE)

include jni/resetprop/Android.mk
include jni/sepolicy-inject/Android.mk
include jni/su/Android.mk

include jni/selinux/libsepol/Android.mk
include jni/selinux/libselinux/Android.mk
