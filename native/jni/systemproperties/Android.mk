LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE:= libsystemproperties
LOCAL_C_INCLUDES := jni/include $(LIBSYSTEMPROPERTIES)
LOCAL_SRC_FILES := \
	context_node.cpp \
	contexts_serialized.cpp \
	contexts_split.cpp \
	prop_area.cpp \
	prop_info.cpp \
	system_properties.cpp \
	property_info_parser.cpp

include $(BUILD_STATIC_LIBRARY)
