#![feature(format_args_nl)]
#![feature(btree_drain_filter)]

pub use base;
use cpio::*;
use patch::*;
use payload::*;

mod cpio;
mod patch;
mod payload;
// Suppress warnings in generated code
#[allow(warnings)]
mod proto;
mod ramdisk;

#[cxx::bridge]
pub mod ffi {
    unsafe extern "C++" {
        include!("compress.hpp");
        fn decompress(buf: &[u8], fd: i32) -> bool;
    }

    #[namespace = "rust"]
    extern "Rust" {
        unsafe fn extract_boot_from_payload(
            partition: *const c_char,
            in_path: *const c_char,
            out_path: *const c_char,
        ) -> bool;

        unsafe fn cpio_commands(argc: i32, argv: *const *const c_char) -> bool;
        fn hexpatch(file: &[u8], from: &[u8], to: &[u8]) -> bool;
        fn patch_encryption(buf: &mut [u8]) -> usize;
        fn patch_verity(buf: &mut [u8]) -> usize;
    }
}
