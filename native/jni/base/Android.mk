LOCAL_PATH := $(call my-dir)

# Magisk project-wide common code

include $(CLEAR_VARS)
LOCAL_MODULE:= libbase
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

# Workaround "hacky" libc.a missing symbols
# To build Magisk with vanilla NDK, remove all usage of libcompat

include $(CLEAR_VARS)
LOCAL_MODULE:= libcompat
LOCAL_SRC_FILES := compat/compat.cpp
include $(BUILD_STATIC_LIBRARY)
