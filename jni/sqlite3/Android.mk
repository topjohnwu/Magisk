LOCAL_PATH:= $(call my-dir)

##
# libsqlite3.a
#

include $(CLEAR_VARS)
LOCAL_MODULE:= libsqlite3
LOCAL_SRC_FILES := sqlite3.c shell.c
include $(BUILD_STATIC_LIBRARY)
