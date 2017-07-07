LOCAL_PATH:= $(call my-dir)

##
# libsqlite.so (stub)
#

include $(CLEAR_VARS)
LOCAL_MODULE:= libsqlite
LOCAL_SRC_FILES := sqlite3_stub.c
include $(BUILD_SHARED_LIBRARY)
