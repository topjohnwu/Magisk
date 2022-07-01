pub use base;

#[cxx::bridge]
pub mod ffi {
    extern "Rust" {
        fn rust_test_entry();
    }
}

fn rust_test_entry() {}
