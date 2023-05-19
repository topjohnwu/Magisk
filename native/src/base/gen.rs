// This file hosts shared build script logic

use std::fmt::Display;
use std::fs::File;
use std::io::Write;
use std::process;

use cxx_gen::Opt;

trait ResultExt<T> {
    fn ok_or_exit(self) -> T;
}

impl<T, E: Display> ResultExt<T> for Result<T, E> {
    fn ok_or_exit(self) -> T {
        match self {
            Ok(r) => r,
            Err(e) => {
                eprintln!("error occurred: {}", e);
                process::exit(1);
            }
        }
    }
}

pub fn gen_cxx_binding(name: &str) {
    println!("cargo:rerun-if-changed=lib.rs");
    let opt = Opt::default();
    let gen = cxx_gen::generate_header_and_cc_with_path("lib.rs", &opt);
    {
        let mut cpp = File::create(format!("{}.cpp", name)).unwrap();
        cpp.write_all(gen.implementation.as_slice()).ok_or_exit();
    }
    {
        let mut hpp = File::create(format!("{}.hpp", name)).unwrap();
        hpp.write_all(gen.header.as_slice()).ok_or_exit();
    }
}
