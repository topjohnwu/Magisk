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
UTIL_SRC := utils/cpio.c \
            utils/file.c \
            utils/img.c \
            utils/list.c \
            utils/misc.c \
            utils/pattern.c \
            utils/vector.c \
            utils/xwrap.c

########################
# Binaries
########################

ifdef B_MAGISK

# magisk main binary
include $(CLEAR_VARS)
LOCAL_MODULE := magisk
LOCAL_SHARED_LIBRARIES := libsqlite libselinux
LOCAL_STATIC_LIBRARIES := libnanopb
LOCAL_C_INCLUDES := \
	jni/include \
	jni/magiskpolicy \
	$(EXT_PATH)/include \
	$(LIBSELINUX) \
	$(LIBNANOPB)

LOCAL_SRC_FILES := \
	core/magisk.c \
	core/daemon.c \
	core/log_monitor.c \
	core/bootstages.c \
	core/socket.c \
	magiskhide/magiskhide.c \
	magiskhide/proc_monitor.c \
	magiskhide/hide_utils.c \
	resetprop/persist_props.c \
	resetprop/resetprop.c \
	resetprop/system_properties.cpp \
	su/su.c \
	su/activity.c \
	su/db.c \
	su/pts.c \
	su/su_daemon.c \
	su/su_socket.c \
	$(UTIL_SRC)

LOCAL_CFLAGS := -DIS_DAEMON -DSELINUX ${MAGISK_DEBUG} \
	-DMAGISK_VERSION="${MAGISK_VERSION}" -DMAGISK_VER_CODE=${MAGISK_VER_CODE} 
LOCAL_LDLIBS := -llog
include $(BUILD_EXECUTABLE)

endif

ifdef B_INIT

# magiskinit
include $(CLEAR_VARS)
LOCAL_MODULE := magiskinit
LOCAL_STATIC_LIBRARIES := libsepol liblzma
LOCAL_C_INCLUDES := \
	jni/include \
	jni/magiskpolicy \
	out/$(TARGET_ARCH_ABI) \
	$(LIBSEPOL) \
	$(LIBLZMA)

LOCAL_SRC_FILES := \
	core/magiskinit.c \
	magiskpolicy/api.c \
	magiskpolicy/magiskpolicy.c \
	magiskpolicy/rules.c \
	magiskpolicy/sepolicy.c \
	$(UTIL_SRC)

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
	magiskboot/main.c \
	magiskboot/bootimg.c \
	magiskboot/hexpatch.c \
	magiskboot/compress.c \
	magiskboot/format.c \
	magiskboot/dtb.c \
	magiskboot/ramdisk.c \
	$(UTIL_SRC)

LOCAL_CFLAGS := -DXWRAP_EXIT
LOCAL_LDLIBS := -lz
include $(BUILD_EXECUTABLE)

endif

ifdef B_BXZ

# b64xz
include $(CLEAR_VARS)
LOCAL_MODULE := b64xz
LOCAL_STATIC_LIBRARIES := liblzma
LOCAL_C_INCLUDES := $(LIBLZMA)
LOCAL_SRC_FILES := b64xz.c
LOCAL_LDFLAGS := -static
include $(BUILD_EXECUTABLE)

endif

ifdef B_BB

# Busybox
include jni/external/busybox/Android.mk

endif

########################
# Externals
########################
include jni/external/Android.mk
