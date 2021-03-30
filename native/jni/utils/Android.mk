LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE:= libutils
LOCAL_C_INCLUDES := jni/include $(LOCAL_PATH)/include
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/include
LOCAL_EXPORT_STATIC_LIBRARIES := libcxx
LOCAL_STATIC_LIBRARIES := libcxx
LOCAL_SRC_FILES := \
    missing.cpp \
    new.cpp \
    files.cpp \
    misc.cpp \
    selinux.cpp \
    logging.cpp \
    xwrap.cpp \
    stream.cpp \
    version.cpp

$(LOCAL_PATH)/version.cpp : FORCE
	$(file > $@,#include <limits.h>)
	$(file >> $@,const char* MAGISK_VERSION="${MAGISK_VERSION}";)
	$(file >> $@,int MAGISK_VER_CODE=${MAGISK_VER_CODE};)
	$(file >> $@,const char* MAGISK_FULL_VER="${MAGISK_VERSION}(${MAGISK_VER_CODE})";)

FORCE: ;
include $(BUILD_STATIC_LIBRARY)
