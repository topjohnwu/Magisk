LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := magisk
LOCAL_STATIC_LIBRARIES := libsepol
LOCAL_SHARED_LIBRARIES := libsqlite libselinux

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/utils \
	$(LOCAL_PATH)/daemon \
	$(LOCAL_PATH)/resetprop \
	$(LOCAL_PATH)/magiskpolicy \
	$(LOCAL_PATH)/external \
	$(LOCAL_PATH)/selinux/libsepol/include

LOCAL_SRC_FILES := \
	main.c \
	utils/misc.c \
	utils/vector.c \
	utils/xwrap.c \
	utils/list.c \
	utils/img.c \
	daemon/daemon.c \
	daemon/socket_trans.c \
	daemon/log_monitor.c \
	daemon/bootstages.c \
	magiskhide/magiskhide.c \
	magiskhide/proc_monitor.c \
	magiskhide/hide_utils.c \
	magiskpolicy/magiskpolicy.c \
	magiskpolicy/rules.c \
	magiskpolicy/sepolicy.c \
	magiskpolicy/api.c \
	resetprop/resetprop.cpp \
	resetprop/system_properties.cpp \
	su/su.c \
	su/activity.c \
	su/db.c \
	su/misc.c \
	su/pts.c \
	su/su_daemon.c \
	su/su_socket.c

LOCAL_CFLAGS := -Wno-implicit-exception-spec-mismatch
LOCAL_LDLIBS := -llog

include $(BUILD_EXECUTABLE)

# External shared libraries, build stub libraries for linking
include jni/external/Android.mk

# libsepol, static library
include jni/selinux/libsepol/Android.mk

# Build magiskboot
include jni/magiskboot/Android.mk
