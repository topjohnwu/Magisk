LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := crt0

# Manually link the compiler runtime library
LOCAL_compiler_rt := $(shell $(TARGET_CC) -target $(LLVM_TRIPLE)$(TARGET_PLATFORM_LEVEL) --print-libgcc-file-name)

LOCAL_EXPORT_LDFLAGS := -static -nostartfiles -nodefaultlibs $(LOCAL_compiler_rt)

LOCAL_SRC_FILES := \
	nolibc.c

include $(BUILD_STATIC_LIBRARY)
