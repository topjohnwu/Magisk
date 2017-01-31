LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := sepolicy-inject
LOCAL_MODULE_TAGS := optional
LOCAL_STATIC_LIBRARIES := libsepol
LOCAL_SRC_FILES := main.c sepolicy.c rules.c utils.c
LOCAL_C_INCLUDES := jni/selinux/libsepol/include/
LOCAL_CFLAGS += -std=gnu11
include $(BUILD_EXECUTABLE)
