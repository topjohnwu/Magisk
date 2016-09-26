my_path := $(call my-dir)

LOCAL_PATH := $(my_path)

include $(CLEAR_VARS)
LOCAL_MODULE := hidesu
LOCAL_MODULE_TAGS := optional
LOCAL_FORCE_STATIC_EXECUTABLE := true
LOCAL_LDFLAGS := -static
LOCAL_STATIC_LIBRARIES := libc libcutils
LOCAL_SRC_FILES := hidesu.c
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE := bootimgtools
LOCAL_MODULE_TAGS := optional
LOCAL_FORCE_STATIC_EXECUTABLE := true
LOCAL_LDFLAGS := -static
LOCAL_STATIC_LIBRARIES := libc libcutils
LOCAL_SRC_FILES := bootimgtools.c extract.c repack.c hexpatch.c
LOCAL_CFLAGS += -std=gnu11
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE := sepolicy-inject
LOCAL_MODULE_TAGS := optional
LOCAL_FORCE_STATIC_EXECUTABLE := true
LOCAL_LDFLAGS := -static
LOCAL_STATIC_LIBRARIES := libc libcutils libsepol
LOCAL_SRC_FILES := sepolicy-inject/sepolicy-inject.c sepolicy-inject/builtin_rules.c
LOCAL_C_INCLUDES := selinux/libsepol/include/
LOCAL_CFLAGS += -std=gnu11
include $(BUILD_EXECUTABLE)

include selinux/libsepol/Android.mk
