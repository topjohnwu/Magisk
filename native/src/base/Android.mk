LOCAL_PATH := $(call my-dir)

# Magisk project-wide common code

include $(CLEAR_VARS)
LOCAL_MODULE := libbase
LOCAL_C_INCLUDES := src/include $(LOCAL_PATH)/include out/generated
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_C_INCLUDES)
LOCAL_EXPORT_STATIC_LIBRARIES := libcxx
LOCAL_STATIC_LIBRARIES := libcxx
LOCAL_SRC_FILES := \
    new.cpp \
    files.cpp \
    misc.cpp \
    selinux.cpp \
    logging.cpp \
    stream.cpp \
    ../external/cxx-rs/src/cxx.cc
include $(BUILD_STATIC_LIBRARY)

# All static executables should link with libcompat

include $(CLEAR_VARS)
LOCAL_MODULE := libcompat
# Workaround "hacky" libc.a missing symbols
# To build Magisk with vanilla NDK, comment out the next line
LOCAL_SRC_FILES := compat/compat.cpp
# Fix static variables' ctor/dtor when using LTO
# See: https://github.com/android/ndk/issues/1461
LOCAL_EXPORT_LDFLAGS := -static -T src/lto_fix.lds
include $(BUILD_STATIC_LIBRARY)
