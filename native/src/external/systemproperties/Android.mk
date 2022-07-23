LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE:= libsystemproperties
LOCAL_C_INCLUDES := $(LOCAL_PATH)/include
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_C_INCLUDES)
LOCAL_STATIC_LIBRARIES := libcxx
LOCAL_SRC_FILES := \
	context_node.cpp \
	contexts_serialized.cpp \
	contexts_split.cpp \
	prop_area.cpp \
	prop_info.cpp \
	system_properties.cpp \
	system_property_api.cpp \
	system_property_set.cpp \
	property_info_parser.cpp

include $(BUILD_STATIC_LIBRARY)
