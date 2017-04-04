LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := magisk
LOCAL_STATIC_LIBRARIES := libselinux libsepol

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/utils \
	$(LOCAL_PATH)/selinux/libselinux/include \
	$(LOCAL_PATH)/selinux/libsepol/include

LOCAL_SRC_FILES := \
	main.c \
	utils/log_monitor.c \
	utils/vector.c \
	utils/xwrap.c

LOCAL_CFLAGS := -static
LOCAL_LDLIBS := -llog

include $(BUILD_EXECUTABLE)

# Libraries
include jni/selinux/libselinux/Android.mk
include jni/selinux/libsepol/Android.mk

# Enable these for seperate binaries
# By default, we create a unified binary

# include jni/magiskboot/Android.mk
# include jni/magiskhide/Android.mk
# include jni/resetprop/Android.mk
# include jni/magiskpolicy/Android.mk
# include jni/su/Android.mk
