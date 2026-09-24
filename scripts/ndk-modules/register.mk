ifneq ($(strip $(LOCAL_PCH) $(LOCAL_FILTER_ASM)),)
$(error $(LOCAL_MODULE): C++ modules do not support LOCAL_PCH or LOCAL_FILTER_ASM)
endif
ifneq ($(filter clean clean-% distclean,$(MAKECMDGOALS)),)
ifneq ($(filter-out clean clean-% distclean,$(MAKECMDGOALS)),)
$(error C++ modules require separate clean and build invocations)
endif
endif

LOCAL_CPP_EXTENSION := $(sort $(if $(LOCAL_CPP_EXTENSION),$(LOCAL_CPP_EXTENSION),$(default-c++-extensions)) .cppm .ccm .cxxm .c++m)
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
