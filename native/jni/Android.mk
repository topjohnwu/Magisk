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
LIBSYSTEMPROPERTIES := jni/systemproperties/include
LIBUTILS := jni/utils/include

########################
# Binaries
########################

ifdef B_MAGISK

include $(CLEAR_VARS)
LOCAL_MODULE := magisk
LOCAL_SHARED_LIBRARIES := libsqlite
LOCAL_STATIC_LIBRARIES := libnanopb libsystemproperties libutils
LOCAL_C_INCLUDES := \
	jni/include \
	$(EXT_PATH)/include \
	$(LIBNANOPB) \
	$(LIBSYSTEMPROPERTIES) \
	$(LIBUTILS)

LOCAL_SRC_FILES := \
	core/applets.cpp \
	core/magisk.cpp \
	core/daemon.cpp \
	core/bootstages.cpp \
	core/socket.cpp \
	core/db.cpp \
	core/scripting.cpp \
	magiskhide/magiskhide.cpp \
	magiskhide/proc_monitor.cpp \
	magiskhide/hide_utils.cpp \
	resetprop/persist_properties.cpp \
	resetprop/resetprop.cpp \
	resetprop/system_property_api.cpp \
	resetprop/system_property_set.cpp \
	su/su.cpp \
	su/connect.cpp \
	su/pts.cpp \
	su/su_daemon.cpp

LOCAL_LDLIBS := -llog
include $(BUILD_EXECUTABLE)

endif

include $(CLEAR_VARS)

ifdef B_INIT
LOCAL_MODULE := magiskinit
BB_INIT := 1
else ifdef B_INIT64
LOCAL_MODULE := magiskinit64
LOCAL_CPPFLAGS += -DUSE_64BIT
BB_INIT := 1
endif

ifdef BB_INIT

LOCAL_STATIC_LIBRARIES := libsepol libxz libutils
LOCAL_C_INCLUDES := \
	jni/include \
	jni/magiskpolicy \
	$(EXT_PATH)/include \
	out \
	out/$(TARGET_ARCH_ABI) \
	$(LIBSEPOL) \
	$(LIBUTILS)

LOCAL_SRC_FILES := \
	core/init.cpp \
	magiskpolicy/api.cpp \
	magiskpolicy/magiskpolicy.cpp \
	magiskpolicy/rules.cpp \
	magiskpolicy/policydb.cpp \
	magiskpolicy/sepolicy.c

LOCAL_LDFLAGS := -static
include $(BUILD_EXECUTABLE)

endif

ifdef B_BOOT

include $(CLEAR_VARS)
LOCAL_MODULE := magiskboot
LOCAL_STATIC_LIBRARIES := libmincrypt liblzma liblz4 libbz2 libfdt libutils
LOCAL_C_INCLUDES := \
	jni/include \
	$(EXT_PATH)/include \
	$(LIBLZMA) \
	$(LIBLZ4) \
	$(LIBBZ2) \
	$(LIBFDT) \
	$(LIBUTILS)

LOCAL_SRC_FILES := \
	magiskboot/main.cpp \
	magiskboot/bootimg.cpp \
	magiskboot/hexpatch.cpp \
	magiskboot/compress.cpp \
	magiskboot/format.cpp \
	magiskboot/dtb.cpp \
	magiskboot/ramdisk.cpp \
	magiskboot/pattern.cpp

LOCAL_LDLIBS := -lz
LOCAL_LDFLAGS := -static
include $(BUILD_EXECUTABLE)

endif

ifdef B_TEST

include $(CLEAR_VARS)
LOCAL_MODULE := test
LOCAL_STATIC_LIBRARIES := libutils
LOCAL_C_INCLUDES := \
	jni/include \
	$(LIBUTILS)
LOCAL_SRC_FILES := test.cpp
include $(BUILD_EXECUTABLE)

endif

ifdef B_BB

include jni/external/busybox/Android.mk

endif

########################
# Libraries
########################
include jni/utils/Android.mk
include jni/systemproperties/Android.mk
include jni/external/Android.mk
