LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := magiskhide
LOCAL_SRC_FILES := main.c hide.c list_monitor.c proc_monitor.c util.c
include $(BUILD_EXECUTABLE)