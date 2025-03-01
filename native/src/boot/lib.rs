#![feature(format_args_nl)]
#![feature(btree_extract_if)]
#![feature(iter_intersperse)]
#![feature(try_blocks)]

pub use base;
use sign::{get_sha, sha256_hash, sign_boot_image, verify_boot_image, SHA};
use std::env;

mod cpio;
mod dtb;
mod patch;
mod payload;
// Suppress warnings in generated code
#[allow(warnings)]
mod proto;
mod sign;
mod cli;

#[cxx::bridge]
pub mod ffi {
    unsafe extern "C++" {
        include!("../base/include/base.hpp");

        #[namespace = "rust"]
        #[cxx_name = "Utf8CStr"]
        type Utf8CStrRef<'a> = base::ffi::Utf8CStrRef<'a>;
    }

    unsafe extern "C++" {
        include!("compress.hpp");
        fn decompress(buf: &[u8], fd: i32) -> bool;
        #[cxx_name = "decompress"]
        unsafe fn decompress_raw(infile: *mut c_char, outfile: *const c_char);
        unsafe fn compress(format: *const c_char, infile: *const c_char, outfile: *const c_char);
        fn xz(buf: &[u8], out: &mut Vec<u8>) -> bool;
        fn unxz(buf: &[u8], out: &mut Vec<u8>) -> bool;

        include!("bootimg.hpp");
        include!("magiskboot.hpp");
        #[cxx_name = "boot_img"]
        type BootImage;
        #[cxx_name = "get_payload"]
        fn payload(self: &BootImage) -> &[u8];
        #[cxx_name = "get_tail"]
        fn tail(self: &BootImage) -> &[u8];
        
        fn cleanup();

        unsafe fn unpack(image: *const c_char, skip_decomp: bool, hdr: bool) -> i32;
        unsafe fn repack(src_img: *const c_char, out_img: *const c_char, skip_comp: bool);
        unsafe fn verify(image: *const c_char, cert: *const c_char) -> i32;
        unsafe fn sign(image: *const c_char, name: *const c_char, cert: *const c_char, key: *const c_char) -> i32;
        unsafe fn split_image_dtb(filename: *const c_char, skip_decomp: bool) -> i32;

        fn formats() -> String;
    }

    extern "Rust" {
        type SHA;
        fn get_sha(use_sha1: bool) -> Box<SHA>;
        fn update(self: &mut SHA, data: &[u8]);
        fn finalize_into(self: &mut SHA, out: &mut [u8]);
        fn output_size(self: &SHA) -> usize;
        fn sha256_hash(data: &[u8], out: &mut [u8]);
    }

    #[namespace = "rust"]
    #[allow(unused_unsafe)]
    extern "Rust" {
        unsafe fn sign_boot_image(
            payload: &[u8],
            name: *const c_char,
            cert: *const c_char,
            key: *const c_char,
        ) -> Vec<u8>;
        unsafe fn verify_boot_image(img: &BootImage, cert: *const c_char) -> bool;
    }
}

#[inline(always)]
pub(crate) fn check_env(env: &str) -> bool {
    env::var(env).is_ok_and(|var| var == "true")
}
