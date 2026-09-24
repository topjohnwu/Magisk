# C++ named modules in ndk-build

This project-local extension adds dependency scanning and BMI generation to
ndk-build. It does not modify the installed NDK or use `build.py` to build modules.
It requires Clang with `clang-scan-deps` and C++20 or newer.
Verified on macOS with ONDK r30.1, across all four standard Android ABIs.

Load `init.mk` once from the project's top-level `Android.mk`, after setting
`LOCAL_PATH`. Keep the ordinary source lists and library dependencies, and replace
the build include for each participating target:

```makefile
LOCAL_PATH := $(call my-dir)
include $(LOCAL_PATH)/path/to/ndk-modules/init.mk

include $(CLEAR_VARS)
LOCAL_MODULE := example
LOCAL_SRC_FILES := math.cppm implementation.cpp consumer.cpp
include $(BUILD_SHARED_LIBRARY_MODULE)
```

`BUILD_STATIC_LIBRARY_MODULE` and `BUILD_EXECUTABLE_MODULE` are also available.
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
