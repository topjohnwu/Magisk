ifneq ($(strip $(LOCAL_PCH) $(LOCAL_FILTER_ASM)),)
$(error $(LOCAL_MODULE): C++ modules do not support LOCAL_PCH or LOCAL_FILTER_ASM)
endif
ifneq ($(filter clean clean-% distclean,$(MAKECMDGOALS)),)
ifneq ($(filter-out clean clean-% distclean,$(MAKECMDGOALS)),)
$(error C++ modules require separate clean and build invocations)
endif
endif

LOCAL_CPP_EXTENSION := $(sort $(if $(LOCAL_CPP_EXTENSION),$(LOCAL_CPP_EXTENSION),$(default-c++-extensions)) .cppm .ccm .cxxm .c++m .ixx)

# Retain the group information while giving ndk-build its combined source list.
# Strip the same per-source architecture tags as build-binary.mk for matching.
MY_CXX_MODULE_INPUTS := $(patsubst %.arm,%,$(patsubst %.neon,%,$(LOCAL_MODULE_SRC_FILES)))
MY_CXX_REGULAR_INPUTS := $(patsubst %.arm,%,$(patsubst %.neon,%,$(LOCAL_SRC_FILES)))
MY_CXX_MODULE_PATHS := $(foreach MY_CXX_FILE,$(MY_CXX_MODULE_INPUTS),$(abspath $(call local-source-file-path,$(MY_CXX_FILE))))
MY_CXX_REGULAR_PATHS := $(foreach MY_CXX_FILE,$(MY_CXX_REGULAR_INPUTS),$(abspath $(call local-source-file-path,$(MY_CXX_FILE))))
ifneq ($(filter $(MY_CXX_MODULE_PATHS),$(MY_CXX_REGULAR_PATHS)),)
$(error $(LOCAL_MODULE): a source is listed in both LOCAL_MODULE_SRC_FILES and LOCAL_SRC_FILES)
endif
ifneq ($(filter-out $(foreach MY_CXX_EXT,$(LOCAL_CPP_EXTENSION),%$(MY_CXX_EXT)),$(MY_CXX_MODULE_INPUTS)),)
$(error $(LOCAL_MODULE): LOCAL_MODULE_SRC_FILES must contain C++ sources)
endif

# ndk-build drops source extensions when naming objects. Diagnose collisions
# such as math.cppm + math.cpp instead of allowing two recipes for math.o.
MY_CXX_OBJECT_NAMES := $(foreach MY_CXX_FILE,$(MY_CXX_MODULE_INPUTS) $(MY_CXX_REGULAR_INPUTS),$(abspath $(call get-object-name,$(MY_CXX_FILE))))
ifneq ($(words $(MY_CXX_OBJECT_NAMES)),$(words $(sort $(MY_CXX_OBJECT_NAMES))))
$(error $(LOCAL_MODULE): sources map to the same object path; use distinct source stems or directories)
endif

LOCAL_SRC_FILES += $(LOCAL_MODULE_SRC_FILES)
LOCAL_MODULE_SRC_FILES := $(MY_CXX_MODULE_INPUTS)

# ndk-build strips an optional lib prefix when it registers LOCAL_MODULE.
MY_CXX_ENABLED.$(TARGET_OBJS).$(call strip-lib-prefix,$(LOCAL_MODULE)) := true

MY_CXX_CLEAN := clean-cxx-modules-$(TARGET_ARCH_ABI)
ifndef MY_CXX_CLEAN_SEEN.$(TARGET_OBJS)
MY_CXX_CLEAN_SEEN.$(TARGET_OBJS) := true
.PHONY: $(MY_CXX_CLEAN)
clean: $(MY_CXX_CLEAN)
$(MY_CXX_CLEAN): MY_CXX_CLEAN_DIR := $(TARGET_OBJS)/cxx-modules
$(MY_CXX_CLEAN):
	@$(call host-rmdir,$(MY_CXX_CLEAN_DIR))
endif
