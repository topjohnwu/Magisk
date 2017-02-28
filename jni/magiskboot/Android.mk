LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := magiskboot
LOCAL_STATIC_LIBRARIES := liblzma
LOCAL_SRC_FILES := main.c unpack.c repack.c hexpatch.c parseimg.c compress.c
LOCAL_LDLIBS += -lz
include $(BUILD_EXECUTABLE)
