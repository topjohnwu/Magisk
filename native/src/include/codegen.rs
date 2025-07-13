// This file hosts shared build script logic

use std::fmt::Display;
use std::fs::File;
use std::io::Write;
use std::path::Path;
use std::{fs, io, process};

use cxx_gen::Opt;

trait ResultExt<T> {
    fn ok_or_exit(self) -> T;
}

impl<T, E: Display> ResultExt<T> for Result<T, E> {
    fn ok_or_exit(self) -> T {
        match self {
            Ok(r) => r,
            Err(e) => {
                eprintln!("error occurred: {e}");
                process::exit(1);
            }
        }
    }
}

fn write_if_diff<P: AsRef<Path>>(path: P, bytes: &[u8]) -> io::Result<()> {
    let path = path.as_ref();
    if let Ok(orig) = fs::read(path) {
        // Do not modify the file if content is the same to make incremental build more optimal
        if orig.as_slice() == bytes {
            return Ok(());
        }
    }
    let mut f = File::create(path)?;
    f.write_all(bytes)
}

pub fn gen_cxx_binding(name: &str) {
    println!("cargo:rerun-if-changed=lib.rs");
    let opt = Opt::default();
    let code = cxx_gen::generate_header_and_cc_with_path("lib.rs", &opt);
    write_if_diff(format!("{name}.cpp"), code.implementation.as_slice()).ok_or_exit();
    write_if_diff(format!("{name}.hpp"), code.header.as_slice()).ok_or_exit();
}
