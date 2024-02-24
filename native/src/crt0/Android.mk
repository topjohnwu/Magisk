LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := crt0

# Manually link the compiler runtime library
LOCAL_compiler_rt := $(shell $(TARGET_CC) -target $(LLVM_TRIPLE)$(TARGET_PLATFORM_LEVEL) --print-libgcc-file-name)

LOCAL_EXPORT_LDFLAGS := -static -nostartfiles -nodefaultlibs $(LOCAL_compiler_rt) -Wl,--error-limit=0

LOCAL_SRC_FILES := \
    dirent.cpp \
	malloc.c \
	mem.c \
	misc.c \
	nolibc.c \
	stdio.c \
	syscall.c \
	syscall/syscall-$(TARGET_ARCH).S \
	tinystdio/tinystdio.c

include $(BUILD_STATIC_LIBRARY)
