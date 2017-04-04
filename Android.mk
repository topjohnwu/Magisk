LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := magiskpolicy
LOCAL_STATIC_LIBRARIES := libsepol
LOCAL_SRC_FILES := magiskpolicy.c sepolicy.c rules.c utils.c ../utils/vector.c
LOCAL_C_INCLUDES := jni/selinux/libsepol/include jni/utils
LOCAL_CFLAGS := -DINDEP_BINARY
include $(BUILD_EXECUTABLE)

include jni/selinux/libsepol/Android.mk
