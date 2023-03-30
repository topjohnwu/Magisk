pub use logging::*;

mod logging;

#[cxx::bridge(namespace = "rust")]
pub mod ffi2 {
    extern "Rust" {
        fn setup_klog();
    }
}
