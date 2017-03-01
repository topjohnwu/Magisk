LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := magiskboot
LOCAL_STATIC_LIBRARIES := liblzma liblz4 libbz2
LOCAL_C_INCLUDES := \
	jni/ndk-compression/xz/src/liblzma/api/ \
	jni/ndk-compression/lz4/lib/ \
	jni/ndk-compression/bzip2

LOCAL_SRC_FILES := main.c unpack.c repack.c hexpatch.c parseimg.c compress.c
LOCAL_LDLIBS += -lz
include $(BUILD_EXECUTABLE)

include jni/ndk-compression/xz/src/liblzma/Android.mk
include jni/ndk-compression/lz4/lib/Android.mk
include jni/ndk-compression/bzip2/Android.mk
