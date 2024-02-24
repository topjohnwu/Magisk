LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := crt0

# Manually link the compiler runtime library
LOCAL_compiler_rt := $(shell $(TARGET_CC) -target $(LLVM_TRIPLE)$(TARGET_PLATFORM_LEVEL) --print-libgcc-file-name)

LOCAL_EXPORT_LDFLAGS := -static -nostartfiles -nodefaultlibs $(LOCAL_compiler_rt) -Wl,--error-limit=0

LOCAL_SRC_FILES := \
	malloc.c \
	nolibc.c \
	stdio.c \
	syscall.c \
	tinystdio/tinystdio.c

include $(BUILD_STATIC_LIBRARY)
