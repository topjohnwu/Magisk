# C++ named modules in ndk-build

This project-local extension adds dependency scanning and BMI generation to
ndk-build. It does not modify the installed NDK or use `build.py` to build modules.
It requires Clang with `clang-scan-deps` and C++20 or newer.
Verified on macOS with ONDK r30.1, across all four standard Android ABIs.

Load `init.mk` once from the project's top-level `Android.mk`, after setting
`LOCAL_PATH`. Declare module-providing sources separately from other sources,
keep the normal library dependencies, and replace the build include for each
participating target:

```makefile
LOCAL_PATH := $(call my-dir)
include $(LOCAL_PATH)/path/to/ndk-modules/init.mk

include $(CLEAR_VARS)
LOCAL_MODULE := example
LOCAL_MODULE_SRC_FILES := math.ixx math_detail.cxx
LOCAL_SRC_FILES := math.cxx consumer.cxx
include $(BUILD_SHARED_LIBRARY_MODULE)
```

`BUILD_STATIC_LIBRARY_MODULE` and `BUILD_EXECUTABLE_MODULE` are also available.
`LOCAL_MODULE_SRC_FILES` contains the primary module interface and importable
partitions (including internal partitions). These files generate BMIs and object
files. `LOCAL_SRC_FILES` contains ordinary sources, import consumers and module
implementation units (`module math;`); those generate only object files.

The scanner checks this classification. List each source in only one group;
module providers in the ordinary list and non-providers in the module list are
errors. `CLEAR_VARS` clears both lists between targets. Module providers are
compiled with `-x c++-module`, including when their extension is `.cxx` or `.ixx`.
Both groups are scanned for imports and scheduled by their actual dependencies,
without a barrier that waits for every module before compiling any consumer.
Provider objects retain the source extension (`math.ixx.o`), so a definition-only
`math.ixx` interface and a `math.cxx` implementation (`math.o`) can share a stem.
Other sources keep ndk-build's ordinary object names; colliding paths are rejected.
Clean when changing a source's extension while retaining its object path, to
discard ndk-build dependency files that still refer to the old filename.

Select the C++ standard using `APP_CPPFLAGS` or `LOCAL_CPPFLAGS` as usual. The
extension preserves these settings instead of silently overriding them.

Named modules and module partitions are discovered with Clang's P1689 scanner;
filenames do not need to match module names. Providers can be in the same target
or in participating targets reachable through the normal
`LOCAL_STATIC_LIBRARIES`, `LOCAL_WHOLE_STATIC_LIBRARIES`, and
`LOCAL_SHARED_LIBRARIES` dependencies. Keep compatible language settings across
providers and consumers. Header units are not supported. Magisk's libc++ supplies
an optional [`std` module subset](../../native/src/external/libcxx/modules/README.md),
using the same provider/dependency mechanism without changes to this extension.

The extension wraps ndk-build's `compile-cpp-source` function after ndk-build has
resolved exported flags. It generates per-source scan results, collates them into
an included Makefile, and lets Make rebuild/reload that file before compiling.
Clang emits the object and BMI together. Explicit object dependencies make fresh
parallel builds safe. Missing BMIs force their providers to rebuild. Compiler
caches are bypassed for participating C++ sources, since restoring only the object
would omit the BMI. Unregistered targets retain the original compilation rules.

Objects, BMIs, response files and scan results live under the normal target object
directory. The aggregate graph lives under `$(TARGET_OBJS)/cxx-modules`; `clean`
removes it. The extension uses internal ndk-build functions, so new NDK versions
need integration testing. PCH and assembly filters are rejected for participating
targets. Run clean and build as separate invocations.

Magisk's existing `.cpp` sources provide modules through
`LOCAL_MODULE_SRC_FILES`; their filenames do not change. Module names are
plain component names such as `base`, `core` and `init`. Core uses interface
partitions `core:utils`, `core:sqlite`, `core:scripting`, `core:su`, `core:deny`
and `core:zygisk`. Its primary interface re-exports them, so consumers access
core APIs through `import core;`. Partitions use `import :name;`, never
the primary interface, keeping the dependency graph acyclic. Handwritten
sources and generated bridges import `std` directly where needed; project
modules do not re-export it. Global module fragments include only the C,
platform and bridge headers needed by that source, plus C++ facilities absent
from the `std` subset. Headers also included by a generated bridge, such as
`<memory>`, are preloaded in the global fragment to keep their declarations in
the global module. Handwritten API headers are merged into their existing
implementation files, with functions exported at their definitions and class
methods defined in the class where possible. Rust ABI declarations, cross-file
implementation declarations and forward declarations required by mutually
dependent types remain.

Shared string constants use the `_cs` literal and `consteval` concatenation:

```cpp
inline constinit const auto &SECURE_DIR = "/data/adb"_cs;
inline constinit const auto &DATABIN = SECURE_DIR + "/magisk";
```

The references keep constant-initialized strings usable in constant expressions
and give concatenated constants static storage. `+` accepts ordinary string
literals on either side. Implicit `const char*` conversion supports C APIs;
use `.c_str()` for C varargs, which do not apply user-defined conversions.
Clang's format-string diagnostics are limited for formats passed through this
implicit conversion.
An unnamed concatenation lives until the end of its full expression. Bind it
to a static `constinit const auto &` before retaining its pointer. Lifetime
annotations diagnose pointers or views escaping a temporary.

The Rust CXX generator keeps its `*-rs.hpp` / `*-rs.cpp` output format.
Providers include the generated application declarations in their export
blocks, so types and functions belong to their named module. The generated
`core-rs.cpp` supplies core's primary interface and re-exports the partitions
listed in Rust's `include!` metadata (for example, `include!(":deny")`). It is
listed in `LOCAL_MODULE_SRC_FILES`; `core:utils` includes `core-rs.hpp` as before.
Other generated `.cpp` files are implementation units of their named modules,
implicitly importing their primary interface and importing additional modules
from the same metadata. Quoted include entries name modules or relative
partitions; runtime header includes use angle brackets. Rust calls the generated
`extern "C"` entry points; handwritten C++ APIs use named-module ownership.

The generated header has a separate implementation-only branch for CXX's
runtime helpers. The bridge includes that branch in its global module fragment
because those helpers implement the global `rust/cxx.h` types. Only generated
`rust::Vec` / `rust::Box` template-specialization declarations and definitions
retain `extern "C++"` to match those existing global templates in Clang 21.
The adapter also uses `bit_cast` for CXX's trivially copyable Str/Slice views to
avoid Clang's imported anonymous-friend mismatch. Revalidate the adapter when
updating CXX.

Non-exported implementation details use module linkage, so imported class
methods refer to the provider's state rather than copies of `static` variables.
`MagiskInit` and `BootConfig`, their handwritten methods and their generated
bridge now share the `init` module. The Zygisk SDK header, generated
CXX headers and generated build flags remain.

The checked-in JNI wrappers are generated as `core/zygisk/jni_hooks.cpp`, an
internal partition named `core:zygisk.jni`. It imports `:zygisk`; `hook.cpp`
imports this partition instead of including a header. `get_defs()` uses module
linkage to reach the hook context without introducing an import cycle. JNI
method tables and wrappers remain internal to `core`. Regenerate the source
from the repository root with
`scripts/env.py python3 native/src/core/zygisk/gen_jni_hooks.py`.

Rust libraries and their generated bindings must exist before invoking ndk-build,
as in the existing hybrid build. Module scanning, BMI generation and compilation
ordering are handled entirely by this extension inside ndk-build.
