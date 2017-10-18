LOCAL_PATH:= $(call my-dir)

# libsqlite.so (stub)
include $(CLEAR_VARS)
LOCAL_MODULE:= libsqlite
LOCAL_SRC_FILES := stubs/sqlite3_stub.c
include $(BUILD_SHARED_LIBRARY)

# libselinux.so (stub)
include $(CLEAR_VARS)
LOCAL_MODULE:= libselinux
LOCAL_C_INCLUDES := $(LIBSELINUX)
LOCAL_SRC_FILES := stubs/selinux_stub.c
include $(BUILD_SHARED_LIBRARY)

# libfdt.a
include $(CLEAR_VARS)
LOCAL_MODULE:= libfdt
LOCAL_C_INCLUDES := $(LIBFDT)
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
LOCAL_C_INCLUDES += $(LIBLZ4)
LOCAL_SRC_FILES := \
	lz4/lib/lz4.c \
	lz4/lib/lz4frame.c \
	lz4/lib/lz4hc.c \
	lz4/lib/xxhash.c
include $(BUILD_STATIC_LIBRARY)

# libbz2.a
include $(CLEAR_VARS)
LOCAL_MODULE := libbz2
LOCAL_C_INCLUDES += $(LIBBZ2)
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
LOCAL_C_INCLUDES += \
	$(EXT_PATH)/xz_config \
	$(EXT_PATH)/xz/src/common \
	$(EXT_PATH)/xz/src/liblzma/api \
	$(EXT_PATH)/xz/src/liblzma/check \
	$(EXT_PATH)/xz/src/liblzma/common \
	$(EXT_PATH)/xz/src/liblzma/delta \
	$(EXT_PATH)/xz/src/liblzma/lz \
	$(EXT_PATH)/xz/src/liblzma/lzma \
	$(EXT_PATH)/xz/src/liblzma/rangecoder \
	$(EXT_PATH)/xz/src/liblzma/simple \
	$(EXT_PATH)/xz/src/liblzma
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
LOCAL_CFLAGS += -DHAVE_CONFIG_H -std=c99
include $(BUILD_STATIC_LIBRARY)

# libsepol.a
include $(SE_PATH)/libsepol/Android.mk
