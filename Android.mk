LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := su
LOCAL_STATIC_LIBRARIES := libselinux libsqlite3
LOCAL_C_INCLUDES := jni/selinux/libselinux/include/ jni/selinux/libsepol/include/ jni/sqlite3/
LOCAL_SRC_FILES := su.c daemon.c activity.c db.c utils.c pts.c
LOCAL_CFLAGS := -DSQLITE_OMIT_LOAD_EXTENSION -DINDEP_BINARY
include $(BUILD_EXECUTABLE)

include jni/selinux/libselinux/Android.mk
include jni/sqlite3/Android.mk
