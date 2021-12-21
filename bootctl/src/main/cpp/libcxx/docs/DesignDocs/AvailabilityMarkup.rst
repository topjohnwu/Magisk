===================
Availability Markup
===================

.. contents::
   :local:

Overview
========

Libc++ is used as a system library on macOS and iOS (amongst others). In order
for users to be able to compile a binary that is intended to be deployed to an
older version of the platform, clang provides the
`availability attribute <https://clang.llvm.org/docs/AttributeReference.html#availability>`_
that can be placed on declarations to describe the lifecycle of a symbol in the
library.

Design
======

When a new feature is introduced that requires dylib support, a macro should be
created in include/__config to mark this feature as unavailable for all the
systems. For example::

    // Define availability macros.
    #if defined(_LIBCPP_USE_AVAILABILITY_APPLE)
    # define _LIBCPP_AVAILABILITY_BAD_OPTIONAL_ACCESS __attribute__((unavailable))
    #else  if defined(_LIBCPP_USE_AVAILABILITY_SOME_OTHER_VENDOR)
    # define _LIBCPP_AVAILABILITY_BAD_OPTIONAL_ACCESS __attribute__((unavailable))
    #else
    # define _LIBCPP_AVAILABILITY_BAD_OPTIONAL_ACCESS
    #endif

When the library is updated by the platform vendor, the markup can be updated.
For example::

    #define _LIBCPP_AVAILABILITY_SHARED_MUTEX                                  \
      __attribute__((availability(macosx,strict,introduced=10.12)))            \
      __attribute__((availability(ios,strict,introduced=10.0)))                \
      __attribute__((availability(tvos,strict,introduced=10.0)))               \
      __attribute__((availability(watchos,strict,introduced=3.0)))

In the source code, the macro can be added on a class if the full class requires
type info from the library for example::

    _LIBCPP_BEGIN_NAMESPACE_EXPERIMENTAL
    class _LIBCPP_EXCEPTION_ABI _LIBCPP_AVAILABILITY_BAD_OPTIONAL_ACCESS bad_optional_access
      : public std::logic_error {

or on a particular symbol:

    _LIBCPP_OVERRIDABLE_FUNC_VIS _LIBCPP_AVAILABILITY_SIZED_NEW_DELETE void  operator delete(void* __p, std::size_t __sz) _NOEXCEPT;


Testing
=======

Some parameters can be passed to lit to run the test-suite and exercise the
availability.

* The `platform` parameter controls the deployment target. For example lit can
  be invoked with `--param=platform=macosx10.8`. Default is the current host.
* The `use_system_cxx_lib` parameter indicates to use another library than the
  just built one. Invoking lit with `--param=use_system_cxx_lib=true` will run
  the test-suite against the host system library. Alternatively a path to the
  directory containing a specific prebuilt libc++ can be used, for example:
  `--param=use_system_cxx_lib=/path/to/macOS/10.8/`.

Tests can be marked as XFAIL based on multiple features made available by lit:


* if `--param=platform=macosx10.8` is passed, the following features will be available:

  - availability
  - availability=x86_64
  - availability=macosx
  - availability=x86_64-macosx
  - availability=x86_64-apple-macosx10.8
  - availability=macosx10.8

  This feature is used to XFAIL a test that *is* using a class or a method marked
  as unavailable *and* that is expected to *fail* if deployed on an older system.

* if `use_system_cxx_lib` and `--param=platform=macosx10.8` are passed to lit,
  the following features will also be available:

  - with_system_cxx_lib
  - with_system_cxx_lib=x86_64
  - with_system_cxx_lib=macosx
  - with_system_cxx_lib=x86_64-macosx
  - with_system_cxx_lib=x86_64-apple-macosx10.8
  - with_system_cxx_lib=macosx10.8

  This feature is used to XFAIL a test that is *not* using a class or a method
  marked as unavailable *but* that is expected to fail if deployed on an older
  system. For example, if the test exhibits a bug in the libc on a particular
  system version, or if the test uses a symbol that is not available on an
  older version of the dylib (but for which there is no availability markup,
  otherwise the XFAIL should use `availability` above).
