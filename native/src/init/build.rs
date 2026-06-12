use crate::codegen::gen_cxx_binding;

#[path = "../include/codegen.rs"]
mod codegen;

fn main() {
    gen_cxx_binding("init-rs");
}
