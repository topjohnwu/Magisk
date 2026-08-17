# AGENTS.md (native subproject)

Guidelines for AI models operating inside the `native/` subproject. Always include and follow the top-level [`AGENTS.md`](../AGENTS.md).

## 1. Environment & Build Requirements

- **General Guidelines:** Always follow the top-level [`AGENTS.md`](../AGENTS.md) for general repository rules, environment execution setup, and commit control policies.
- **Working Directory:** Execute commands from repo root via `./build.py`.
- **Pre-build Requirement:** ALWAYS run `./build.py native` at least once before editing native sources to generate FFI bindings, headers, and flags (`flags.h`, `flags.rs`, `*-rs.hpp`, `*-rs.cpp`, protobuf generated modules).

## 2. Component Architecture

Native C, C++, and Rust source modules under `native/src/`:
- **`base/`**: System wrappers, logging, custom string abstractions (`Utf8CStr`), mount helpers, and common utilities (C++/Rust).
- **`boot/`**: Boot image parsing, unpacking, repacking, and ramdisk patching logic (`magiskboot`) (Rust/C++).
- **`core/`**: Magisk daemon (`magiskd`), Zygisk engine, `su` implementation, applets, and system properties (`resetprop`) (C++/Rust).
- **`init/`**: Early boot `magiskinit`, ramdisk patching, SELinux pre-init (C/C++/Rust).
- **`sepolicy/`**: SELinux policy engine (`libpolicy`) and `magiskpolicy` CLI (C++/Rust).
- **`external/`**: Embedded dependencies (`cxx-rs`, `selinux`, `crt0`, `system_properties`, `lsplt`, `lz4-sys`, `xz-embedded`).

### Key Native Binary Output Targets
- `magisk`: Core daemon, Zygisk, `su`, and applets executable.
- `magiskinit`: Early init replacement executable (static).
- `magiskboot`: Boot image patcher executable (static).
- `magiskpolicy`: SELinux policy tool executable.
- `resetprop`: System property reader/writer executable.

## 3. Build System Orchestration

The build process follows a two-stage hybrid pipeline orchestrated by `build.py`:

```
1. dump_flags_native()    -->  Outputs flags.h & flags.rs to native/out/generated/
2. build_rust_src()       -->  Cargo build outputs lib<tgt>.a for each target ABI
                               Cargo build.rs runs cxx_gen to produce *-rs.hpp / *-rs.cpp
                               Cargo build.rs runs pb-rs to generate Protobuf bindings
                               Static libraries moved to native/out/<arch>/lib<tgt>-rs.a
3. build_cpp_src()        -->  ndk-build runs using Android.mk & Application.mk
                               Android-rs.mk imports lib<tgt>-rs.a as PREBUILT_STATIC_LIBRARY
                               Compiles C/C++ sources + *-rs.cpp bridge files + cxx.cc
4. clean_elf()            -->  tools/elf-cleaner strips incompatible ELF dynamic tags
```

## 4. FFI Architecture & Mechanics

- **Bridge Engine:** C++/Rust FFI uses `cxx` (`cxx-rs`) via `#[cxx::bridge]` modules declared in crate `lib.rs` files.
- **Header & Source Generation:**
  - `codegen.rs` (`gen_cxx_binding()`) invokes `cxx_gen` in crate `build.rs` scripts.
  - Automatically generates C++ bridge headers (`*-rs.hpp`) and source wrappers (`*-rs.cpp`) directly in each crate directory.
  - Generates bindings for `base-rs`, `core-rs`, `init-rs`, `boot-rs`, and `policy-rs`.
- **Linking:** Generated `*-rs.cpp` bridge code and `cxx.cc` are compiled directly by `ndk-build` alongside native C++ source files, linking against the compiled Rust static library (`lib<tgt>-rs.a`).

## 5. Build Targets & Commands (from Root)

- **Build All Native Binaries:** `./build.py native`
- **Build Specific Target(s):** `./build.py native [magisk|magiskinit|magiskboot|magiskpolicy|resetprop]`
- **Rust Clippy Lint:** `./build.py clippy`
- **Cargo Commands:** `./build.py cargo check`, `./build.py cargo test`
- **Generate IDE Database:** `./build.py gen`
- **Clean Native Artifacts:** `./build.py clean native`

## 6. Rust & C++ Conventions

- **Rust Edition & Profile:** Rust Edition 2024. Profile configured with `panic = "immediate-abort"` across dev and release profiles.
- **C++ Standard:** C++20 with `libc++` static linking.
- **Clippy Rules:** `unwrap_used = "deny"` in workspace configuration. **Do NOT use `.unwrap()` in Rust code.** Use `?`, `unwrap_or`, `unwrap_or_else`, or pattern matching.
- **Incremental Build Protection:** File writers (`write_if_diff`) skip rewriting identical generated files to preserve compilation timestamps and avoid unnecessary rebuilds.

## 7. Magisk Rust Patterns & Non-Standard Idioms

AI models modifying or writing Rust code in `native/src/` MUST follow these Magisk-specific idioms:

1. **Custom String Handling (`Utf8CStr` & `cstr!`):**
   - Standard C/C++ APIs require null-terminated UTF-8 strings. Do NOT create intermediate `CString` allocations.
   - Use `&Utf8CStr` for string references, `Utf8CString` for heap buffers, and `Utf8CStrBufArr<N>` for stack buffers (`base/cstr.rs`).
   - Use `cstr!("literal")` for compile-time null-terminated static strings.
   - Use `StringExt::nul_terminate` for in-place null byte termination using reserved string capacity.

2. **Instant Logging Error Model (`LoggedError`):**
   - Application logic does NOT use standard error enums or `anyhow`.
   - Errors are logged immediately at the failure point via `log_err!`, `.log()`, or `.log_with_msg()` and converted into zero-sized `LoggedError` / `LoggedResult<T>`.
   - In debug builds, `#[track_caller]` logs the file and line number of error sites automatically. Use `.silent()` to silence expected failures.

3. **Libc System Call Conversions (`LibcReturn`):**
   - When calling raw libc or C system APIs in Rust or `xwrap.rs`, use `LibcReturn::into_os_result` or `check_err` to automatically map negative integer returns or null pointers into `OsResult` or `LoggedResult`.

4. **Zero Allocation & Binary Size Rules:**
   - Prefer custom `argh` derivation (`base/argh.rs`) for command-line parsing instead of adding standard argument dependencies.
   - Avoid `serde` overhead; use procedural `Encodable`/`Decodable` binary traits over UNIX domain sockets for IPC.

5. **Nightly Synchronization & IPC:**
   - Magisk uses nightly features (`unix_socket_ancillary_data`, `nonpoison_mutex`).
   - Use non-poisoning mutexes/condvars for daemon threading and `send_fd`/`recv_fd` for raw file descriptor passing.
