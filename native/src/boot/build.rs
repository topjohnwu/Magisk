use protobuf_codegen::Customize;

fn main() {
    println!("cargo:rerun-if-changed=update_metadata.proto");
    protobuf_codegen::Codegen::new()
        .pure()
        .include(".")
        .input("update_metadata.proto")
        .customize(Customize::default().gen_mod_rs(false))
        .out_dir(".")
        .run_from_script();
}
