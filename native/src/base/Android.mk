LOCAL_PATH := $(call my-dir)

# Magisk project-wide common code

include $(CLEAR_VARS)
LOCAL_MODULE := libbase
LOCAL_C_INCLUDES := \
    src/include \
    $(LOCAL_PATH)/include \
    src/external/cxx-rs/include \
    out/generated
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_C_INCLUDES)
LOCAL_EXPORT_STATIC_LIBRARIES := libcxx
LOCAL_STATIC_LIBRARIES := libcxx
LOCAL_CFLAGS := -DRUST_CXX_NO_EXCEPTIONS
LOCAL_SRC_FILES := \
    new.cpp \
    files.cpp \
    misc.cpp \
    logging.cpp \
    base-rs.cpp \
    ../external/cxx-rs/src/cxx.cc
include $(BUILD_STATIC_LIBRARY)
