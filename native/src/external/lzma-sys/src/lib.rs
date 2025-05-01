#![allow(bad_style)]
#![doc(html_root_url = "https://docs.rs/lzma-sys/0.1")]

use libc::{c_char, c_uchar, c_void, size_t};
use std::u64;

#[cfg(target_env = "msvc")]
#[doc(hidden)]
pub type __enum_ty = libc::c_int;
#[cfg(not(target_env = "msvc"))]
#[doc(hidden)]
pub type __enum_ty = libc::c_uint;
pub type lzma_bool = c_uchar;
pub type lzma_ret = __enum_ty;
pub type lzma_action = __enum_ty;
type lzma_reserved_enum = __enum_ty;
pub type lzma_check = __enum_ty;
pub type lzma_vli = u64;
pub type lzma_mode = __enum_ty;
pub type lzma_match_finder = __enum_ty;

pub const LZMA_OK: lzma_ret = 0;
pub const LZMA_STREAM_END: lzma_ret = 1;
pub const LZMA_NO_CHECK: lzma_ret = 2;
pub const LZMA_UNSUPPORTED_CHECK: lzma_ret = 3;
pub const LZMA_GET_CHECK: lzma_ret = 4;
pub const LZMA_MEM_ERROR: lzma_ret = 5;
pub const LZMA_MEMLIMIT_ERROR: lzma_ret = 6;
pub const LZMA_FORMAT_ERROR: lzma_ret = 7;
pub const LZMA_OPTIONS_ERROR: lzma_ret = 8;
pub const LZMA_DATA_ERROR: lzma_ret = 9;
pub const LZMA_BUF_ERROR: lzma_ret = 10;
pub const LZMA_PROG_ERROR: lzma_ret = 11;

pub const LZMA_RUN: lzma_action = 0;
pub const LZMA_SYNC_FLUSH: lzma_action = 1;
pub const LZMA_FULL_FLUSH: lzma_action = 2;
pub const LZMA_FULL_BARRIER: lzma_action = 4;
pub const LZMA_FINISH: lzma_action = 3;

pub const LZMA_CHECK_NONE: lzma_check = 0;
pub const LZMA_CHECK_CRC32: lzma_check = 1;
pub const LZMA_CHECK_CRC64: lzma_check = 4;
pub const LZMA_CHECK_SHA256: lzma_check = 10;

pub const LZMA_MODE_FAST: lzma_mode = 1;
pub const LZMA_MODE_NORMAL: lzma_mode = 2;

pub const LZMA_MF_HC3: lzma_match_finder = 0x03;
pub const LZMA_MF_HC4: lzma_match_finder = 0x04;
pub const LZMA_MF_BT2: lzma_match_finder = 0x12;
pub const LZMA_MF_BT3: lzma_match_finder = 0x13;
pub const LZMA_MF_BT4: lzma_match_finder = 0x14;

pub const LZMA_TELL_NO_CHECK: u32 = 0x01;
pub const LZMA_TELL_UNSUPPORTED_CHECK: u32 = 0x02;
pub const LZMA_TELL_ANY_CHECK: u32 = 0x04;
pub const LZMA_IGNORE_CHECK: u32 = 0x10;
pub const LZMA_CONCATENATED: u32 = 0x08;

pub const LZMA_PRESET_DEFAULT: u32 = 6;
pub const LZMA_PRESET_LEVEL_MASK: u32 = 0x1f;
pub const LZMA_PRESET_EXTREME: u32 = 1 << 31;

pub const LZMA_DICT_SIZE_MIN: u32 = 4096;
pub const LZMA_DICT_SIZE_DEFAULT: u32 = 1 << 23;

pub const LZMA_LCLP_MIN: u32 = 0;
pub const LZMA_LCLP_MAX: u32 = 4;
pub const LZMA_LC_DEFAULT: u32 = 3;

pub const LZMA_LP_DEFAULT: u32 = 0;

pub const LZMA_PB_MIN: u32 = 0;
pub const LZMA_PB_MAX: u32 = 4;
pub const LZMA_PB_DEFAULT: u32 = 2;

pub const LZMA_BACKWARD_SIZE_MIN: lzma_vli = 4;
pub const LZMA_BACKWARD_SIZE_MAX: lzma_vli = 1 << 34;

pub const LZMA_VLI_MAX: lzma_vli = u64::MAX / 2;
pub const LZMA_VLI_UNKNOWN: lzma_vli = u64::MAX;
pub const LZMA_VLI_BYTES_MAX: usize = 9;

pub const LZMA_FILTER_X86: lzma_vli = 0x04;
pub const LZMA_FILTER_POWERPC: lzma_vli = 0x05;
pub const LZMA_FILTER_IA64: lzma_vli = 0x06;
pub const LZMA_FILTER_ARM: lzma_vli = 0x07;
pub const LZMA_FILTER_ARMTHUMB: lzma_vli = 0x08;
pub const LZMA_FILTER_SPARC: lzma_vli = 0x09;
pub const LZMA_FILTER_LZMA1: lzma_vli = 0x4000000000000001;
pub const LZMA_FILTER_LZMA2: lzma_vli = 0x21;

#[repr(C)]
pub struct lzma_allocator {
    pub alloc: Option<extern "C" fn(*mut c_void, size_t, size_t) -> *mut c_void>,
    pub free: Option<extern "C" fn(*mut c_void, *mut c_void)>,
    pub opaque: *mut c_void,
}

pub enum lzma_internal {}

#[repr(C)]
pub struct lzma_stream {
    pub next_in: *const u8,
    pub avail_in: size_t,
    pub total_in: u64,
    pub next_out: *mut u8,
    pub avail_out: size_t,
    pub total_out: u64,
    pub allocator: *const lzma_allocator,

    internal: *mut lzma_internal,
    reserved_ptr1: *mut c_void,
    reserved_ptr2: *mut c_void,
    reserved_ptr3: *mut c_void,
    reserved_ptr4: *mut c_void,
    reserved_int1: u64,
    reserved_int2: u64,
    reserved_int3: size_t,
    reserved_int4: size_t,
    reserved_enum1: lzma_reserved_enum,
    reserved_enum2: lzma_reserved_enum,
}

#[repr(C)]
pub struct lzma_filter {
    pub id: lzma_vli,
    pub options: *mut c_void,
}

#[repr(C)]
pub struct lzma_mt {
    pub flags: u32,
    pub threads: u32,
    pub block_size: u64,
    pub timeout: u32,
    pub preset: u32,
    pub filters: *const lzma_filter,
    pub check: lzma_check,

    reserved_enum1: lzma_reserved_enum,
    reserved_enum2: lzma_reserved_enum,
    reserved_enum3: lzma_reserved_enum,
    reserved_int1: u32,
    reserved_int2: u32,
    reserved_int3: u32,
    reserved_int4: u32,
    reserved_int5: u64,
    reserved_int6: u64,
    reserved_int7: u64,
    reserved_int8: u64,
    reserved_ptr1: *mut c_void,
    reserved_ptr2: *mut c_void,
    reserved_ptr3: *mut c_void,
    reserved_ptr4: *mut c_void,
}

#[repr(C)]
#[derive(Copy, Clone)]
pub struct lzma_options_lzma {
    pub dict_size: u32,
    pub preset_dict: *const u8,
    pub preset_dict_size: u32,
    pub lc: u32,
    pub lp: u32,
    pub pb: u32,
    pub mode: lzma_mode,
    pub nice_len: u32,
    pub mf: lzma_match_finder,
    pub depth: u32,

    reserved_int1: u32,
    reserved_int2: u32,
    reserved_int3: u32,
    reserved_int4: u32,
    reserved_int5: u32,
    reserved_int6: u32,
    reserved_int7: u32,
    reserved_int8: u32,
    reserved_enum1: lzma_reserved_enum,
    reserved_enum2: lzma_reserved_enum,
    reserved_enum3: lzma_reserved_enum,
    reserved_enum4: lzma_reserved_enum,
    reserved_ptr1: *mut c_void,
    reserved_ptr2: *mut c_void,
}

#[repr(C)]
pub struct lzma_stream_flags {
    pub version: u32,
    pub backward_size: lzma_vli,
    pub check: lzma_check,

    reserved_enum1: lzma_reserved_enum,
    reserved_enum2: lzma_reserved_enum,
    reserved_enum3: lzma_reserved_enum,
    reserved_enum4: lzma_reserved_enum,
    reserved_bool1: lzma_bool,
    reserved_bool2: lzma_bool,
    reserved_bool3: lzma_bool,
    reserved_bool4: lzma_bool,
    reserved_bool5: lzma_bool,
    reserved_bool6: lzma_bool,
    reserved_bool7: lzma_bool,
    reserved_bool8: lzma_bool,
    reserved_int1: u32,
    reserved_int2: u32,
}

#[repr(C)]
pub struct lzma_options_bcj {
    pub start_offset: u32,
}

extern "C" {
    pub fn lzma_code(strm: *mut lzma_stream, action: lzma_action) -> lzma_ret;
    pub fn lzma_end(strm: *mut lzma_stream);
    pub fn lzma_get_progress(strm: *mut lzma_stream, progress_in: *mut u64, progress_out: *mut u64);
    pub fn lzma_memusage(strm: *const lzma_stream) -> u64;
    pub fn lzma_memlimit_get(strm: *const lzma_stream) -> u64;
    pub fn lzma_memlimit_set(strm: *mut lzma_stream, memlimit: u64) -> lzma_ret;

    pub fn lzma_easy_encoder_memusage(preset: u32) -> u64;
    pub fn lzma_easy_decoder_memusage(preset: u32) -> u64;
    pub fn lzma_easy_encoder(strm: *mut lzma_stream, preset: u32, check: lzma_check) -> lzma_ret;
    pub fn lzma_easy_buffer_encode(
        preset: u32,
        check: lzma_check,
        allocator: *const lzma_allocator,
        input: *const u8,
        in_size: size_t,
        out: *mut u8,
        out_pos: *mut size_t,
        out_size: size_t,
    ) -> lzma_ret;

    pub fn lzma_stream_encoder(
        strm: *mut lzma_stream,
        filters: *const lzma_filter,
        check: lzma_check,
    ) -> lzma_ret;
    pub fn lzma_stream_encoder_mt_memusage(options: *const lzma_mt) -> u64;
    pub fn lzma_stream_encoder_mt(strm: *mut lzma_stream, options: *const lzma_mt) -> lzma_ret;

    pub fn lzma_alone_encoder(
        strm: *mut lzma_stream,
        options: *const lzma_options_lzma,
    ) -> lzma_ret;

    pub fn lzma_stream_buffer_bound(uncompressed_size: size_t) -> size_t;
    pub fn lzma_stream_buffer_encode(
        filters: *mut lzma_filter,
        check: lzma_check,
        allocator: *const lzma_allocator,
        input: *const u8,
        in_size: size_t,
        out: *mut u8,
        out_pos: *mut size_t,
        out_size: size_t,
    ) -> lzma_ret;

    pub fn lzma_stream_decoder(strm: *mut lzma_stream, memlimit: u64, flags: u32) -> lzma_ret;
    pub fn lzma_auto_decoder(strm: *mut lzma_stream, memlimit: u64, flags: u32) -> lzma_ret;
    pub fn lzma_alone_decoder(strm: *mut lzma_stream, memlimit: u64) -> lzma_ret;
    pub fn lzma_stream_buffer_decode(
        memlimit: *mut u64,
        flags: u32,
        allocator: *const lzma_allocator,
        input: *const u8,
        in_pos: *mut size_t,
        in_size: size_t,
        out: *mut u8,
        out_pos: *mut size_t,
        out_size: size_t,
    ) -> lzma_ret;

    pub fn lzma_check_is_supported(check: lzma_check) -> lzma_bool;
    pub fn lzma_check_size(check: lzma_check) -> u32;

    pub fn lzma_crc32(buf: *const u8, size: size_t, crc: u32) -> u32;
    pub fn lzma_crc64(buf: *const u8, size: size_t, crc: u64) -> u64;
    pub fn lzma_get_check(strm: *const lzma_stream) -> lzma_check;

    pub fn lzma_filter_encoder_is_supported(id: lzma_vli) -> lzma_bool;
    pub fn lzma_filter_decoder_is_supported(id: lzma_vli) -> lzma_bool;
    pub fn lzma_filters_copy(
        src: *const lzma_filter,
        dest: *mut lzma_filter,
        allocator: *const lzma_allocator,
    ) -> lzma_ret;
    pub fn lzma_raw_encoder_memusage(filters: *const lzma_filter) -> u64;
    pub fn lzma_raw_decoder_memusage(filters: *const lzma_filter) -> u64;
    pub fn lzma_raw_encoder(strm: *mut lzma_stream, filters: *const lzma_filter) -> lzma_ret;
    pub fn lzma_raw_decoder(strm: *mut lzma_stream, filters: *const lzma_filter) -> lzma_ret;
    pub fn lzma_filters_update(strm: *mut lzma_stream, filters: *const lzma_filter) -> lzma_ret;
    pub fn lzma_raw_buffer_encode(
        filters: *const lzma_filter,
        allocator: *const lzma_allocator,
        input: *const u8,
        in_size: size_t,
        out: *mut u8,
        out_pos: *mut size_t,
        out_size: size_t,
    ) -> lzma_ret;
    pub fn lzma_raw_buffer_decode(
        filters: *const lzma_filter,
        allocator: *const lzma_allocator,
        input: *const u8,
        in_pos: *mut size_t,
        in_size: size_t,
        out: *mut u8,
        out_pos: *mut size_t,
        out_size: size_t,
    ) -> lzma_ret;
    pub fn lzma_properties_size(size: *mut u32, filter: *const lzma_filter) -> lzma_ret;
    pub fn lzma_properties_encode(filter: *const lzma_filter, props: *mut u8) -> lzma_ret;
    pub fn lzma_properties_decode(
        filter: *mut lzma_filter,
        allocator: *const lzma_allocator,
        props: *const u8,
        props_size: size_t,
    ) -> lzma_ret;
    pub fn lzma_physmem() -> u64;
    pub fn lzma_cputhreads() -> u32;

    pub fn lzma_stream_header_encode(options: *const lzma_stream_flags, out: *mut u8) -> lzma_ret;
    pub fn lzma_stream_footer_encode(options: *const lzma_stream_flags, out: *mut u8) -> lzma_ret;
    pub fn lzma_stream_header_decode(options: *mut lzma_stream_flags, input: *const u8)
        -> lzma_ret;
    pub fn lzma_stream_footer_decode(options: *mut lzma_stream_flags, input: *const u8)
        -> lzma_ret;
    pub fn lzma_stream_flags_compare(
        a: *const lzma_stream_flags,
        b: *const lzma_stream_flags,
    ) -> lzma_ret;

    pub fn lzma_version_number() -> u32;
    pub fn lzma_version_string() -> *const c_char;

    pub fn lzma_vli_encode(
        vli: lzma_vli,
        vli_pos: *mut size_t,
        out: *mut u8,
        out_pos: *mut size_t,
        out_size: size_t,
    ) -> lzma_ret;
    pub fn lzma_vli_decode(
        vli: *mut lzma_vli,
        vli_pos: *mut size_t,
        input: *const u8,
        in_pos: *mut size_t,
        in_size: size_t,
    ) -> lzma_ret;
    pub fn lzma_vli_size(vli: lzma_vli) -> u32;

    pub fn lzma_lzma_preset(options: *mut lzma_options_lzma, preset: u32) -> lzma_bool;
    pub fn lzma_mf_is_supported(mf: lzma_match_finder) -> lzma_bool;
}
