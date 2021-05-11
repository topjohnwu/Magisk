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

# libnanopb.a
include $(CLEAR_VARS)
LOCAL_MODULE:= libnanopb
LOCAL_C_INCLUDES := $(LOCAL_PATH)/nanopb
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_C_INCLUDES)
LOCAL_SRC_FILES := \
    nanopb/pb_common.c \
    nanopb/pb_decode.c \
    nanopb/pb_encode.c
include $(BUILD_STATIC_LIBRARY)

# libfdt.a
include $(CLEAR_VARS)
LOCAL_MODULE:= libfdt
LOCAL_C_INCLUDES := $(LOCAL_PATH)/dtc/libfdt
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_C_INCLUDES)
LOCAL_SRC_FILES := \
    dtc/libfdt/fdt.c \
    dtc/libfdt/fdt_addresses.c \
    dtc/libfdt/fdt_empty_tree.c \
    dtc/libfdt/fdt_overlay.c \
    dtc/libfdt/fdt_ro.c \
    dtc/libfdt/fdt_rw.c \
    dtc/libfdt/fdt_strerror.c \
    dtc/libfdt/fdt_sw.c \
    dtc/libfdt/fdt_wip.c
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

# libbz2.a
include $(CLEAR_VARS)
LOCAL_MODULE := libbz2
LOCAL_C_INCLUDES := $(LOCAL_PATH)/bzip2
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_C_INCLUDES)
LOCAL_SRC_FILES := \
    bzip2/blocksort.c  \
    bzip2/huffman.c    \
    bzip2/crctable.c   \
    bzip2/randtable.c  \
    bzip2/compress.c   \
    bzip2/decompress.c \
    bzip2/bzlib.c
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
    selinux/libsepol/src/deprecated_funcs.c \
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
    selinux/libsepol/src/port_record.c \
    selinux/libsepol/src/ports.c \
    selinux/libsepol/src/roles.c \
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
    selinux/libsepol/cil/src/cil_verify.c
LOCAL_CFLAGS := -Dgetline=__getline -Wno-implicit-function-declaration
include $(BUILD_STATIC_LIBRARY)

# libselinux.a
include $(CLEAR_VARS)
LIBSELINUX := $(SE_PATH)/libselinux/include
LOCAL_MODULE:= libselinux
LOCAL_C_INCLUDES := $(LIBSELINUX)
LOCAL_EXPORT_C_INCLUDES := $(LIBSELINUX)
LOCAL_STATIC_LIBRARIES := libpcre2
LOCAL_CFLAGS := \
    -Wno-implicit-function-declaration -Wno-int-conversion -Wno-unused-function \
    -Wno-macro-redefined -D_GNU_SOURCE -DUSE_PCRE2 \
    -DNO_PERSISTENTLY_STORED_PATTERNS -DDISABLE_SETRANS -DDISABLE_BOOL \
    -DNO_MEDIA_BACKEND -DNO_X_BACKEND -DNO_DB_BACKEND -DNO_ANDROID_BACKEND \
    -Dfgets_unlocked=fgets -D'__fsetlocking(...)='
LOCAL_SRC_FILES := \
    selinux/libselinux/src/avc.c \
    selinux/libselinux/src/avc_internal.c \
    selinux/libselinux/src/avc_sidtab.c \
    selinux/libselinux/src/booleans.c \
    selinux/libselinux/src/callbacks.c \
    selinux/libselinux/src/canonicalize_context.c \
    selinux/libselinux/src/checkAccess.c \
    selinux/libselinux/src/check_context.c \
    selinux/libselinux/src/checkreqprot.c \
    selinux/libselinux/src/compute_av.c \
    selinux/libselinux/src/compute_create.c \
    selinux/libselinux/src/compute_member.c \
    selinux/libselinux/src/compute_relabel.c \
    selinux/libselinux/src/compute_user.c \
    selinux/libselinux/src/context.c \
    selinux/libselinux/src/deny_unknown.c \
    selinux/libselinux/src/disable.c \
    selinux/libselinux/src/enabled.c \
    selinux/libselinux/src/fgetfilecon.c \
    selinux/libselinux/src/freecon.c \
    selinux/libselinux/src/freeconary.c \
    selinux/libselinux/src/fsetfilecon.c \
    selinux/libselinux/src/get_context_list.c \
    selinux/libselinux/src/get_default_type.c \
    selinux/libselinux/src/get_initial_context.c \
    selinux/libselinux/src/getenforce.c \
    selinux/libselinux/src/getfilecon.c \
    selinux/libselinux/src/getpeercon.c \
    selinux/libselinux/src/init.c \
    selinux/libselinux/src/is_customizable_type.c \
    selinux/libselinux/src/label.c \
    selinux/libselinux/src/label_file.c \
    selinux/libselinux/src/label_support.c \
    selinux/libselinux/src/lgetfilecon.c \
    selinux/libselinux/src/load_policy.c \
    selinux/libselinux/src/lsetfilecon.c \
    selinux/libselinux/src/mapping.c \
    selinux/libselinux/src/matchmediacon.c \
    selinux/libselinux/src/matchpathcon.c \
    selinux/libselinux/src/policyvers.c \
    selinux/libselinux/src/procattr.c \
    selinux/libselinux/src/query_user_context.c \
    selinux/libselinux/src/regex.c \
    selinux/libselinux/src/reject_unknown.c \
    selinux/libselinux/src/selinux_check_securetty_context.c \
    selinux/libselinux/src/selinux_config.c \
    selinux/libselinux/src/selinux_restorecon.c \
    selinux/libselinux/src/sestatus.c \
    selinux/libselinux/src/setenforce.c \
    selinux/libselinux/src/setexecfilecon.c \
    selinux/libselinux/src/setfilecon.c \
    selinux/libselinux/src/setrans_client.c \
    selinux/libselinux/src/seusers.c \
    selinux/libselinux/src/sha1.c \
    selinux/libselinux/src/stringrep.c \
    selinux/libselinux/src/validatetrans.c
include $(BUILD_STATIC_LIBRARY)

# libpcre2.a
include $(CLEAR_VARS)
LIBPCRE2 := $(LOCAL_PATH)/pcre/include
LOCAL_MODULE:= libpcre2
LOCAL_CFLAGS := -DHAVE_CONFIG_H
LOCAL_C_INCLUDES := $(LIBPCRE2) $(LIBPCRE2)_internal
LOCAL_EXPORT_C_INCLUDES := $(LIBPCRE2)
LOCAL_SRC_FILES := \
    pcre/dist2/src/pcre2_auto_possess.c \
    pcre/dist2/src/pcre2_chartables.c \
    pcre/dist2/src/pcre2_compile.c \
    pcre/dist2/src/pcre2_config.c \
    pcre/dist2/src/pcre2_context.c \
    pcre/dist2/src/pcre2_convert.c \
    pcre/dist2/src/pcre2_dfa_match.c \
    pcre/dist2/src/pcre2_error.c \
    pcre/dist2/src/pcre2_extuni.c \
    pcre/dist2/src/pcre2_find_bracket.c \
    pcre/dist2/src/pcre2_fuzzsupport.c \
    pcre/dist2/src/pcre2_jit_compile.c \
    pcre/dist2/src/pcre2_maketables.c \
    pcre/dist2/src/pcre2_match.c \
    pcre/dist2/src/pcre2_match_data.c \
    pcre/dist2/src/pcre2_newline.c \
    pcre/dist2/src/pcre2_ord2utf.c \
    pcre/dist2/src/pcre2_pattern_info.c \
    pcre/dist2/src/pcre2_script_run.c \
    pcre/dist2/src/pcre2_serialize.c \
    pcre/dist2/src/pcre2_string_utils.c \
    pcre/dist2/src/pcre2_study.c \
    pcre/dist2/src/pcre2_substitute.c \
    pcre/dist2/src/pcre2_substring.c \
    pcre/dist2/src/pcre2_tables.c \
    pcre/dist2/src/pcre2_ucd.c \
    pcre/dist2/src/pcre2_valid_utf.c \
    pcre/dist2/src/pcre2_xclass.c
include $(BUILD_STATIC_LIBRARY)

# libxhook.a
include $(CLEAR_VARS)
LOCAL_MODULE:= libxhook
LOCAL_C_INCLUDES := $(LOCAL_PATH)/xhook/libxhook/jni
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_C_INCLUDES)
LOCAL_CFLAGS := -Wall -Wextra -Werror -fvisibility=hidden
LOCAL_CONLYFLAGS := -std=c11
LOCAL_SRC_FILES := \
    xhook/libxhook/jni/xh_log.c \
    xhook/libxhook/jni/xh_version.c \
    xhook/libxhook/jni/xh_jni.c \
    xhook/libxhook/jni/xhook.c \
    xhook/libxhook/jni/xh_core.c \
    xhook/libxhook/jni/xh_util.c \
    xhook/libxhook/jni/xh_elf.c
include $(BUILD_STATIC_LIBRARY)

# libz.a
include $(CLEAR_VARS)
LOCAL_MODULE:= libz
LOCAL_C_INCLUDES := $(LOCAL_PATH)/zlib
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_C_INCLUDES)
LOCAL_CFLAGS := -DHAVE_HIDDEN -DZLIB_CONST -Wall -Werror -Wno-unused -Wno-unused-parameter
LOCAL_SRC_FILES := \
    zlib/adler32.c \
    zlib/compress.c \
    zlib/cpu_features.c \
    zlib/crc32.c \
    zlib/deflate.c \
    zlib/gzclose.c \
    zlib/gzlib.c \
    zlib/gzread.c \
    zlib/gzwrite.c \
    zlib/infback.c \
    zlib/inffast.c \
    zlib/inflate.c \
    zlib/inftrees.c \
    zlib/trees.c \
    zlib/uncompr.c \
    zlib/zutil.c
include $(BUILD_STATIC_LIBRARY)

CWD := $(LOCAL_PATH)
include $(CWD)/systemproperties/Android.mk
include $(CWD)/mincrypt/Android.mk
include $(CWD)/libcxx/Android.mk
