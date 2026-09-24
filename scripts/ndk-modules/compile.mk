# Included immediately after ndk-build emits a C++ compilation rule. _SRC,
# _OBJ and _FLAGS still describe that translation unit, including export flags.
ifeq ($(filter clean clean-% distclean,$(MAKECMDGOALS)),)
MY_CXX_GRAPH := $(TARGET_OBJS)/cxx-modules/deps.mk
MY_CXX_SCAN := $(_OBJ).ddi
MY_CXX_ARGS := $(_OBJ).scan.args
MY_CXX_SCANNER := $(dir $(TARGET_CXX))clang-scan-deps$(HOST_EXEEXT)

ifeq ($(wildcard $(MY_CXX_SCANNER)),)
$(error C++ modules require clang-scan-deps next to $(TARGET_CXX))
endif

MY_CXX_SCAN_ARGS := \
    --compiler $(TARGET_CXX) --source $(_SRC) --object $(_OBJ) \
    --module $(LOCAL_MODULE) \
    --visible $(subst $(space),$(comma),$(strip $(call module-get-all-dependencies,$(LOCAL_MODULE)))) \
    -- $(_FLAGS)

$(call generate-file-dir,$(MY_CXX_ARGS).tmp)
$(call generate-list-file,$(MY_CXX_SCAN_ARGS),$(MY_CXX_ARGS))

$(MY_CXX_SCAN): MY_CXX_PYTHON := $(HOST_PYTHON)
$(MY_CXX_SCAN): MY_CXX_HELPER := $(MY_CXX_MODULES_DIR)/modules.py
$(MY_CXX_SCAN): MY_CXX_INPUT := $(MY_CXX_ARGS)
$(MY_CXX_SCAN): MY_CXX_TOOL := $(MY_CXX_SCANNER)
$(MY_CXX_SCAN): $(MY_CXX_ARGS) $(_SRC) $(MY_CXX_MODULES_DIR)/modules.py $(MY_CXX_SCANNER) $(TARGET_CXX)
	@$(MY_CXX_PYTHON) "$(MY_CXX_HELPER)" scan --scanner "$(MY_CXX_TOOL)" --args "$(MY_CXX_INPUT)" --output "$@"

-include $(MY_CXX_SCAN).d

# Avoid compiler caches which may restore the object but omit its BMI.
$(_OBJ): PRIVATE_CC := $(TARGET_CXX)
$(_OBJ): PRIVATE_CFLAGS += @$(_OBJ).modules.rsp
$(_OBJ): $(MY_CXX_SCAN)
$(_OBJ).commands.json: PRIVATE_COMPILE_COMMAND_ARG += @$(_OBJ).modules.rsp
$(_OBJ).commands.json: $(_OBJ).modules.rsp
$(MY_CXX_TIDY_TARGET): PRIVATE_CFLAGS += @$(_OBJ).modules.rsp

MY_CXX_INPUTS.$(MY_CXX_GRAPH) := $(MY_CXX_INPUTS.$(MY_CXX_GRAPH)) $(MY_CXX_SCAN)
$(MY_CXX_GRAPH).list: MY_CXX_INPUTS := $(MY_CXX_INPUTS.$(MY_CXX_GRAPH))
$(MY_CXX_GRAPH): $(MY_CXX_SCAN)

ifndef MY_CXX_GRAPH_SEEN.$(MY_CXX_GRAPH)
MY_CXX_GRAPH_SEEN.$(MY_CXX_GRAPH) := true
$(call generate-file-dir,$(MY_CXX_GRAPH).list)
.PHONY: $(MY_CXX_GRAPH).force
$(MY_CXX_GRAPH).list: $(MY_CXX_GRAPH).force
$(MY_CXX_GRAPH).list: MY_CXX_PYTHON := $(HOST_PYTHON)
$(MY_CXX_GRAPH).list: MY_CXX_HELPER := $(MY_CXX_MODULES_DIR)/modules.py
$(MY_CXX_GRAPH).list:
	@$(MY_CXX_PYTHON) "$(MY_CXX_HELPER)" manifest --output "$@" $(MY_CXX_INPUTS)

$(MY_CXX_GRAPH): MY_CXX_PYTHON := $(HOST_PYTHON)
$(MY_CXX_GRAPH): MY_CXX_HELPER := $(MY_CXX_MODULES_DIR)/modules.py
$(MY_CXX_GRAPH): $(MY_CXX_GRAPH).list $(MY_CXX_MODULES_DIR)/modules.py
	@$(MY_CXX_PYTHON) "$(MY_CXX_HELPER)" graph --manifest "$@.list" --output "$@"

# An included makefile is rebuilt before any objects, then Make restarts with
# the discovered dependency graph. Do not suppress scanner/graph failures.
include $(MY_CXX_GRAPH)
endif
endif
