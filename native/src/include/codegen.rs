// This file hosts shared build script logic

use std::collections::BTreeMap;
use std::fmt::{Display, Write as _};
use std::fs::File;
use std::io::Write;
use std::path::Path;
use std::{fs, io, process};

use cxx_gen::{Include, IncludeKind, Opt};

mod bridge;
mod cxx;

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

fn parse_import(line: &str) -> Option<(&str, bool)> {
    let path = line.strip_prefix("#include \"")?.strip_suffix('"')?;
    if let Some(module) = path.strip_prefix(bridge::EXPORT_IMPORT) {
        Some((module, true))
    } else {
        path.strip_prefix(bridge::IMPORT)
            .map(|module| (module, false))
    }
}

pub fn gen_cxx_binding(name: &str) {
    println!("cargo:rerun-if-changed=lib.rs");
    let mut opt = Opt::default();
    opt.cxx_impl_annotations = Some("[[gnu::always_inline]]".to_string());
    opt.include.push(Include {
        path: "rust/cxx.h".to_string(),
        kind: IncludeKind::Bracketed,
    });
    let source = fs::read_to_string("lib.rs").ok_or_exit();
    let source = bridge::source(&source).ok_or_exit();
    let code = cxx_gen::generate_header_and_cc(source, &opt).ok_or_exit();
    let code = cxx::split(code).ok_or_exit();
    let header = code.declarations;
    let implementation = code.rust_api.definitions;
    let helpers = code.rust_api.helpers;
    let rust_preamble = code.rust_api.preamble;
    let wrappers = code.cxx_wrappers.definitions;
    let wrapper_helpers = code.cxx_wrappers.helpers;
    let wrapper_preamble = code.cxx_wrappers.preamble;
    let mut includes = Vec::new();
    let mut imports = BTreeMap::new();
    let mut declarations = String::new();
    // Preserve header order and keep module imports separate from textual includes.
    for line in header.lines() {
        if let Some((module, export)) = parse_import(line) {
            imports
                .entry(module.to_string())
                .and_modify(|value| *value |= export)
                .or_insert(export);
        } else if line.starts_with("#include ") {
            includes.push(line.to_string());
        } else if line != "#pragma once" {
            writeln!(declarations, "{line}").ok_or_exit();
        }
    }
    // The Rust API partition cannot import its own business interface back.
    // Forward-declare opaque C++ types that are only passed by reference.
    let (module, forward) = match name {
        "base-rs" => ("base", ""),
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
    let interface_imports = imports
        .iter()
        .map(|(dependency, export)| {
            let export = if *export { "export " } else { "" };
            format!("{export}import {dependency};\n")
        })
        .collect::<String>();
    if name == "init-rs" {
        includes.push("#include <vector>".to_string());
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
    for line in rust_preamble.lines() {
        if line.starts_with("#include ")
            && parse_import(line).is_none()
            && !includes.iter().any(|include| include == line)
        {
            includes.push(line.to_string());
        }
    }
    // CXX's anonymous-namespace impl<T> friend belongs to the header's TU.
    // Importing its public types from a BMI can select a different friend.
    // These views are trivially copyable and have the exact Fat representation;
    // bit_cast preserves the generator's unchecked representation copy without
    // accessing private members (and checks both properties at compile time).
    if !includes.iter().any(|include| include == "#include <bit>") {
        includes.push("#include <bit>".to_string());
    }
    let adapt_views = |definitions: String| {
        definitions
        .replace("    Slice<T> slice = typename Slice<T>::uninit{};\n    slice.repr = repr;\n    return slice;", "    return ::std::bit_cast<Slice<T>>(repr);")
        .replace("    Str str = Str::uninit{};\n    str.repr = repr;\n    return str;", "    return ::std::bit_cast<Str>(repr);")
        .replace("    return slice.repr;", "    return ::std::bit_cast<repr::Fat>(slice);")
        .replace("    return str.repr;", "    return ::std::bit_cast<repr::Fat>(str);")
    };
    let helpers = adapt_views(helpers);
    // Keep CXX's runtime helpers attached to the global rust/cxx.h types.
    // Application declarations and definitions belong to the named module.
    let definitions = implementation
        .replace(
            "namespace rust {\ninline namespace cxxbridge1 {\n",
            "extern \"C++\" {\nnamespace rust {\ninline namespace cxxbridge1 {\n",
        )
        .replace(
            "} // namespace cxxbridge1\n} // namespace rust",
            "} // namespace cxxbridge1\n} // namespace rust\n} // extern C++",
        );
    let prelude = includes.join("\n");
    // Declarations and their Rust-facing definitions share one translation unit.
    let rust_api = format!(
        "module;\n{prelude}\n\nexport module {module}:rs;\nimport std;\n{interface_imports}\nextern \"C++\" {{\n{helpers}\n}}\n\nexport {{\n{forward}{declarations}\n}}\n\n{definitions}"
    );
    for line in wrapper_preamble.lines() {
        if line.starts_with("#include ")
            && parse_import(line).is_none()
            && !includes.iter().any(|include| include == line)
        {
            includes.push(line.to_string());
        }
    }
    let helpers = adapt_views(wrapper_helpers);
    let prelude = includes.join("\n");
    let imports = imports
        .into_keys()
        .filter(|m| !m.starts_with(':'))
        .map(|m| format!("import {m};\n"))
        .collect::<String>();
    // Define global runtime helpers before importing their definitions from BMIs.
    let wrappers =
        format!("{prelude}\n\n{helpers}\nimport {module};\nimport std;\n{imports}\n{wrappers}");
    let component = name
        .strip_suffix("-rs")
        .expect("bridge name must end in -rs");
    write_if_diff(format!("{name}.cpp"), rust_api.as_bytes()).ok_or_exit();
    write_if_diff(format!("{component}-cxx.cpp"), wrappers.as_bytes()).ok_or_exit();
    for ext in ["hpp", "ixx"] {
        fs::remove_file(format!("{name}.{ext}"))
            .or_else(|e| {
                if e.kind() == io::ErrorKind::NotFound {
                    Ok(())
                } else {
                    Err(e)
                }
            })
            .ok_or_exit();
    }
}
