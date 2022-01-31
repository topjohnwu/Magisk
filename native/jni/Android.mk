LOCAL_PATH := $(call my-dir)

########################
# Binaries
########################

ifdef B_MAGISK

include $(CLEAR_VARS)
LOCAL_MODULE := magisk
LOCAL_STATIC_LIBRARIES := \
    libutils \
    libnanopb \
    libsystemproperties \
    libphmap \
    libxhook

LOCAL_SRC_FILES := \
    core/applets.cpp \
    core/magisk.cpp \
    core/daemon.cpp \
    core/bootstages.cpp \
    core/socket.cpp \
    core/db.cpp \
    core/scripting.cpp \
    core/restorecon.cpp \
    core/module.cpp \
    core/logging.cpp \
    core/thread.cpp \
    resetprop/persist.cpp \
    resetprop/resetprop.cpp \
    su/su.cpp \
    su/connect.cpp \
    su/pts.cpp \
    su/su_daemon.cpp \
    zygisk/entry.cpp \
    zygisk/main.cpp \
    zygisk/utils.cpp \
    zygisk/hook.cpp \
    zygisk/memory.cpp \
    zygisk/deny/cli.cpp \
    zygisk/deny/utils.cpp \
    zygisk/deny/revert.cpp

LOCAL_LDLIBS := -llog

include $(BUILD_EXECUTABLE)

endif

ifdef B_INIT

include $(CLEAR_VARS)
LOCAL_MODULE := magiskinit
LOCAL_STATIC_LIBRARIES := \
    libutilx \
    libsepol \
    libxz

LOCAL_SRC_FILES := \
    init/init.cpp \
    init/mount.cpp \
    init/rootdir.cpp \
    init/getinfo.cpp \
    init/twostage.cpp \
    core/socket.cpp \
    magiskpolicy/sepolicy.cpp \
    magiskpolicy/magiskpolicy.cpp \
    magiskpolicy/rules.cpp \
    magiskpolicy/policydb.cpp \
    magiskpolicy/statement.cpp \
    magiskboot/pattern.cpp

LOCAL_LDFLAGS := -static
include $(BUILD_EXECUTABLE)

endif

ifdef B_BOOT

include $(CLEAR_VARS)
LOCAL_MODULE := magiskboot
LOCAL_STATIC_LIBRARIES := \
    libutilx \
    libmincrypt \
    liblzma \
    liblz4 \
    libbz2 \
    libfdt \
    libz \
    libzopfli

LOCAL_SRC_FILES := \
    magiskboot/main.cpp \
    magiskboot/bootimg.cpp \
    magiskboot/hexpatch.cpp \
    magiskboot/compress.cpp \
    magiskboot/format.cpp \
    magiskboot/dtb.cpp \
    magiskboot/ramdisk.cpp \
    magiskboot/pattern.cpp \
    magiskboot/cpio.cpp

LOCAL_LDFLAGS := -static
include $(BUILD_EXECUTABLE)

endif

ifdef B_POLICY

include $(CLEAR_VARS)
LOCAL_MODULE := magiskpolicy
LOCAL_STATIC_LIBRARIES := \
    libutilx \
    libsepol

LOCAL_SRC_FILES := \
    core/applet_stub.cpp \
    magiskpolicy/sepolicy.cpp \
    magiskpolicy/magiskpolicy.cpp \
    magiskpolicy/rules.cpp \
    magiskpolicy/policydb.cpp \
    magiskpolicy/statement.cpp

LOCAL_CFLAGS := -DAPPLET_STUB_MAIN=magiskpolicy_main
LOCAL_LDFLAGS := -static
include $(BUILD_EXECUTABLE)

endif

ifdef B_PROP

include $(CLEAR_VARS)
LOCAL_MODULE := resetprop
LOCAL_STATIC_LIBRARIES := \
    libutilx \
    libnanopb \
    libsystemproperties

LOCAL_SRC_FILES := \
    core/applet_stub.cpp \
    resetprop/persist_properties.cpp \
    resetprop/resetprop.cpp \

LOCAL_CFLAGS := -DAPPLET_STUB_MAIN=resetprop_main
LOCAL_LDFLAGS := -static
include $(BUILD_EXECUTABLE)

endif

ifdef B_TEST
ifneq (,$(wildcard jni/test.cpp))

include $(CLEAR_VARS)
LOCAL_MODULE := test
LOCAL_STATIC_LIBRARIES := \
    libutils \
    libphmap

LOCAL_SRC_FILES := test.cpp
include $(BUILD_EXECUTABLE)

endif
endif

ifdef B_BB

include jni/external/busybox/Android.mk

endif

########################
# Libraries
########################
include jni/utils/Android.mk
include jni/external/Android.mk
