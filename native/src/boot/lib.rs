#![feature(format_args_nl)]

extern crate core;

pub use base;
pub use payload::*;

mod payload;
mod update_metadata;

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
    }
}
