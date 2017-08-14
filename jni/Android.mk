LOCAL_PATH := $(call my-dir)

########################
# Binaries
########################

# magisk main binary
include $(CLEAR_VARS)
LOCAL_MODULE := magisk
LOCAL_STATIC_LIBRARIES := libsepol
LOCAL_SHARED_LIBRARIES := libsqlite libselinux

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/include \
	$(LOCAL_PATH)/external \
	$(LOCAL_PATH)/selinux/libsepol/include

LOCAL_SRC_FILES := \
	daemon/magisk.c \
	daemon/daemon.c \
	daemon/socket_trans.c \
	daemon/log_monitor.c \
	daemon/bootstages.c \
	utils/misc.c \
	utils/vector.c \
	utils/xwrap.c \
	utils/list.c \
	utils/img.c \
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
LOCAL_CPPFLAGS := -std=c++11
LOCAL_LDLIBS := -llog
include $(BUILD_EXECUTABLE)

# magiskboot
include $(CLEAR_VARS)
LOCAL_MODULE := magiskboot
LOCAL_STATIC_LIBRARIES := libz liblzma liblz4 libbz2
LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/magiskboot \
	$(LOCAL_PATH)/include \
	$(LOCAL_PATH)/ndk-compression/zlib \
	$(LOCAL_PATH)/ndk-compression/xz/src/liblzma/api \
	$(LOCAL_PATH)/ndk-compression/lz4/lib \
	$(LOCAL_PATH)/ndk-compression/bzip2

LOCAL_SRC_FILES := \
	magiskboot/main.c \
	magiskboot/bootimg.c \
	magiskboot/hexpatch.c \
	magiskboot/compress.c \
	magiskboot/boot_utils.c \
	magiskboot/cpio.c \
	magiskboot/sha1.c \
	utils/xwrap.c \
	utils/vector.c \
	utils/list.c
LOCAL_CFLAGS := -DZLIB_CONST
include $(BUILD_EXECUTABLE)

# 32-bit static binaries
ifneq ($(TARGET_ARCH_ABI), x86_64)
ifneq ($(TARGET_ARCH_ABI), arm64-v8a)
# b64xz
include $(CLEAR_VARS)
LOCAL_MODULE := b64xz
LOCAL_STATIC_LIBRARIES := liblzma
LOCAL_C_INCLUDES := $(LOCAL_PATH)/ndk-compression/xz/src/liblzma/api
LOCAL_SRC_FILES := b64xz.c
LOCAL_LDFLAGS := -static
include $(BUILD_EXECUTABLE)
# Busybox
include jni/busybox/Android.mk
endif
endif

########################
# Libraries
########################

# External shared libraries, include stub libselinux and libsqlite
include jni/external/Android.mk

# libsepol, static library
include jni/selinux/libsepol/Android.mk

# Compression libraries for magiskboot
include jni/ndk-compression/zlib/Android.mk
include jni/ndk-compression/xz/src/liblzma/Android.mk
include jni/ndk-compression/lz4/lib/Android.mk
include jni/ndk-compression/bzip2/Android.mk
