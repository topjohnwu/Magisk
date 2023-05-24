use crate::gen::gen_cxx_binding;

#[path = "../include/gen.rs"]
mod gen;

fn main() {
    gen_cxx_binding("base-rs");
}
