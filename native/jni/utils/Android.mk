LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE:= libutils
LOCAL_C_INCLUDES := jni/include $(LIBUTILS)
LOCAL_SRC_FILES := \
	new.cpp \
	file.cpp \
	misc.cpp \
	selinux.cpp \
	logging.cpp \
	cpio.cpp \
	xwrap.cpp

include $(BUILD_STATIC_LIBRARY)
