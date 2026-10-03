use pb_rs::ConfigBuilder;
use pb_rs::types::FileDescriptor;

use crate::codegen::gen_cxx_binding;

#[path = "../include/codegen.rs"]
mod codegen;

#[allow(clippy::unwrap_used)]
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

    let target_os = std::env::var("CARGO_CFG_TARGET_OS").unwrap_or_default();
    if target_os != "android" {
        // Compile liblz4 C sources to satisfy lz4-sys extern symbols on host
        let mut lz4_build = cc::Build::new();
        lz4_build
            .include("../external/lz4/lib")
            .file("../external/lz4/lib/lz4.c")
            .file("../external/lz4/lib/lz4frame.c")
            .file("../external/lz4/lib/lz4hc.c")
            .file("../external/lz4/lib/xxhash.c")
            .warnings(false)
            .compile("lz4");

        // Compile magiskboot C++ sources
        let mut build = cc::Build::new();
        build
            .cpp(true)
            .std("c++23")
            .define("_GNU_SOURCE", None)
            .include("../include")
            .include("../base/include")
            .include("../external/cxx-rs/include")
            .include("../../out/generated")
            .file("bootimg.cpp")
            .file("boot-rs.cpp")
            .warnings(false)
            .compile("boot_cxx");
    }
}
