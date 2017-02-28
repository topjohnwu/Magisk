LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := resetprop
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := resetprop.cpp system_properties.cpp libc_logging.cpp
LOCAL_LDLIBS += -latomic
include $(BUILD_EXECUTABLE)
