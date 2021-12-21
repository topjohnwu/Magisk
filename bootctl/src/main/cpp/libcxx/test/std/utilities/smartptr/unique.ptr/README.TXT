Test Naming and Directory Structure
===================================

The directory structure for the unique_ptr class templates differs from the
normal test directory naming conventions (e.g. matching the stable name in the standard).

Instead of having a [unique.ptr.single] and [unique.ptr.runtime] directory,
each containing their own tests, a single directory, "unique.ptr.class",
contains both sets of tests.

This allows the common behavior of the two unique_ptr specializations to be
tested in the same place without duplication.

Tests specific to [unique.ptr.single] have the suffix ".single.pass.cpp"
and those specific to [unique.ptr.runtime] are named "*.runtime.pass.cpp".
Tests for both specializations are named normally.
