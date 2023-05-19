use crate::gen::gen_cxx_binding;

#[path = "../base/gen.rs"]
mod gen;

fn main() {
    gen_cxx_binding("init-rs");
}
