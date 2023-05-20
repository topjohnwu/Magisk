#![feature(format_args_nl)]

pub use base;
pub use payload::*;

mod payload;
mod update_metadata;

#[cxx::bridge]
pub mod ffi {
    extern "C++" {
        include!("compress.hpp");
        pub unsafe fn decompress(in_: *const u8, in_size: u64, fd: i32) -> bool;
    }

    #[namespace = "rust"]
    extern "Rust" {
        unsafe fn extract_boot_from_payload(
            in_path: *const c_char,
            out_path: *const c_char,
        ) -> bool;
    }
}
