#![feature(format_args_nl)]
#![feature(btree_extract_if)]
#![feature(iter_intersperse)]
#![feature(try_blocks)]

pub use base;
use compress::{compress_bytes, decompress_bytes};
use format::fmt2name;
use sign::{SHA, get_sha, sha256_hash, sign_payload_for_cxx};
use std::env;

mod compress;
mod cpio;
mod dtb;
mod patch;
mod payload;
// Suppress warnings in generated code
mod cli;
mod format;
#[allow(warnings)]
mod proto;
mod sign;

#[cxx::bridge]
pub mod ffi {
    enum FileFormat {
        UNKNOWN,
        /* Boot formats */
        CHROMEOS,
        AOSP,
        AOSP_VENDOR,
        DHTB,
        BLOB,
        /* Compression formats */
        GZIP,
        ZOPFLI,
        XZ,
        LZMA,
        BZIP2,
        LZ4,
        LZ4_LEGACY,
        LZ4_LG,
        /* Unsupported compression */
        LZOP,
        /* Misc */
        MTK,
        DTB,
        ZIMAGE,
    }

    unsafe extern "C++" {
        include!("../base/include/base.hpp");

        #[namespace = "rust"]
        #[cxx_name = "Utf8CStr"]
        type Utf8CStrRef<'a> = base::ffi::Utf8CStrRef<'a>;
    }

    unsafe extern "C++" {
        include!("format.hpp");
        fn check_fmt(buf: &[u8]) -> FileFormat;

        include!("magiskboot.hpp");
        fn cleanup();
        fn unpack(image: Utf8CStrRef, skip_decomp: bool, hdr: bool) -> i32;
        fn repack(src_img: Utf8CStrRef, out_img: Utf8CStrRef, skip_comp: bool);
        fn split_image_dtb(filename: Utf8CStrRef, skip_decomp: bool) -> i32;
    }

    extern "Rust" {
        type SHA;
        fn get_sha(use_sha1: bool) -> Box<SHA>;
        fn update(self: &mut SHA, data: &[u8]);
        fn finalize_into(self: &mut SHA, out: &mut [u8]);
        fn output_size(self: &SHA) -> usize;
        fn sha256_hash(data: &[u8], out: &mut [u8]);

        fn compress_bytes(format: FileFormat, in_bytes: &[u8], out_fd: i32);
        fn decompress_bytes(format: FileFormat, in_bytes: &[u8], out_fd: i32);
        fn fmt2name(fmt: FileFormat) -> *const c_char;

        #[cxx_name = "sign_payload"]
        fn sign_payload_for_cxx(payload: &[u8]) -> Vec<u8>;
    }

    // BootImage FFI
    unsafe extern "C++" {
        include!("bootimg.hpp");
        #[cxx_name = "boot_img"]
        type BootImage;

        #[cxx_name = "get_payload"]
        fn payload(self: &BootImage) -> &[u8];
        #[cxx_name = "get_tail"]
        fn tail(self: &BootImage) -> &[u8];
        fn is_signed(self: &BootImage) -> bool;
        fn tail_off(self: &BootImage) -> u64;

        #[Self = BootImage]
        #[cxx_name = "create"]
        fn new(img: Utf8CStrRef) -> UniquePtr<BootImage>;
    }
    extern "Rust" {
        #[cxx_name = "verify"]
        fn verify_for_cxx(self: &BootImage) -> bool;
    }
}

#[inline(always)]
pub(crate) fn check_env(env: &str) -> bool {
    env::var(env).is_ok_and(|var| var == "true")
}
