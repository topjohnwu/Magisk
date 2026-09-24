# C++ named modules for ndk-build. Include once from the project's Android.mk.
ifndef MY_CXX_MODULES_DIR
MY_CXX_MODULES_DIR := $(call my-dir)

BUILD_SHARED_LIBRARY_MODULE := $(MY_CXX_MODULES_DIR)/shared-library.mk
BUILD_STATIC_LIBRARY_MODULE := $(MY_CXX_MODULES_DIR)/static-library.mk
BUILD_EXECUTABLE_MODULE := $(MY_CXX_MODULES_DIR)/executable.mk

# ndk-build registers LOCAL_* first, then resolves exported flags and emits
# compilation rules. Wrap the latter so scanning sees the actual compile flags.
define MY_CXX_SAVE_COMPILER
MY_CXX_ORIGINAL_COMPILE_CPP = $(value compile-cpp-source)
endef
$(eval $(MY_CXX_SAVE_COMPILER))

compile-cpp-source = \
    $(call MY_CXX_ORIGINAL_COMPILE_CPP,$1,$2) \
    $(if $(MY_CXX_ENABLED.$(TARGET_OBJS).$(LOCAL_MODULE)),\
        $(eval MY_CXX_TIDY_TARGET := $(LOCAL_OBJS_DIR)/$1.tidy) \
        $(eval include $(MY_CXX_MODULES_DIR)/compile.mk))
endif
