LOCAL_PATH := $(call my-dir)

# Some handy paths
EXT_PATH := jni/external
SE_PATH := $(EXT_PATH)/selinux
LIBSELINUX := $(SE_PATH)/libselinux/include
LIBSEPOL := $(SE_PATH)/libsepol/include $(SE_PATH)/libsepol/cil/include
LIBLZMA := $(EXT_PATH)/xz/src/liblzma/api
LIBLZ4 := $(EXT_PATH)/lz4/lib
LIBBZ2 := $(EXT_PATH)/bzip2
LIBFDT := $(EXT_PATH)/dtc/libfdt
LIBNANOPB := $(EXT_PATH)/nanopb
LIBSYSTEMPROPERTIES := jni/resetprop/libsystemproperties/include
COMMON_UTILS := \
	utils/file.c \
	utils/list.c \
	utils/misc.c \
	utils/vector.c \
	utils/selinux.c \
	utils/logging.c \
	utils/xwrap.c

########################
# Binaries
########################

ifdef B_MAGISK

# magisk main binary
include $(CLEAR_VARS)
LOCAL_MODULE := magisk
LOCAL_SHARED_LIBRARIES := libsqlite
LOCAL_STATIC_LIBRARIES := libnanopb libsystemproperties
LOCAL_C_INCLUDES := \
	jni/include \
	$(EXT_PATH)/include \
	$(LIBNANOPB) \
	$(LIBSYSTEMPROPERTIES)

LOCAL_SRC_FILES := \
	core/magisk.c \
	core/daemon.c \
	core/log_daemon.c \
	core/bootstages.c \
	core/socket.c \
	core/db.c \
	magiskhide/magiskhide.c \
	magiskhide/proc_monitor.c \
	magiskhide/hide_utils.c \
	resetprop/persist_properties.c \
	resetprop/resetprop.c \
	resetprop/system_property_api.cpp \
	resetprop/system_property_set.cpp \
	su/su.c \
	su/connect.c \
	su/pts.c \
	su/su_daemon.c \
	utils/img.c \
	$(COMMON_UTILS)

LOCAL_CFLAGS := -DIS_DAEMON
LOCAL_LDLIBS := -llog
include $(BUILD_EXECUTABLE)

endif

ifdef B_INIT

# magiskinit
include $(CLEAR_VARS)
LOCAL_MODULE := magiskinit
LOCAL_STATIC_LIBRARIES := libsepol libxz
LOCAL_C_INCLUDES := \
	jni/include \
	jni/magiskpolicy \
	$(EXT_PATH)/include \
	out \
	out/$(TARGET_ARCH_ABI) \
	$(LIBSEPOL)

LOCAL_SRC_FILES := \
	core/magiskinit.c \
	magiskpolicy/api.c \
	magiskpolicy/magiskpolicy.c \
	magiskpolicy/rules.c \
	magiskpolicy/sepolicy.c \
	$(COMMON_UTILS)

LOCAL_LDFLAGS := -static
include $(BUILD_EXECUTABLE)

endif

ifdef B_BOOT

# magiskboot
include $(CLEAR_VARS)
LOCAL_MODULE := magiskboot
LOCAL_STATIC_LIBRARIES := libmincrypt liblzma liblz4 libbz2 libfdt
LOCAL_C_INCLUDES := \
	jni/include \
	$(EXT_PATH)/include \
	$(LIBLZMA) \
	$(LIBLZ4) \
	$(LIBBZ2) \
	$(LIBFDT)

LOCAL_SRC_FILES := \
	magiskboot/cpio.c \
	magiskboot/main.c \
	magiskboot/bootimg.c \
	magiskboot/hexpatch.c \
	magiskboot/compress.c \
	magiskboot/format.c \
	magiskboot/dtb.c \
	magiskboot/ramdisk.c \
	magiskboot/pattern.c \
	$(COMMON_UTILS)

LOCAL_CFLAGS := -DXWRAP_EXIT
LOCAL_LDLIBS := -lz
include $(BUILD_EXECUTABLE)

endif

ifdef B_BXZ

# b64xz
include $(CLEAR_VARS)
LOCAL_MODULE := b64xz
LOCAL_STATIC_LIBRARIES := libxz
LOCAL_C_INCLUDES := $(EXT_PATH)/include
LOCAL_SRC_FILES := b64xz.c
LOCAL_LDFLAGS := -static
include $(BUILD_EXECUTABLE)

endif

ifdef B_BB

# Busybox
include jni/external/busybox/Android.mk

endif

########################
# Libraries
########################
include jni/external/Android.mk
include jni/resetprop/libsystemproperties/Android.mk
