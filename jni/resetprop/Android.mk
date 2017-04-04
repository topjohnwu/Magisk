LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := resetprop
LOCAL_SRC_FILES := resetprop.cpp system_properties.cpp libc_logging.cpp
LOCAL_LDLIBS += -latomic
LOCAL_CFLAGS := -Wno-implicit-exception-spec-mismatch -DINDEP_BINARY
include $(BUILD_EXECUTABLE)
