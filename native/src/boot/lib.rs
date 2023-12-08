#![feature(format_args_nl)]
#![feature(btree_extract_if)]
#![feature(iter_intersperse)]

pub use base;
use cpio::cpio_commands;
use dtb::dtb_commands;
use patch::hexpatch;
use payload::extract_boot_from_payload;
use sign::{get_sha, sha1_hash, sha256_hash, sign_boot_image, verify_boot_image, SHA};
use std::env;

mod cpio;
mod dtb;
mod patch;
mod payload;
// Suppress warnings in generated code
#[allow(warnings)]
mod proto;
mod ramdisk;
mod sign;

#[cxx::bridge]
pub mod ffi {
    unsafe extern "C++" {
        include!("compress.hpp");
        fn decompress(buf: &[u8], fd: i32) -> bool;
        fn xz(buf: &[u8], out: &mut Vec<u8>) -> bool;
        fn unxz(buf: &[u8], out: &mut Vec<u8>) -> bool;

        include!("bootimg.hpp");
        #[cxx_name = "boot_img"]
        type BootImage;
        #[cxx_name = "get_payload"]
        fn payload(self: &BootImage) -> &[u8];
        #[cxx_name = "get_tail"]
        fn tail(self: &BootImage) -> &[u8];
    }

    extern "Rust" {
        type SHA;
        fn get_sha(use_sha1: bool) -> Box<SHA>;
        fn update(self: &mut SHA, data: &[u8]);
        fn finalize_into(self: &mut SHA, out: &mut [u8]);
        fn output_size(self: &SHA) -> usize;
        fn sha1_hash(data: &[u8], out: &mut [u8]);
        fn sha256_hash(data: &[u8], out: &mut [u8]);

        fn hexpatch(file: &[u8], from: &[u8], to: &[u8]) -> bool;
    }

    #[namespace = "rust"]
    extern "Rust" {
        unsafe fn extract_boot_from_payload(
            partition: *const c_char,
            in_path: *const c_char,
            out_path: *const c_char,
        ) -> bool;
        unsafe fn cpio_commands(argc: i32, argv: *const *const c_char) -> bool;
        unsafe fn verify_boot_image(img: &BootImage, cert: *const c_char) -> bool;
        unsafe fn sign_boot_image(
            payload: &[u8],
            name: *const c_char,
            cert: *const c_char,
            key: *const c_char,
        ) -> Vec<u8>;
        unsafe fn dtb_commands(argc: i32, argv: *const *const c_char) -> bool;
    }
}

#[inline(always)]
pub(crate) fn check_env(env: &str) -> bool {
    env::var(env).map_or(false, |var| var == "true")
}
