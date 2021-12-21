============
Using libc++
============

.. contents::
  :local:

Getting Started
===============

If you already have libc++ installed you can use it with clang.

.. code-block:: bash

    $ clang++ -stdlib=libc++ test.cpp
    $ clang++ -std=c++11 -stdlib=libc++ test.cpp

On OS X and FreeBSD libc++ is the default standard library
and the ``-stdlib=libc++`` is not required.

.. _alternate libcxx:

If you want to select an alternate installation of libc++ you
can use the following options.

.. code-block:: bash

  $ clang++ -std=c++11 -stdlib=libc++ -nostdinc++ \
            -I<libcxx-install-prefix>/include/c++/v1 \
            -L<libcxx-install-prefix>/lib \
            -Wl,-rpath,<libcxx-install-prefix>/lib \
            test.cpp

The option ``-Wl,-rpath,<libcxx-install-prefix>/lib`` adds a runtime library
search path. Meaning that the systems dynamic linker will look for libc++ in
``<libcxx-install-prefix>/lib`` whenever the program is run. Alternatively the
environment variable ``LD_LIBRARY_PATH`` (``DYLD_LIBRARY_PATH`` on OS X) can
be used to change the dynamic linkers search paths after a program is compiled.

An example of using ``LD_LIBRARY_PATH``:

.. code-block:: bash

  $ clang++ -stdlib=libc++ -nostdinc++ \
            -I<libcxx-install-prefix>/include/c++/v1
            -L<libcxx-install-prefix>/lib \
            test.cpp -o
  $ ./a.out # Searches for libc++ in the systems library paths.
  $ export LD_LIBRARY_PATH=<libcxx-install-prefix>/lib
  $ ./a.out # Searches for libc++ along LD_LIBRARY_PATH


Using ``<filesystem>`` and libc++fs
====================================

Libc++ provides the implementation of the filesystem library in a separate
library. Users of ``<filesystem>`` and ``<experimental/filesystem>`` are
required to link ``-lc++fs``.

.. note::
  Prior to libc++ 7.0, users of ``<experimental/filesystem>`` were required
  to link libc++experimental.

.. warning::
  The Filesystem library is still experimental in nature. As such normal
  guarantees about ABI stability and backwards compatibility do not yet apply
  to it. In the future, this restriction will be removed.


Using libc++experimental and ``<experimental/...>``
=====================================================

Libc++ provides implementations of experimental technical specifications
in a separate library, ``libc++experimental.a``. Users of ``<experimental/...>``
headers may be required to link ``-lc++experimental``.

.. code-block:: bash

  $ clang++ -std=c++14 -stdlib=libc++ test.cpp -lc++experimental

Libc++experimental.a may not always be available, even when libc++ is already
installed. For information on building libc++experimental from source see
:ref:`Building Libc++ <build instructions>` and
:ref:`libc++experimental CMake Options <libc++experimental options>`.

Note that as of libc++ 7.0 using the ``<experimental/filesystem>`` requires linking
libc++fs instead of libc++experimental.

Also see the `Experimental Library Implementation Status <http://libcxx.llvm.org/ts1z_status.html>`__
page.

.. warning::
  Experimental libraries are Experimental.
    * The contents of the ``<experimental/...>`` headers and ``libc++experimental.a``
      library will not remain compatible between versions.
    * No guarantees of API or ABI stability are provided.

Using libc++ on Linux
=====================

On Linux libc++ can typically be used with only '-stdlib=libc++'. However
some libc++ installations require the user manually link libc++abi themselves.
If you are running into linker errors when using libc++ try adding '-lc++abi'
to the link line.  For example:

.. code-block:: bash

  $ clang++ -stdlib=libc++ test.cpp -lc++ -lc++abi -lm -lc -lgcc_s -lgcc

Alternately, you could just add libc++abi to your libraries list, which in
most situations will give the same result:

.. code-block:: bash

  $ clang++ -stdlib=libc++ test.cpp -lc++abi


Using libc++ with GCC
---------------------

GCC does not provide a way to switch from libstdc++ to libc++. You must manually
configure the compile and link commands.

In particular you must tell GCC to remove the libstdc++ include directories
using ``-nostdinc++`` and to not link libstdc++.so using ``-nodefaultlibs``.

Note that ``-nodefaultlibs`` removes all of the standard system libraries and
not just libstdc++ so they must be manually linked. For example:

.. code-block:: bash

  $ g++ -nostdinc++ -I<libcxx-install-prefix>/include/c++/v1 \
         test.cpp -nodefaultlibs -lc++ -lc++abi -lm -lc -lgcc_s -lgcc


GDB Pretty printers for libc++
------------------------------

GDB does not support pretty-printing of libc++ symbols by default. Unfortunately
libc++ does not provide pretty-printers itself. However there are 3rd
party implementations available and although they are not officially
supported by libc++ they may be useful to users.

Known 3rd Party Implementations Include:

* `Koutheir's libc++ pretty-printers <https://github.com/koutheir/libcxx-pretty-printers>`_.


Libc++ Configuration Macros
===========================

Libc++ provides a number of configuration macros which can be used to enable
or disable extended libc++ behavior, including enabling "debug mode" or
thread safety annotations.

**_LIBCPP_DEBUG**:
  See :ref:`using-debug-mode` for more information.

**_LIBCPP_ENABLE_THREAD_SAFETY_ANNOTATIONS**:
  This macro is used to enable -Wthread-safety annotations on libc++'s
  ``std::mutex`` and ``std::lock_guard``. By default these annotations are
  disabled and must be manually enabled by the user.

**_LIBCPP_DISABLE_VISIBILITY_ANNOTATIONS**:
  This macro is used to disable all visibility annotations inside libc++.
  Defining this macro and then building libc++ with hidden visibility gives a
  build of libc++ which does not export any symbols, which can be useful when
  building statically for inclusion into another library.

**_LIBCPP_DISABLE_EXTERN_TEMPLATE**:
  This macro is used to disable extern template declarations in the libc++
  headers. The intended use case is for clients who wish to use the libc++
  headers without taking a dependency on the libc++ library itself.

**_LIBCPP_ENABLE_TUPLE_IMPLICIT_REDUCED_ARITY_EXTENSION**:
  This macro is used to re-enable an extension in `std::tuple` which allowed
  it to be implicitly constructed from fewer initializers than contained
  elements. Elements without an initializer are default constructed. For example:

  .. code-block:: cpp

    std::tuple<std::string, int, std::error_code> foo() {
      return {"hello world", 42}; // default constructs error_code
    }


  Since libc++ 4.0 this extension has been disabled by default. This macro
  may be defined to re-enable it in order to support existing code that depends
  on the extension. New use of this extension should be discouraged.
  See `PR 27374 <http://llvm.org/PR27374>`_ for more information.

  Note: The "reduced-arity-initialization" extension is still offered but only
  for explicit conversions. Example:

  .. code-block:: cpp

    auto foo() {
      using Tup = std::tuple<std::string, int, std::error_code>;
      return Tup{"hello world", 42}; // explicit constructor called. OK.
    }

**_LIBCPP_DISABLE_ADDITIONAL_DIAGNOSTICS**:
  This macro disables the additional diagnostics generated by libc++ using the
  `diagnose_if` attribute. These additional diagnostics include checks for:

    * Giving `set`, `map`, `multiset`, `multimap` and their `unordered_`
      counterparts a comparator which is not const callable.
    * Giving an unordered associative container a hasher that is not const
      callable.

**_LIBCPP_NO_VCRUNTIME**:
  Microsoft's C and C++ headers are fairly entangled, and some of their C++
  headers are fairly hard to avoid. In particular, `vcruntime_new.h` gets pulled
  in from a lot of other headers and provides definitions which clash with
  libc++ headers, such as `nothrow_t` (note that `nothrow_t` is a struct, so
  there's no way for libc++ to provide a compatible definition, since you can't
  have multiple definitions).

  By default, libc++ solves this problem by deferring to Microsoft's vcruntime
  headers where needed. However, it may be undesirable to depend on vcruntime
  headers, since they may not always be available in cross-compilation setups,
  or they may clash with other headers. The `_LIBCPP_NO_VCRUNTIME` macro
  prevents libc++ from depending on vcruntime headers. Consequently, it also
  prevents libc++ headers from being interoperable with vcruntime headers (from
  the aforementioned clashes), so users of this macro are promising to not
  attempt to combine libc++ headers with the problematic vcruntime headers. This
  macro also currently prevents certain `operator new`/`operator delete`
  replacement scenarios from working, e.g. replacing `operator new` and
  expecting a non-replaced `operator new[]` to call the replaced `operator new`.

**_LIBCPP_ENABLE_NODISCARD**:
  Allow the library to add ``[[nodiscard]]`` attributes to entities not specified
  as ``[[nodiscard]]`` by the current language dialect. This includes
  backporting applications of ``[[nodiscard]]`` from newer dialects and
  additional extended applications at the discretion of the library. All
  additional applications of ``[[nodiscard]]`` are disabled by default.
  See :ref:`Extended Applications of [[nodiscard]] <nodiscard extension>` for
  more information.

**_LIBCPP_DISABLE_NODISCARD_EXT**:
  This macro prevents the library from applying ``[[nodiscard]]`` to entities
  purely as an extension. See :ref:`Extended Applications of [[nodiscard]] <nodiscard extension>`
  for more information.

**_LIBCPP_ENABLE_DEPRECATION_WARNINGS**:
  This macro enables warnings when using deprecated components. For example,
  when compiling in C++11 mode, using `std::auto_ptr` with the macro defined
  will trigger a warning saying that `std::auto_ptr` is deprecated. By default,
  this macro is not defined.

C++17 Specific Configuration Macros
-----------------------------------
**_LIBCPP_ENABLE_CXX17_REMOVED_FEATURES**:
  This macro is used to re-enable all the features removed in C++17. The effect
  is equivalent to manually defining each macro listed below.

**_LIBCPP_ENABLE_CXX17_REMOVED_UNEXPECTED_FUNCTIONS**:
  This macro is used to re-enable the `set_unexpected`, `get_unexpected`, and
  `unexpected` functions, which were removed in C++17.

**_LIBCPP_ENABLE_CXX17_REMOVED_AUTO_PTR**:
  This macro is used to re-enable `std::auto_ptr` in C++17.

C++2a Specific Configuration Macros:
------------------------------------
**_LIBCPP_DISABLE_NODISCARD_AFTER_CXX17**:
  This macro can be used to disable diagnostics emitted from functions marked
  ``[[nodiscard]]`` in dialects after C++17.  See :ref:`Extended Applications of [[nodiscard]] <nodiscard extension>`
  for more information.


Libc++ Extensions
=================

This section documents various extensions provided by libc++, how they're
provided, and any information regarding how to use them.

.. _nodiscard extension:

Extended applications of ``[[nodiscard]]``
------------------------------------------

The ``[[nodiscard]]`` attribute is intended to help users find bugs where
function return values are ignored when they shouldn't be. After C++17 the
C++ standard has started to declared such library functions as ``[[nodiscard]]``.
However, this application is limited and applies only to dialects after C++17.
Users who want help diagnosing misuses of STL functions may desire a more
liberal application of ``[[nodiscard]]``.

For this reason libc++ provides an extension that does just that! The
extension must be enabled by defining ``_LIBCPP_ENABLE_NODISCARD``. The extended
applications of ``[[nodiscard]]`` takes two forms:

1. Backporting ``[[nodiscard]]`` to entities declared as such by the
   standard in newer dialects, but not in the present one.

2. Extended applications of ``[[nodiscard]]``, at the libraries discretion,
   applied to entities never declared as such by the standard.

Users may also opt-out of additional applications ``[[nodiscard]]`` using
additional macros.

Applications of the first form, which backport ``[[nodiscard]]`` from a newer
dialect may be disabled using macros specific to the dialect it was added. For
example ``_LIBCPP_DISABLE_NODISCARD_AFTER_CXX17``.

Applications of the second form, which are pure extensions, may be disabled
by defining ``_LIBCPP_DISABLE_NODISCARD_EXT``.


Entities declared with ``_LIBCPP_NODISCARD_EXT``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This section lists all extended applications of ``[[nodiscard]]`` to entities
which no dialect declares as such (See the second form described above).

* ``get_temporary_buffer``
