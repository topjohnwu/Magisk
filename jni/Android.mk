LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := magisk
LOCAL_STATIC_LIBRARIES := libsepol
LOCAL_SHARED_LIBRARIES := libsqlite libselinux

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/utils \
	$(LOCAL_PATH)/selinux/libselinux/include \
	$(LOCAL_PATH)/selinux/libsepol/include \
	$(LOCAL_PATH)/sqlite3

LOCAL_SRC_FILES := \
	main.c \
	utils/misc.c \
	utils/vector.c \
	utils/xwrap.c \
	utils/log_monitor.c \
	magiskhide/magiskhide.c \
	magiskhide/hide_daemon.c \
	magiskhide/proc_monitor.c \
	magiskpolicy/magiskpolicy.c \
	magiskpolicy/rules.c \
	magiskpolicy/sepolicy.c \
	magiskpolicy/utils.c \
	resetprop/resetprop.cpp \
	resetprop/libc_logging.cpp \
	resetprop/system_properties.cpp \
	su/su.c \
	su/daemon.c \
	su/activity.c \
	su/db.c \
	su/utils.c \
	su/pts.c

LOCAL_CFLAGS := -Wno-implicit-exception-spec-mismatch
LOCAL_LDLIBS := -llog

include $(BUILD_EXECUTABLE)

# Libraries
include jni/selinux/libselinux/Android.mk
include jni/selinux/libsepol/Android.mk
include jni/sqlite3/Android.mk

#####################################################################
# In order to build separate binaries, please comment out everything 
# starting from line 3 (including the 3 lines for libraries)
# Then, uncomment the line you want below
#####################################################################
# include jni/resetprop/Android.mk
# include jni/magiskpolicy/Android.mk

# Build magiskboot
# include jni/magiskboot/Android.mk
