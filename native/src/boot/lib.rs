#![feature(format_args_nl)]
#![feature(btree_drain_filter)]
#![feature(iter_collect_into)]

extern crate core;

pub use base;
pub use cpio::*;
pub use payload::*;
pub use ramdisk::*;

mod cpio;
mod payload;
mod ramdisk;
mod update_metadata;

#[cxx::bridge]
pub mod ffi {
    unsafe extern "C++" {
        include!("compress.hpp");
        include!("magiskboot.hpp");
        fn decompress(buf: &[u8], fd: i32) -> bool;
        fn patch_encryption(buf: &mut [u8]) -> usize;
        fn patch_verity(buf: &mut [u8]) -> usize;
    }

    #[namespace = "rust"]
    extern "Rust" {
        unsafe fn extract_boot_from_payload(
            partition: *const c_char,
            in_path: *const c_char,
            out_path: *const c_char,
        ) -> bool;

        unsafe fn cpio_commands(argc: i32, argv: *const *const c_char) -> bool;
    }
}
