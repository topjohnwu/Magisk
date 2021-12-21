==========
Debug Mode
==========

.. contents::
   :local:

.. _using-debug-mode:

Using Debug Mode
================

Libc++ provides a debug mode that enables assertions meant to detect incorrect
usage of the standard library. By default these assertions are disabled but
they can be enabled using the ``_LIBCPP_DEBUG`` macro.

**_LIBCPP_DEBUG** Macro
-----------------------

**_LIBCPP_DEBUG**:
  This macro is used to enable assertions and iterator debugging checks within
  libc++. By default it is undefined.

  **Values**: ``0``, ``1``

  Defining ``_LIBCPP_DEBUG`` to ``0`` or greater enables most of libc++'s
  assertions. Defining ``_LIBCPP_DEBUG`` to ``1`` enables "iterator debugging"
  which provides additional assertions about the validity of iterators used by
  the program.

  Note that this option has no effect on libc++'s ABI

**_LIBCPP_DEBUG_USE_EXCEPTIONS**:
  When this macro is defined ``_LIBCPP_ASSERT`` failures throw
  ``__libcpp_debug_exception`` instead of aborting. Additionally this macro
  disables exception specifications on functions containing ``_LIBCPP_ASSERT``
  checks. This allows assertion failures to correctly throw through these
  functions.

Handling Assertion Failures
---------------------------

When a debug assertion fails the assertion handler is called via the
``std::__libcpp_debug_function`` function pointer. It is possible to override
this function pointer using a different handler function. Libc++ provides two
different assertion handlers, the default handler
``std::__libcpp_abort_debug_handler`` which aborts the program, and
``std::__libcpp_throw_debug_handler`` which throws an instance of
``std::__libcpp_debug_exception``. Libc++ can be changed to use the throwing
assertion handler as follows:

.. code-block:: cpp

  #define _LIBCPP_DEBUG 1
  #include <string>
  int main() {
    std::__libcpp_debug_function = std::__libcpp_throw_debug_function;
    try {
      std::string::iterator bad_it;
      std::string str("hello world");
      str.insert(bad_it, '!'); // causes debug assertion
    } catch (std::__libcpp_debug_exception const&) {
      return EXIT_SUCCESS;
    }
    return EXIT_FAILURE;
  }

Debug Mode Checks
=================

Libc++'s debug mode offers two levels of checking. The first enables various
precondition checks throughout libc++. The second additionally enables
"iterator debugging" which checks the validity of iterators used by the program.

Basic Checks
============

These checks are enabled when ``_LIBCPP_DEBUG`` is defined to either 0 or 1.

The following checks are enabled by ``_LIBCPP_DEBUG``:

  * FIXME: Update this list

Iterator Debugging Checks
=========================

These checks are enabled when ``_LIBCPP_DEBUG`` is defined to 1.

The following containers and STL classes support iterator debugging:

  * ``std::string``
  * ``std::vector<T>`` (``T != bool``)
  * ``std::list``
  * ``std::unordered_map``
  * ``std::unordered_multimap``
  * ``std::unordered_set``
  * ``std::unordered_multiset``

The remaining containers do not currently support iterator debugging.
Patches welcome.
