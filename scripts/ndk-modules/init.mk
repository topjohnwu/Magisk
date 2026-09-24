# C++ named modules for ndk-build. Include once from the project's Android.mk.
ifndef MY_CXX_MODULES_DIR
MY_CXX_MODULES_DIR := $(call my-dir)

BUILD_SHARED_LIBRARY_MODULE := $(MY_CXX_MODULES_DIR)/shared-library.mk
BUILD_STATIC_LIBRARY_MODULE := $(MY_CXX_MODULES_DIR)/static-library.mk
BUILD_EXECUTABLE_MODULE := $(MY_CXX_MODULES_DIR)/executable.mk

# Include our source list in CLEAR_VARS and ndk-build's per-target save/restore.
modules-LOCALS += MODULE_SRC_FILES

MY_CXX_IS_MODULE_SOURCE = $(filter $(abspath $(call local-source-file-path,$1)),\
    $(foreach MY_CXX_FILE,$(LOCAL_MODULE_SRC_FILES),\
        $(abspath $(call local-source-file-path,$(MY_CXX_FILE)))))

# ndk-build registers LOCAL_* first, then resolves exported flags and emits
# compilation rules. Wrap the latter so scanning sees the actual compile flags.
define MY_CXX_SAVE_COMPILER
MY_CXX_ORIGINAL_COMPILE_CPP = $(value compile-cpp-source)
endef
$(eval $(MY_CXX_SAVE_COMPILER))

define MY_CXX_SAVE_OBJECT_NAME
MY_CXX_ORIGINAL_OBJECT_NAME = $(value get-object-name)
endef
$(eval $(MY_CXX_SAVE_OBJECT_NAME))

# Keep the interface suffix, allowing foo.ixx and foo.cxx in one target.
get-object-name = $(strip $(if $(and $(MY_CXX_ENABLED.$(TARGET_OBJS).$(call strip-lib-prefix,$(LOCAL_MODULE))),$(call MY_CXX_IS_MODULE_SOURCE,$1)),\
    $(patsubst %$(TARGET_OBJ_EXTENSION),%$(suffix $1)$(TARGET_OBJ_EXTENSION),$(call MY_CXX_ORIGINAL_OBJECT_NAME,$1)),\
    $(call MY_CXX_ORIGINAL_OBJECT_NAME,$1)))

compile-cpp-source = \
    $(if $(MY_CXX_ENABLED.$(TARGET_OBJS).$(LOCAL_MODULE)),\
        $(eval LOCAL_OBJECTS := $(addprefix $(LOCAL_OBJS_DIR)/,$(filter %$(TARGET_OBJ_EXTENSION),$(foreach MY_CXX_FILE,$(LOCAL_SRC_FILES),$(call get-object-name,$(MY_CXX_FILE)))))) \
        $(eval MY_CXX_IS_MODULE := $(call MY_CXX_IS_MODULE_SOURCE,$1)) \
        $(if $(MY_CXX_IS_MODULE),\
            $(call add-src-files-target-cflags,$1,-x c++-module))) \
    $(call MY_CXX_ORIGINAL_COMPILE_CPP,$1,$2) \
    $(if $(MY_CXX_ENABLED.$(TARGET_OBJS).$(LOCAL_MODULE)),\
        $(eval MY_CXX_TIDY_TARGET := $(LOCAL_OBJS_DIR)/$1.tidy) \
        $(eval include $(MY_CXX_MODULES_DIR)/compile.mk))
endif
