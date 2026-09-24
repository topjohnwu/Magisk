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
    // Quoted include! entries name modules; runtime headers use angle brackets.
    for line in header.lines() {
        if let Some(module) = line
            .strip_prefix("#include \"")
            .and_then(|s| s.strip_suffix('"'))
        {
            imports.insert(module.to_string());
        } else if line.starts_with("#include ") {
            includes.insert(line.to_string());
        } else if line != "#pragma once" {
            writeln!(declarations, "{line}").ok_or_exit();
        }
    }
    let (module, forward) = match name {
        "base-rs" => (
            "base",
            "struct Utf8CStr;\nstruct FnBoolStrStr;\nstruct FnBoolStr;\n",
        ),
        "core-rs" => ("core", ""),
        "policy-rs" => ("policy", "class sepol_impl;\n"),
        "boot-rs" => ("boot", "struct boot_img;\n"),
        "init-rs" => (
            "init",
            "using kv_pairs = std::vector<std::pair<std::string, std::string>>;\n",
        ),
        _ => panic!("unknown CXX bridge {name}"),
    };
    imports.remove(module);
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
        // These specialize the global rust/cxx.h templates, not application
        // entities. Keep their declarations attached to that global template.
        writeln!(
            declarations,
            "extern \"C++\" {{ namespace rust {{ inline namespace cxxbridge1 {{\n{specializations}}}\n}}\n}}"
        )
        .ok_or_exit();
    }
    let header_includes = includes.iter().cloned().collect::<Vec<_>>().join("\n");

    let mut definitions = String::new();
    for line in implementation.lines() {
        if line.starts_with("#include \"") {
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
    // CXX's runtime helpers define members of the global rust/cxx.h types.
    // Include them in the bridge implementation's global module fragment;
    // application declarations and definitions belong to the named module.
    let Some(first_declaration) = header.lines().find(|line| {
        line.starts_with("enum class ") || line.starts_with("struct ") || line.starts_with("using ")
    }) else {
        panic!("missing CXX application declarations in {name}");
    };
    let Some(start) = definitions.find(first_declaration) else {
        panic!("missing CXX implementation declarations in {name}");
    };
    let (helpers, definitions) = definitions.split_at(start);
    let definitions = definitions
        .replace(
            "namespace rust {\ninline namespace cxxbridge1 {\n",
            "extern \"C++\" {\nnamespace rust {\ninline namespace cxxbridge1 {\n",
        )
        .replace(
            "} // namespace cxxbridge1\n} // namespace rust",
            "} // namespace cxxbridge1\n} // namespace rust\n} // extern C++",
        );
    // The bridge imports these types from its interface or partitions.
    // Reuse CXX's guards instead of defining the shared types a second time.
    let mut guards = String::new();
    for line in header.lines() {
        if let Some(guard) = line.strip_prefix("#ifndef CXXBRIDGE1_")
            && (guard.starts_with("STRUCT_") || guard.starts_with("ENUM_"))
        {
            writeln!(guards, "#define CXXBRIDGE1_{guard}").ok_or_exit();
        }
    }
    let prelude = includes.iter().cloned().collect::<Vec<_>>().join("\n");
    let interface = format!(
        "#pragma once\n#ifdef MAGISK_CXX_BRIDGE_IMPL\n{prelude}\n{helpers}{guards}\n#else\n{header_includes}\n\n{forward}{declarations}\n#endif\n"
    );
    // The core bridge is the primary interface, re-exporting its partitions.
    // Other bridges are implementation units of their handwritten interfaces.
    let export = if name == "core-rs" { "export " } else { "" };
    let imports = imports
        .into_iter()
        .map(|m| format!("{export}import {m};\n"))
        .collect::<String>();
    let implementation = format!(
        "module;\n#define MAGISK_CXX_BRIDGE_IMPL\n#include \"{name}.hpp\"\n#undef MAGISK_CXX_BRIDGE_IMPL\n\n{export}module {module};\nimport std;\n{imports}\n{definitions}"
    );
    write_if_diff(format!("{name}.hpp"), interface.as_bytes()).ok_or_exit();
    write_if_diff(format!("{name}.cpp"), implementation.as_bytes()).ok_or_exit();
}
