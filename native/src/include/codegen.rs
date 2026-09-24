// This file hosts shared build script logic

use std::collections::BTreeSet;
use std::fmt::{Display, Write as _};
use std::fs::File;
use std::io::Write;
use std::path::Path;
use std::{fs, io, process};

use cxx_gen::{Include, IncludeKind, Opt};

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
    let mut opt = Opt::default();
    opt.cxx_impl_annotations = Some("[[gnu::always_inline]]".to_string());
    opt.include.push(Include {
        path: "rust/cxx.h".to_string(),
        kind: IncludeKind::Bracketed,
    });
    let code = cxx_gen::generate_header_and_cc_with_path("lib.rs", &opt);
    let header = String::from_utf8(code.header).ok_or_exit();
    let implementation = String::from_utf8(code.implementation).ok_or_exit();
    let mut includes = BTreeSet::new();
    let mut imports = BTreeSet::new();
    let mut declarations = String::new();
    for line in header.lines() {
        if let Some(module) = line
            .strip_prefix("#include \"magisk.")
            .and_then(|s| s.strip_suffix('"'))
        {
            imports.insert(format!("magisk.{module}"));
        } else if line.starts_with("#include ") {
            includes.insert(line.to_string());
        } else if line != "#pragma once" {
            writeln!(declarations, "{line}").ok_or_exit();
        }
    }
    let forward = match name {
        "base-rs" => "struct Utf8CStr;\nstruct FnBoolStrStr;\nstruct FnBoolStr;\n",
        "core-rs" => "struct Utf8CStr;\nstruct sqlite3;\nstruct DbValues;\nstruct DbStatement;\n",
        "policy-rs" => "struct Utf8CStr;\nclass sepol_impl;\n",
        "boot-rs" => "struct Utf8CStr;\nstruct boot_img;\n",
        "init-rs" => {
            "struct Utf8CStr;\nusing kv_pairs = std::vector<std::pair<std::string, std::string>>;\n"
        }
        _ => panic!("unknown CXX bridge {name}"),
    };
    if name == "init-rs" {
        includes.insert("#include <vector>".to_string());
    }
    // Interface bodies may instantiate these views before the bridge's
    // implementation is compiled. Publish the generated specializations first.
    let mut specializations = String::new();
    let mut explicit = false;
    for line in implementation.lines() {
        if explicit
            && (line.contains("Vec<") || (line.contains("Box<") && line.contains("::drop(")))
            && let Some(signature) = line.strip_suffix(" {")
        {
            writeln!(specializations, "template <>\n{signature};").ok_or_exit();
        }
        explicit = line == "template <>";
    }
    if !specializations.is_empty() {
        writeln!(
            declarations,
            "namespace rust {{ inline namespace cxxbridge1 {{\n{specializations}}}\n}}"
        )
        .ok_or_exit();
    }
    let prelude = includes.iter().cloned().collect::<Vec<_>>().join("\n");
    let interface = format!("#pragma once\n{prelude}\n\n{forward}{declarations}");

    let mut definitions = String::new();
    for line in implementation.lines() {
        if line.starts_with("#include \"magisk.") {
            continue;
        } else if line.starts_with("#include ") {
            includes.insert(line.to_string());
        } else {
            writeln!(definitions, "{line}").ok_or_exit();
        }
    }
    // CXX's anonymous-namespace impl<T> friend belongs to the header's TU.
    // Importing its public types from a BMI can select a different friend.
    // These views are trivially copyable and have the exact Fat representation;
    // bit_cast preserves the generator's unchecked representation copy without
    // accessing private members (and checks both properties at compile time).
    includes.insert("#include <bit>".to_string());
    let definitions = definitions
        .replace("    Slice<T> slice = typename Slice<T>::uninit{};\n    slice.repr = repr;\n    return slice;", "    return ::std::bit_cast<Slice<T>>(repr);")
        .replace("    Str str = Str::uninit{};\n    str.repr = repr;\n    return str;", "    return ::std::bit_cast<Str>(repr);")
        .replace("    return slice.repr;", "    return ::std::bit_cast<repr::Fat>(slice);")
        .replace("    return str.repr;", "    return ::std::bit_cast<repr::Fat>(str);");
    let prelude = includes.iter().cloned().collect::<Vec<_>>().join("\n");
    let imports = imports
        .into_iter()
        .map(|m| format!("import {m};\n"))
        .collect::<String>();
    let implementation = format!("{prelude}\n#include \"{name}.hpp\"\n\n{imports}\n{definitions}");
    write_if_diff(format!("{name}.hpp"), interface.as_bytes()).ok_or_exit();
    write_if_diff(format!("{name}.cpp"), implementation.as_bytes()).ok_or_exit();
}
