.. _index:

=============================
"libc++" C++ Standard Library
=============================

Overview
========

libc++ is a new implementation of the C++ standard library, targeting C++11 and
above.

* Features and Goals

  * Correctness as defined by the C++11 standard.
  * Fast execution.
  * Minimal memory use.
  * Fast compile times.
  * ABI compatibility with gcc's libstdc++ for some low-level features
    such as exception objects, rtti and memory allocation.
  * Extensive unit tests.

* Design and Implementation:

  * Extensive unit tests
  * Internal linker model can be dumped/read to textual format
  * Additional linking features can be plugged in as "passes"
  * OS specific and CPU specific code factored out


Getting Started with libc++
---------------------------

.. toctree::
   :maxdepth: 2

   ReleaseNotes
   UsingLibcxx
   BuildingLibcxx
   TestingLibcxx


Current Status
--------------

After its initial introduction, many people have asked "why start a new
library instead of contributing to an existing library?" (like Apache's
libstdcxx, GNU's libstdc++, STLport, etc).  There are many contributing
reasons, but some of the major ones are:

* From years of experience (including having implemented the standard
  library before), we've learned many things about implementing
  the standard containers which require ABI breakage and fundamental changes
  to how they are implemented.  For example, it is generally accepted that
  building std::string using the "short string optimization" instead of
  using Copy On Write (COW) is a superior approach for multicore
  machines (particularly in C++11, which has rvalue references).  Breaking
  ABI compatibility with old versions of the library was
  determined to be critical to achieving the performance goals of
  libc++.

* Mainline libstdc++ has switched to GPL3, a license which the developers
  of libc++ cannot use.  libstdc++ 4.2 (the last GPL2 version) could be
  independently extended to support C++11, but this would be a fork of the
  codebase (which is often seen as worse for a project than starting a new
  independent one).  Another problem with libstdc++ is that it is tightly
  integrated with G++ development, tending to be tied fairly closely to the
  matching version of G++.

* STLport and the Apache libstdcxx library are two other popular
  candidates, but both lack C++11 support.  Our experience (and the
  experience of libstdc++ developers) is that adding support for C++11 (in
  particular rvalue references and move-only types) requires changes to
  almost every class and function, essentially amounting to a rewrite.
  Faced with a rewrite, we decided to start from scratch and evaluate every
  design decision from first principles based on experience.
  Further, both projects are apparently abandoned: STLport 5.2.1 was
  released in Oct'08, and STDCXX 4.2.1 in May'08.

Platform and Compiler Support
-----------------------------

libc++ is known to work on the following platforms, using gcc and
clang.
Note that functionality provided by ``<atomic>`` is only functional with clang
and GCC.

============ ==================== ============ ========================
OS           Arch                 Compilers    ABI Library
============ ==================== ============ ========================
Mac OS X     i386, x86_64         Clang, GCC   libc++abi
FreeBSD 10+  i386, x86_64, ARM    Clang, GCC   libcxxrt, libc++abi
Linux        i386, x86_64         Clang, GCC   libc++abi
============ ==================== ============ ========================

The following minimum compiler versions are strongly recommended.

* Clang 3.5 and above
* GCC 4.7 and above.

Anything older *may* work.

C++ Dialect Support
---------------------

* C++11 - Complete
* `C++14 - Complete <http://libcxx.llvm.org/cxx1y_status.html>`__
* `C++17 - In Progress <http://libcxx.llvm.org/cxx1z_status.html>`__
* `Post C++14 Technical Specifications - In Progress <http://libcxx.llvm.org/ts1z_status.html>`__

Notes and Known Issues
----------------------

This list contains known issues with libc++

* Building libc++ with ``-fno-rtti`` is not supported. However
  linking against it with ``-fno-rtti`` is supported.
* On OS X v10.8 and older the CMake option ``-DLIBCXX_LIBCPPABI_VERSION=""``
  must be used during configuration.


A full list of currently open libc++ bugs can be `found here`__.

.. __:  https://bugs.llvm.org/buglist.cgi?component=All%20Bugs&product=libc%2B%2B&query_format=advanced&resolution=---&order=changeddate%20DESC%2Cassigned_to%20DESC%2Cbug_status%2Cpriority%2Cbug_id&list_id=74184

Design Documents
----------------

.. toctree::
   :maxdepth: 1

   DesignDocs/AvailabilityMarkup
   DesignDocs/DebugMode
   DesignDocs/CapturingConfigInfo
   DesignDocs/ABIVersioning
   DesignDocs/VisibilityMacros
   DesignDocs/ThreadingSupportAPI
   DesignDocs/FileTimeType

* `<atomic> design <http://libcxx.llvm.org/atomic_design.html>`_
* `<type_traits> design <http://libcxx.llvm.org/type_traits_design.html>`_
* `Notes by Marshall Clow`__

.. __: https://cplusplusmusings.wordpress.com/2012/07/05/clang-and-standard-libraries-on-mac-os-x/

Build Bots and Test Coverage
----------------------------

* `LLVM Buildbot Builders <http://lab.llvm.org:8011/console>`_
* `Apple Jenkins Builders <http://lab.llvm.org:8080/green/view/Libcxx/>`_
* `Windows Appveyor Builders <https://ci.appveyor.com/project/llvm-mirror/libcxx>`_
* `Code Coverage Results <http://efcs.ca/libcxx-coverage>`_

Getting Involved
================

First please review our `Developer's Policy <http://llvm.org/docs/DeveloperPolicy.html>`__
and `Getting started with LLVM <http://llvm.org/docs/GettingStarted.html>`__.

**Bug Reports**

If you think you've found a bug in libc++, please report it using
the `LLVM Bugzilla`_. If you're not sure, you
can post a message to the `libcxx-dev mailing list`_ or on IRC.

**Patches**

If you want to contribute a patch to libc++, the best place for that is
`Phabricator <http://llvm.org/docs/Phabricator.html>`_. Please add `libcxx-commits` as a subscriber.
Also make sure you are subscribed to the `libcxx-commits mailing list <http://lists.llvm.org/mailman/listinfo/libcxx-commits>`_.

**Discussion and Questions**

Send discussions and questions to the
`libcxx-dev mailing list <http://lists.llvm.org/mailman/listinfo/libcxx-dev>`_.



Quick Links
===========
* `LLVM Homepage <http://llvm.org/>`_
* `libc++abi Homepage <http://libcxxabi.llvm.org/>`_
* `LLVM Bugzilla <https://bugs.llvm.org/>`_
* `libcxx-commits Mailing List`_
* `libcxx-dev Mailing List`_
* `Browse libc++ -- SVN <http://llvm.org/svn/llvm-project/libcxx/trunk/>`_
* `Browse libc++ -- ViewVC <http://llvm.org/viewvc/llvm-project/libcxx/trunk/>`_
