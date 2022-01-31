LOCAL_PATH := $(call my-dir)

# Magisk project-wide common code

include $(CLEAR_VARS)
LOCAL_MODULE:= libutils
LOCAL_C_INCLUDES := jni/include $(LOCAL_PATH)/include out/generated
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_C_INCLUDES)
LOCAL_EXPORT_STATIC_LIBRARIES := libcxx
LOCAL_STATIC_LIBRARIES := libcxx
LOCAL_SRC_FILES := \
    new.cpp \
    files.cpp \
    misc.cpp \
    selinux.cpp \
    logging.cpp \
    xwrap.cpp \
    stream.cpp
include $(BUILD_STATIC_LIBRARY)

# libutils + "hacky" libc.a missing symbols

# To build Magisk with vanilla NDK, simply
# remove compat.cpp from sources, or replace
# all usage of libutilx to libutils

include $(CLEAR_VARS)
LOCAL_MODULE:= libutilx
LOCAL_EXPORT_STATIC_LIBRARIES := libutils
LOCAL_STATIC_LIBRARIES := libutils
LOCAL_SRC_FILES := compat/compat.cpp
include $(BUILD_STATIC_LIBRARY)
