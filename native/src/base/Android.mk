LOCAL_PATH := $(call my-dir)

# Magisk project-wide common code

include $(CLEAR_VARS)
LOCAL_MODULE := libbase
LOCAL_C_INCLUDES := $(LOCAL_PATH)/include out/generated
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_C_INCLUDES)
LOCAL_EXPORT_STATIC_LIBRARIES := cxx_std
LOCAL_STATIC_LIBRARIES := cxx_std
LOCAL_MODULE_SRC_FILES := utils.cpp types.cpp base-rs.cpp base.cpp
LOCAL_SRC_FILES := base-cxx.cpp ../external/cxx-rs/src/cxx.cc
include $(BUILD_STATIC_LIBRARY_MODULE)
