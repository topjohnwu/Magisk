LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE:= libutils
LOCAL_C_INCLUDES := jni/include $(LIBUTILS)
LOCAL_SRC_FILES := \
	file.c \
	list.c \
	misc.c \
	vector.c \
	selinux.c \
	logging.c \
	xwrap.c

include $(BUILD_STATIC_LIBRARY)
