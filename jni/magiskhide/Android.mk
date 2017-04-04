LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := magiskhide
LOCAL_SRC_FILES := magiskhide.c hide.c list_monitor.c proc_monitor.c util.c
LOCAL_CFLAGS := -DINDEP_BINARY
include $(BUILD_EXECUTABLE)