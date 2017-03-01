LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := su
LOCAL_STATIC_LIBRARIES := libselinux
LOCAL_C_INCLUDES := jni/selinux/libselinux/include/ jni/selinux/libsepol/include/ jni/su/sqlite3/
LOCAL_SRC_FILES := su.c daemon.c activity.c db.c utils.c pts.c sqlite3/sqlite3.c
LOCAL_CFLAGS := -DSQLITE_OMIT_LOAD_EXTENSION -std=gnu11
include $(BUILD_EXECUTABLE)

include jni/selinux/libselinux/Android.mk
