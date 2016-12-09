my_path := $(call my-dir)

LOCAL_PATH := $(my_path)

include $(CLEAR_VARS)
LOCAL_MODULE := magiskhide
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := magiskhide.c
LOCAL_CFLAGS += -std=gnu11
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE := bootimgtools
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := bootimgtools/main.c bootimgtools/extract.c bootimgtools/repack.c bootimgtools/hexpatch.c
LOCAL_CFLAGS += -std=gnu11
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE := sepolicy-inject
LOCAL_MODULE_TAGS := optional
LOCAL_STATIC_LIBRARIES := libsepol
LOCAL_SRC_FILES := sepolicy-inject/sepolicy-inject.c sepolicy-inject/builtin_rules.c
LOCAL_C_INCLUDES := jni/selinux/libsepol/include/
LOCAL_CFLAGS += -std=gnu11
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE := resetprop
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := resetprop/resetprop.cpp resetprop/system_properties.cpp resetprop/libc_logging.cpp
LOCAL_LDLIBS += -latomic
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE := su
LOCAL_MODULE_TAGS := eng debug optional
LOCAL_STATIC_LIBRARIES := libselinux
LOCAL_C_INCLUDES := jni/selinux/libselinux/include/ jni/selinux/libsepol/include/ jni/su/sqlite3/
LOCAL_SRC_FILES := su/su.c su/daemon.c su/activity.c su/db.c su/utils.c su/pts.c su/hacks.c su/binds.c su/sqlite3/sqlite3.c
LOCAL_CFLAGS := -DSQLITE_OMIT_LOAD_EXTENSION -std=gnu11
include $(BUILD_EXECUTABLE)

include jni/selinux/libsepol/Android.mk
include jni/selinux/libselinux/Android.mk
