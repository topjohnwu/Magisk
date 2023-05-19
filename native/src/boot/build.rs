use protobuf_codegen::Customize;

use crate::gen::gen_cxx_binding;

#[path = "../base/gen.rs"]
mod gen;

fn main() {
    println!("cargo:rerun-if-changed=update_metadata.proto");
    protobuf_codegen::Codegen::new()
        .pure()
        .include(".")
        .input("update_metadata.proto")
        .customize(Customize::default().gen_mod_rs(false))
        .out_dir(".")
        .run_from_script();

    gen_cxx_binding("boot-rs");
}
