use crate::codegen::gen_cxx_binding;

#[path = "../include/codegen.rs"]
mod codegen;

fn main() {
    gen_cxx_binding("base-rs");

    let target_os = std::env::var("CARGO_CFG_TARGET_OS").unwrap_or_default();
    if target_os != "android" {
        let mut build = cc::Build::new();
        build
            .cpp(true)
            .std("c++23")
            .include("include")
            .include("../include")
            .include("../../out/generated")
            .include("../external/cxx-rs/include")
            .file("base.cpp")
            .file("base-rs.cpp")
            .file("../external/cxx-rs/src/cxx.cc")
            .warnings(false)
            .compile("base_cxx");
    }
}
