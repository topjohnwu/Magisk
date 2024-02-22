LOCAL_PATH := $(call my-dir)

########################
# Binaries
########################

ifdef B_MAGISK

include $(CLEAR_VARS)
LOCAL_MODULE := magisk
LOCAL_STATIC_LIBRARIES := \
    libbase \
    libsystemproperties \
    libphmap \
    liblsplt \
    libmagisk-rs \
    libxdl

LOCAL_SRC_FILES := \
    core/applets.cpp \
    core/magisk.cpp \
    core/daemon.cpp \
    core/bootstages.cpp \
    core/socket.cpp \
    core/db.cpp \
    core/package.cpp \
    core/scripting.cpp \
    core/selinux.cpp \
    core/module.cpp \
    core/thread.cpp \
    core/core-rs.cpp \
    core/resetprop/resetprop.cpp \
    core/su/su.cpp \
    core/su/connect.cpp \
    core/su/pts.cpp \
    core/su/su_daemon.cpp \
    core/zygisk/entry.cpp \
    core/zygisk/main.cpp \
    core/zygisk/hook.cpp \
    core/deny/cli.cpp \
    core/deny/utils.cpp \
    core/deny/revert.cpp \
    core/zygisk/memory.cpp \
    core/zygisk/proc_monitor.cpp \
    core/zygisk/ptrace.cpp \
    core/zygisk/ptrace_utils.cpp \
    core/deny/ptrace.cpp

LOCAL_LDLIBS := -llog
LOCAL_LDFLAGS := -Wl,--dynamic-list=src/exported_sym.txt

include $(BUILD_EXECUTABLE)

endif

ifdef B_PRELOAD

include $(CLEAR_VARS)
LOCAL_MODULE := init-ld
LOCAL_SRC_FILES := init/preload.c
LOCAL_STRIP_MODE := --strip-all
include $(BUILD_SHARED_LIBRARY)

endif

ifdef B_INIT

include $(CLEAR_VARS)
LOCAL_MODULE := magiskinit
LOCAL_STATIC_LIBRARIES := \
    libbase \
    libcompat \
    libpolicy \
    libxz \
    libinit-rs

LOCAL_SRC_FILES := \
    init/init.cpp \
    init/mount.cpp \
    init/rootdir.cpp \
    init/getinfo.cpp \
    init/twostage.cpp \
    init/selinux.cpp \
    init/init-rs.cpp

include $(BUILD_EXECUTABLE)

endif

ifdef B_BOOT

include $(CLEAR_VARS)
LOCAL_MODULE := magiskboot
LOCAL_STATIC_LIBRARIES := \
    libbase \
    libcompat \
    liblzma \
    liblz4 \
    libbz2 \
    libz \
    libzopfli \
    libboot-rs

LOCAL_SRC_FILES := \
    boot/main.cpp \
    boot/bootimg.cpp \
    boot/compress.cpp \
    boot/format.cpp \
    boot/boot-rs.cpp

include $(BUILD_EXECUTABLE)

endif

ifdef B_POLICY

include $(CLEAR_VARS)
LOCAL_MODULE := magiskpolicy
LOCAL_STATIC_LIBRARIES := \
    libbase \
    libpolicy \
    libpolicy-rs

LOCAL_SRC_FILES := sepolicy/main.cpp

include $(BUILD_EXECUTABLE)

endif

ifdef B_PROP

include $(CLEAR_VARS)
LOCAL_MODULE := resetprop
LOCAL_STATIC_LIBRARIES := \
    libbase \
    libsystemproperties \
    libmagisk-rs

LOCAL_SRC_FILES := \
    core/applet_stub.cpp \
    core/resetprop/resetprop.cpp \
    core/core-rs.cpp

LOCAL_CFLAGS := -DAPPLET_STUB_MAIN=resetprop_main
include $(BUILD_EXECUTABLE)

endif

########################
# Libraries
########################

include $(CLEAR_VARS)
LOCAL_MODULE := libpolicy
LOCAL_STATIC_LIBRARIES := \
    libbase \
    libsepol
LOCAL_C_INCLUDES := src/sepolicy/include
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_C_INCLUDES)
LOCAL_SRC_FILES := \
    sepolicy/api.cpp \
    sepolicy/sepolicy.cpp \
    sepolicy/rules.cpp \
    sepolicy/policydb.cpp \
    sepolicy/statement.cpp \
    sepolicy/policy-rs.cpp
include $(BUILD_STATIC_LIBRARY)

include src/Android-rs.mk
include src/base/Android.mk
include src/external/Android.mk

ifdef B_BB

include src/external/busybox/Android.mk

endif
