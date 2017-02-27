LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := bootimgtools
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := main.c unpack.c repack.c hexpatch.c parseimg.c compress.c
LOCAL_CFLAGS += -std=gnu11
LOCAL_LDLIBS += -lz
include $(BUILD_EXECUTABLE)
