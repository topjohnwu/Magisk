use pb_rs::{types::FileDescriptor, ConfigBuilder};

use crate::gen::gen_cxx_binding;

#[path = "../include/gen.rs"]
mod gen;

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
