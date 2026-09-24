LOCAL_PATH := $(call my-dir)
include $(LOCAL_PATH)/../../scripts/ndk-modules/init.mk

########################
# Binaries
########################

ifdef B_MAGISK

include $(CLEAR_VARS)
LOCAL_MODULE := magisk
LOCAL_STATIC_LIBRARIES := libcore
LOCAL_SRC_FILES := core/applets.cpp

LOCAL_LDLIBS := -llog
LOCAL_LDFLAGS := -Wl,--dynamic-list=src/exported_sym.txt

include $(BUILD_EXECUTABLE_MODULE)

endif

ifdef B_PRELOAD

include $(CLEAR_VARS)
LOCAL_MODULE := init-ld
LOCAL_SRC_FILES := init/preload.c
LOCAL_LDFLAGS := -Wl,--strip-all
include $(BUILD_SHARED_LIBRARY)

endif

ifdef B_INIT

include $(CLEAR_VARS)
LOCAL_MODULE := magiskinit
LOCAL_STATIC_LIBRARIES := \
    libbase \
    libpolicy \
    libxz \
    libinit-rs

LOCAL_MODULE_SRC_FILES := init/getinfo.cpp
LOCAL_SRC_FILES := \
    init/mount.cpp \
    init/rootdir.cpp \
    init/init-rs.cpp

LOCAL_LDFLAGS := -static

ifdef B_CRT0
LOCAL_STATIC_LIBRARIES += crt0
LOCAL_LDFLAGS += -Wl,--defsym=vfprintf=tiny_vfprintf
endif

include $(BUILD_EXECUTABLE_MODULE)

endif

ifdef B_BOOT

include $(CLEAR_VARS)
LOCAL_MODULE := magiskboot
LOCAL_STATIC_LIBRARIES := \
    libbase \
    liblz4 \
    libboot-rs

LOCAL_MODULE_SRC_FILES := boot/bootimg.cpp
LOCAL_SRC_FILES := boot/boot-rs.cpp

LOCAL_LDFLAGS := -static

ifdef B_CRT0
LOCAL_STATIC_LIBRARIES += crt0
LOCAL_LDFLAGS += -lm -Wl,--defsym=vfprintf=musl_vfprintf
endif

include $(BUILD_EXECUTABLE_MODULE)

endif

ifdef B_POLICY

include $(CLEAR_VARS)
LOCAL_MODULE := magiskpolicy
LOCAL_STATIC_LIBRARIES := \
    libbase \
    libpolicy \
    libpolicy-rs

include $(BUILD_EXECUTABLE)

endif

ifdef B_PROP

include $(CLEAR_VARS)
LOCAL_MODULE := resetprop
LOCAL_STATIC_LIBRARIES := libcore
LOCAL_SRC_FILES := core/applet_stub.cpp

LOCAL_CFLAGS := -DAPPLET_STUB_MAIN=resetprop_main
include $(BUILD_EXECUTABLE_MODULE)

endif

########################
# Libraries
########################

ifneq ($(strip $(B_MAGISK) $(B_PROP)),)
include $(CLEAR_VARS)
LOCAL_MODULE := libcore
LOCAL_STATIC_LIBRARIES := libbase libsystemproperties liblsplt libmagisk-rs
LOCAL_MODULE_SRC_FILES := \
    core/utils.cpp \
    core/sqlite.cpp \
    core/scripting.cpp \
    core/su/su.cpp \
    core/deny/utils.cpp \
    core/zygisk/module.cpp \
    core/core-rs.cpp
LOCAL_SRC_FILES := \
    core/resetprop/sys.cpp \
    core/deny/cli.cpp \
    core/deny/logcat.cpp \
    core/zygisk/hook.cpp \
    core/zygisk/entry.cpp
LOCAL_EXPORT_LDLIBS := -llog
include $(BUILD_STATIC_LIBRARY_MODULE)
endif

include $(CLEAR_VARS)
LOCAL_MODULE := libpolicy
LOCAL_STATIC_LIBRARIES := \
    libbase \
    libsepol
LOCAL_MODULE_SRC_FILES := sepolicy/sepolicy.cpp
LOCAL_SRC_FILES := \
    sepolicy/api.cpp \
    sepolicy/policydb.cpp \
    sepolicy/policy-rs.cpp
include $(BUILD_STATIC_LIBRARY_MODULE)

CWD := $(LOCAL_PATH)
include $(CWD)/Android-rs.mk
include $(CWD)/base/Android.mk
include $(CWD)/external/Android.mk
include src/external/libcxx/modules/Android.mk
