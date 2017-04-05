LOCAL_PATH:= $(call my-dir)

##
# libsqlite.so
#

include $(CLEAR_VARS)
LOCAL_MODULE:= libsqlite
LOCAL_SRC_FILES := sqlite3.c shell.c
include $(BUILD_SHARED_LIBRARY)
