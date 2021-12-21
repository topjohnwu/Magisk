=====================
Threading Support API
=====================

.. contents::
   :local:

Overview
========

Libc++ supports using multiple different threading models and configurations
to implement the threading parts of libc++, including ``<thread>`` and ``<mutex>``.
These different models provide entirely different interfaces from each
other. To address this libc++ wraps the underlying threading API in a new and
consistent API, which it uses internally to implement threading primitives.

The ``<__threading_support>`` header is where libc++ defines its internal
threading interface. It contains forward declarations of the internal threading
interface as well as definitions for the interface.

External Threading API and the ``<__external_threading>`` header
================================================================

In order to support vendors with custom threading API's libc++ allows the
entire internal threading interface to be provided by an external,
vendor provided, header.

When ``_LIBCPP_HAS_THREAD_API_EXTERNAL`` is defined the ``<__threading_support>``
header simply forwards to the ``<__external_threading>`` header (which must exist).
It is expected that the ``<__external_threading>`` header provide the exact
interface normally provided by ``<__threading_support>``.

External Threading Library
==========================

libc++ can be compiled with its internal threading API delegating to an external
library. Such a configuration is useful for library vendors who wish to
distribute a thread-agnostic libc++ library, where the users of the library are
expected to provide the implementation of the libc++ internal threading API.

On a production setting, this would be achieved through a custom
``<__external_threading>`` header, which declares the libc++ internal threading
API but leaves out the implementation.

The ``-DLIBCXX_BUILD_EXTERNAL_THREAD_LIBRARY`` option allows building libc++ in
such a configuration while allowing it to be tested on a platform that supports
any of the threading systems (e.g. pthread) supported in ``__threading_support``
header. Therefore, the main purpose of this option is to allow testing of this
particular configuration of the library without being tied to a vendor-specific
threading system. This option is only meant to be used by libc++ library
developers.

Threading Configuration Macros
==============================

**_LIBCPP_HAS_NO_THREADS**
  This macro is defined when libc++ is built without threading support. It
  should not be manually defined by the user.

**_LIBCPP_HAS_THREAD_API_EXTERNAL**
  This macro is defined when libc++ should use the ``<__external_threading>``
  header to provide the internal threading API. This macro overrides
  ``_LIBCPP_HAS_THREAD_API_PTHREAD``.

**_LIBCPP_HAS_THREAD_API_PTHREAD**
  This macro is defined when libc++ should use POSIX threads to implement the
  internal threading API.

**_LIBCPP_HAS_THREAD_API_WIN32**
  This macro is defined when libc++ should use Win32 threads to implement the
  internal threading API.

**_LIBCPP_HAS_THREAD_LIBRARY_EXTERNAL**
  This macro is defined when libc++ expects the definitions of the internal
  threading API to be provided by an external library. When defined
  ``<__threading_support>`` will only provide the forward declarations and
  typedefs for the internal threading API.

**_LIBCPP_BUILDING_THREAD_LIBRARY_EXTERNAL**
  This macro is used to build an external threading library using the
  ``<__threading_support>``. Specifically it exposes the threading API
  definitions in ``<__threading_support>`` as non-inline definitions meant to
  be compiled into a library.
