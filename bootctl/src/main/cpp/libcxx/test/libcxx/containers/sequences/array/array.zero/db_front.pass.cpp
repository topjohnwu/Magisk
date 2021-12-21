//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: libcpp-no-exceptions
// MODULES_DEFINES: _LIBCPP_DEBUG=1
// MODULES_DEFINES: _LIBCPP_DEBUG_USE_EXCEPTIONS

// Can't test the system lib because this test enables debug mode
// UNSUPPORTED: with_system_cxx_lib

// test array<T, 0>::front() throws a debug exception.

#define _LIBCPP_DEBUG 1
#define _LIBCPP_DEBUG_USE_EXCEPTIONS
#include <array>

template <class Array>
inline bool CheckDebugThrows(Array& Arr) {
  try {
    Arr.front();
  } catch (std::__libcpp_debug_exception const&) {
    return true;
  }
  return false;
}

int main()
{
  {
    typedef std::array<int, 0> C;
    C c = {};
    C const& cc = c;
    assert(CheckDebugThrows(c));
    assert(CheckDebugThrows(cc));
  }
  {
    typedef std::array<const int, 0> C;
    C c = {{}};
    C const& cc = c;
    assert(CheckDebugThrows(c));
    assert(CheckDebugThrows(cc));
  }
}
