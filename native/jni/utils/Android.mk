LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE:= libutils
LOCAL_C_INCLUDES := jni/include $(LOCAL_PATH)/include
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/include
LOCAL_SRC_FILES := \
	missing.cpp \
	new.cpp \
	files.cpp \
	misc.cpp \
	selinux.cpp \
	logging.cpp \
	cpio.cpp \
	xwrap.cpp \
	stream.cpp

include $(BUILD_STATIC_LIBRARY)
