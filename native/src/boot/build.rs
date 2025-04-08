use pb_rs::{ConfigBuilder, types::FileDescriptor};

use crate::codegen::gen_cxx_binding;

#[path = "../include/codegen.rs"]
mod codegen;

fn main() {
    println!("cargo:rerun-if-changed=proto/update_metadata.proto");

    gen_cxx_binding("boot-rs");

    let cb = ConfigBuilder::new(
        &["proto/update_metadata.proto"],
        None,
        Some(&"proto"),
        &["."],
    )
    .unwrap();
    FileDescriptor::run(
        &cb.single_module(true)
            .dont_use_cow(true)
            .generate_getters(true)
            .build(),
    )
    .unwrap();
}
