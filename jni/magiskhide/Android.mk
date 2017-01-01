LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := magiskhide
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := main.c hide.c list_monitor.c proc_monitor.c util.c
LOCAL_CFLAGS += -std=gnu11 -O3
include $(BUILD_EXECUTABLE)