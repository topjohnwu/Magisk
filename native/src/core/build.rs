use pb_rs::{ConfigBuilder, types::FileDescriptor};

use crate::codegen::gen_cxx_binding;

#[path = "../include/codegen.rs"]
mod codegen;

fn main() {
    println!("cargo:rerun-if-changed=resetprop/proto/persistent_properties.proto");

    gen_cxx_binding("core-rs");

    let cb = ConfigBuilder::new(
        &["resetprop/proto/persistent_properties.proto"],
        None,
        Some(&"resetprop/proto"),
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
