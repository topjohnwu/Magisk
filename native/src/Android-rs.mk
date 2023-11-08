LOCAL_PATH := $(call my-dir)

###########################
# Rust compilation outputs
###########################

LIBRARY_PATH = ../out/$(TARGET_ARCH_ABI)/libmagisk-rs.a
ifneq (,$(wildcard $(LOCAL_PATH)/$(LIBRARY_PATH)))
include $(CLEAR_VARS)
LOCAL_MODULE := magisk-rs
LOCAL_EXPORT_C_INCLUDES := src/core/include
LOCAL_SRC_FILES := $(LIBRARY_PATH)
include $(PREBUILT_STATIC_LIBRARY)
endif

LIBRARY_PATH = ../out/$(TARGET_ARCH_ABI)/libmagiskboot-rs.a
ifneq (,$(wildcard $(LOCAL_PATH)/$(LIBRARY_PATH)))
include $(CLEAR_VARS)
LOCAL_MODULE := boot-rs
LOCAL_SRC_FILES := $(LIBRARY_PATH)
include $(PREBUILT_STATIC_LIBRARY)
endif

LIBRARY_PATH = ../out/$(TARGET_ARCH_ABI)/libmagiskinit-rs.a
ifneq (,$(wildcard $(LOCAL_PATH)/$(LIBRARY_PATH)))
include $(CLEAR_VARS)
LOCAL_MODULE := init-rs
LOCAL_SRC_FILES := $(LIBRARY_PATH)
include $(PREBUILT_STATIC_LIBRARY)
endif

LIBRARY_PATH = ../out/$(TARGET_ARCH_ABI)/libmagiskpolicy-rs.a
ifneq (,$(wildcard $(LOCAL_PATH)/$(LIBRARY_PATH)))
include $(CLEAR_VARS)
LOCAL_MODULE := policy-rs
LOCAL_SRC_FILES := $(LIBRARY_PATH)
include $(PREBUILT_STATIC_LIBRARY)
endif
