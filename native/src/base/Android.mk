LOCAL_PATH := $(call my-dir)

# Magisk project-wide common code

include $(CLEAR_VARS)
LOCAL_MODULE := libbase
LOCAL_C_INCLUDES := \
    src/include \
    $(LOCAL_PATH)/include \
    $(LOCAL_PATH)/../external/cxx-rs/include \
    out/generated
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_C_INCLUDES)
LOCAL_EXPORT_STATIC_LIBRARIES := libcxx
LOCAL_STATIC_LIBRARIES := libcxx
LOCAL_CFLAGS := -DRUST_CXX_NO_EXCEPTIONS
LOCAL_SRC_FILES := \
    new.cpp \
    files.cpp \
    misc.cpp \
    selinux.cpp \
    logging.cpp \
    stream.cpp \
    base-rs.cpp \
    ../external/cxx-rs/src/cxx.cc
include $(BUILD_STATIC_LIBRARY)

# All static executables should link with libcompat

include $(CLEAR_VARS)
LOCAL_MODULE := libcompat
LOCAL_SRC_FILES := compat/compat.cpp
# Fix static variables' ctor/dtor when using LTO
# See: https://github.com/android/ndk/issues/1461
LOCAL_EXPORT_LDFLAGS := -static -T src/lto_fix.lds -Wl,--wrap=rename -Wl,--wrap=renameat
# For some reason, using the hacky libc.a with x86 will trigger stack protection violation
# when mixing Rust and C++ code. Disable stack protector to bypass this issue.
ifeq ($(TARGET_ARCH), x86)
LOCAL_EXPORT_CFLAGS := -fno-stack-protector
endif
include $(BUILD_STATIC_LIBRARY)
