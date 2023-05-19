use crate::gen::gen_cxx_binding;

mod gen;

fn main() {
    gen_cxx_binding("base-rs");
}
