LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE:= libutils
LOCAL_C_INCLUDES := jni/include $(LIBUTILS)
LOCAL_SRC_FILES := \
	file.cpp \
	misc.cpp \
	selinux.cpp \
	logging.cpp \
	xwrap.cpp \
	CharArray.cpp

include $(BUILD_STATIC_LIBRARY)
