LOCAL_PATH := $(call my-dir)

###########################
# Rust compilation outputs
###########################

include $(CLEAR_VARS)
LOCAL_MODULE := magisk-rs
LOCAL_EXPORT_C_INCLUDES := src/core/include
LOCAL_LIB = ../out/$(TARGET_ARCH_ABI)/libmagisk-rs.a
ifneq (,$(wildcard $(LOCAL_PATH)/$(LOCAL_LIB)))
LOCAL_SRC_FILES := $(LOCAL_LIB)
include $(PREBUILT_STATIC_LIBRARY)
else
include $(BUILD_STATIC_LIBRARY)
endif

include $(CLEAR_VARS)
LOCAL_MODULE := boot-rs
LOCAL_LIB = ../out/$(TARGET_ARCH_ABI)/libmagiskboot-rs.a
ifneq (,$(wildcard $(LOCAL_PATH)/$(LOCAL_LIB)))
LOCAL_SRC_FILES := $(LOCAL_LIB)
include $(PREBUILT_STATIC_LIBRARY)
else
include $(BUILD_STATIC_LIBRARY)
endif

include $(CLEAR_VARS)
LOCAL_MODULE := init-rs
LOCAL_LIB = ../out/$(TARGET_ARCH_ABI)/libmagiskinit-rs.a
ifneq (,$(wildcard $(LOCAL_PATH)/$(LOCAL_LIB)))
LOCAL_SRC_FILES := $(LOCAL_LIB)
include $(PREBUILT_STATIC_LIBRARY)
else
include $(BUILD_STATIC_LIBRARY)
endif

include $(CLEAR_VARS)
LOCAL_MODULE := policy-rs
LOCAL_LIB = ../out/$(TARGET_ARCH_ABI)/libmagiskpolicy-rs.a
ifneq (,$(wildcard $(LOCAL_PATH)/$(LOCAL_LIB)))
LOCAL_SRC_FILES := $(LOCAL_LIB)
include $(PREBUILT_STATIC_LIBRARY)
else
include $(BUILD_STATIC_LIBRARY)
endif
