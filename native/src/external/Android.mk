LOCAL_PATH := $(call my-dir)

# libxz.a
include $(CLEAR_VARS)
LOCAL_MODULE:= libxz
LOCAL_C_INCLUDES := $(LOCAL_PATH)/xz-embedded
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_C_INCLUDES)
LOCAL_SRC_FILES := \
    xz-embedded/xz_crc32.c \
    xz-embedded/xz_dec_lzma2.c \
    xz-embedded/xz_dec_stream.c
include $(BUILD_STATIC_LIBRARY)

# liblz4.a
include $(CLEAR_VARS)
LOCAL_MODULE := liblz4
LOCAL_C_INCLUDES := $(LOCAL_PATH)/lz4/lib
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_C_INCLUDES)
LOCAL_SRC_FILES := \
    lz4/lib/lz4.c \
    lz4/lib/lz4frame.c \
    lz4/lib/lz4hc.c \
    lz4/lib/xxhash.c
include $(BUILD_STATIC_LIBRARY)

# liblzma.a
include $(CLEAR_VARS)
LOCAL_MODULE := liblzma
LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/xz_config \
    $(LOCAL_PATH)/xz/src/common \
    $(LOCAL_PATH)/xz/src/liblzma/api \
    $(LOCAL_PATH)/xz/src/liblzma/check \
    $(LOCAL_PATH)/xz/src/liblzma/common \
    $(LOCAL_PATH)/xz/src/liblzma/delta \
    $(LOCAL_PATH)/xz/src/liblzma/lz \
    $(LOCAL_PATH)/xz/src/liblzma/lzma \
    $(LOCAL_PATH)/xz/src/liblzma/rangecoder \
    $(LOCAL_PATH)/xz/src/liblzma/simple \
    $(LOCAL_PATH)/xz/src/liblzma
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/xz/src/liblzma/api
LOCAL_SRC_FILES := \
    xz/src/common/tuklib_cpucores.c \
    xz/src/common/tuklib_exit.c \
    xz/src/common/tuklib_mbstr_fw.c \
    xz/src/common/tuklib_mbstr_width.c \
    xz/src/common/tuklib_open_stdxxx.c \
    xz/src/common/tuklib_physmem.c \
    xz/src/common/tuklib_progname.c \
    xz/src/liblzma/check/check.c \
    xz/src/liblzma/check/crc32_fast.c \
    xz/src/liblzma/check/crc32_table.c \
    xz/src/liblzma/check/crc64_fast.c \
    xz/src/liblzma/check/crc64_table.c \
    xz/src/liblzma/check/sha256.c \
    xz/src/liblzma/common/alone_decoder.c \
    xz/src/liblzma/common/alone_encoder.c \
    xz/src/liblzma/common/auto_decoder.c \
    xz/src/liblzma/common/block_buffer_decoder.c \
    xz/src/liblzma/common/block_buffer_encoder.c \
    xz/src/liblzma/common/block_decoder.c \
    xz/src/liblzma/common/block_encoder.c \
    xz/src/liblzma/common/block_header_decoder.c \
    xz/src/liblzma/common/block_header_encoder.c \
    xz/src/liblzma/common/block_util.c \
    xz/src/liblzma/common/common.c \
    xz/src/liblzma/common/easy_buffer_encoder.c \
    xz/src/liblzma/common/easy_decoder_memusage.c \
    xz/src/liblzma/common/easy_encoder.c \
    xz/src/liblzma/common/easy_encoder_memusage.c \
    xz/src/liblzma/common/easy_preset.c \
    xz/src/liblzma/common/filter_buffer_decoder.c \
    xz/src/liblzma/common/filter_buffer_encoder.c \
    xz/src/liblzma/common/filter_common.c \
    xz/src/liblzma/common/filter_decoder.c \
    xz/src/liblzma/common/filter_encoder.c \
    xz/src/liblzma/common/filter_flags_decoder.c \
    xz/src/liblzma/common/filter_flags_encoder.c \
    xz/src/liblzma/common/hardware_cputhreads.c \
    xz/src/liblzma/common/hardware_physmem.c \
    xz/src/liblzma/common/index.c \
    xz/src/liblzma/common/index_decoder.c \
    xz/src/liblzma/common/index_encoder.c \
    xz/src/liblzma/common/index_hash.c \
    xz/src/liblzma/common/outqueue.c \
    xz/src/liblzma/common/stream_buffer_decoder.c \
    xz/src/liblzma/common/stream_buffer_encoder.c \
    xz/src/liblzma/common/stream_decoder.c \
    xz/src/liblzma/common/stream_encoder.c \
    xz/src/liblzma/common/stream_encoder_mt.c \
    xz/src/liblzma/common/stream_flags_common.c \
    xz/src/liblzma/common/stream_flags_decoder.c \
    xz/src/liblzma/common/stream_flags_encoder.c \
    xz/src/liblzma/common/vli_decoder.c \
    xz/src/liblzma/common/vli_encoder.c \
    xz/src/liblzma/common/vli_size.c \
    xz/src/liblzma/delta/delta_common.c \
    xz/src/liblzma/delta/delta_decoder.c \
    xz/src/liblzma/delta/delta_encoder.c \
    xz/src/liblzma/lz/lz_decoder.c \
    xz/src/liblzma/lz/lz_encoder.c \
    xz/src/liblzma/lz/lz_encoder_mf.c \
    xz/src/liblzma/lzma/fastpos_table.c \
    xz/src/liblzma/lzma/fastpos_tablegen.c \
    xz/src/liblzma/lzma/lzma2_decoder.c \
    xz/src/liblzma/lzma/lzma2_encoder.c \
    xz/src/liblzma/lzma/lzma_decoder.c \
    xz/src/liblzma/lzma/lzma_encoder.c \
    xz/src/liblzma/lzma/lzma_encoder_optimum_fast.c \
    xz/src/liblzma/lzma/lzma_encoder_optimum_normal.c \
    xz/src/liblzma/lzma/lzma_encoder_presets.c \
    xz/src/liblzma/rangecoder/price_table.c \
    xz/src/liblzma/rangecoder/price_tablegen.c \
    xz/src/liblzma/simple/arm.c \
    xz/src/liblzma/simple/armthumb.c \
    xz/src/liblzma/simple/ia64.c \
    xz/src/liblzma/simple/powerpc.c \
    xz/src/liblzma/simple/simple_coder.c \
    xz/src/liblzma/simple/simple_decoder.c \
    xz/src/liblzma/simple/simple_encoder.c \
    xz/src/liblzma/simple/sparc.c \
    xz/src/liblzma/simple/x86.c
LOCAL_CFLAGS := -DHAVE_CONFIG_H -Wno-implicit-function-declaration
include $(BUILD_STATIC_LIBRARY)

SE_PATH := $(LOCAL_PATH)/selinux

# libsepol.a
include $(CLEAR_VARS)
LIBSEPOL := $(SE_PATH)/libsepol/include $(SE_PATH)/libsepol/cil/include
LOCAL_MODULE := libsepol
LOCAL_C_INCLUDES := $(LIBSEPOL) $(LOCAL_PATH)/selinux/libsepol/src
LOCAL_EXPORT_C_INCLUDES := $(LIBSEPOL)
LOCAL_SRC_FILES := \
    selinux/libsepol/src/assertion.c \
    selinux/libsepol/src/avrule_block.c \
    selinux/libsepol/src/avtab.c \
    selinux/libsepol/src/boolean_record.c \
    selinux/libsepol/src/booleans.c \
    selinux/libsepol/src/conditional.c \
    selinux/libsepol/src/constraint.c \
    selinux/libsepol/src/context.c \
    selinux/libsepol/src/context_record.c \
    selinux/libsepol/src/debug.c \
    selinux/libsepol/src/ebitmap.c \
    selinux/libsepol/src/expand.c \
    selinux/libsepol/src/handle.c \
    selinux/libsepol/src/hashtab.c \
    selinux/libsepol/src/hierarchy.c \
    selinux/libsepol/src/ibendport_record.c \
    selinux/libsepol/src/ibendports.c \
    selinux/libsepol/src/ibpkey_record.c \
    selinux/libsepol/src/ibpkeys.c \
    selinux/libsepol/src/iface_record.c \
    selinux/libsepol/src/interfaces.c \
    selinux/libsepol/src/kernel_to_cil.c \
    selinux/libsepol/src/kernel_to_common.c \
    selinux/libsepol/src/kernel_to_conf.c \
    selinux/libsepol/src/link.c \
    selinux/libsepol/src/mls.c \
    selinux/libsepol/src/module.c \
    selinux/libsepol/src/module_to_cil.c \
    selinux/libsepol/src/node_record.c \
    selinux/libsepol/src/nodes.c \
    selinux/libsepol/src/optimize.c \
    selinux/libsepol/src/polcaps.c \
    selinux/libsepol/src/policydb.c \
    selinux/libsepol/src/policydb_convert.c \
    selinux/libsepol/src/policydb_public.c \
    selinux/libsepol/src/policydb_validate.c \
    selinux/libsepol/src/port_record.c \
    selinux/libsepol/src/ports.c \
    selinux/libsepol/src/services.c \
    selinux/libsepol/src/sidtab.c \
    selinux/libsepol/src/symtab.c \
    selinux/libsepol/src/user_record.c \
    selinux/libsepol/src/users.c \
    selinux/libsepol/src/util.c \
    selinux/libsepol/src/write.c \
    selinux/libsepol/cil/src/cil.c \
    selinux/libsepol/cil/src/cil_binary.c \
    selinux/libsepol/cil/src/cil_build_ast.c \
    selinux/libsepol/cil/src/cil_copy_ast.c \
    selinux/libsepol/cil/src/cil_find.c \
    selinux/libsepol/cil/src/cil_fqn.c \
    selinux/libsepol/cil/src/cil_lexer.c \
    selinux/libsepol/cil/src/cil_list.c \
    selinux/libsepol/cil/src/cil_log.c \
    selinux/libsepol/cil/src/cil_mem.c \
    selinux/libsepol/cil/src/cil_parser.c \
    selinux/libsepol/cil/src/cil_policy.c \
    selinux/libsepol/cil/src/cil_post.c \
    selinux/libsepol/cil/src/cil_reset_ast.c \
    selinux/libsepol/cil/src/cil_resolve_ast.c \
    selinux/libsepol/cil/src/cil_stack.c \
    selinux/libsepol/cil/src/cil_strpool.c \
    selinux/libsepol/cil/src/cil_symtab.c \
    selinux/libsepol/cil/src/cil_tree.c \
    selinux/libsepol/cil/src/cil_verify.c \
    selinux/libsepol/cil/src/cil_write_ast.c

LOCAL_CFLAGS := -Wno-unused-but-set-variable
ifeq ($(TARGET_ARCH),riscv64)
LOCAL_CFLAGS += -DHAVE_REALLOCARRAY
endif
include $(BUILD_STATIC_LIBRARY)

# liblsplt.a
include $(CLEAR_VARS)
LOCAL_MODULE:= liblsplt
LOCAL_C_INCLUDES := $(LOCAL_PATH)/lsplt/lsplt/src/main/jni/include
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_C_INCLUDES)
LOCAL_CFLAGS := -Wall -Wextra -Werror -fvisibility=hidden -D__android_log_print=magisk_log_print
LOCAL_CPPFLAGS := -std=c++20
LOCAL_STATIC_LIBRARIES := libcxx
LOCAL_SRC_FILES := \
    lsplt/lsplt/src/main/jni/elf_util.cc \
    lsplt/lsplt/src/main/jni/lsplt.cc
include $(BUILD_STATIC_LIBRARY)

CWD := $(LOCAL_PATH)
include $(CWD)/system_properties/Android.mk
include $(CWD)/libcxx/Android.mk

ifdef B_CRT0
include $(CWD)/crt0/Android.mk
endif
