//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <memory>

// pointer_safety get_pointer_safety();

// The pointer_safety interface is no longer provided in C++03 in the new ABI.
// XFAIL: c++98, c++03

// MODULES_DEFINES: _LIBCPP_ABI_POINTER_SAFETY_ENUM_TYPE
#define _LIBCPP_ABI_POINTER_SAFETY_ENUM_TYPE
#include <memory>
#include <cassert>

int main()
{
  {
    static_assert(std::is_enum<std::pointer_safety>::value, "");
    static_assert(!std::is_convertible<std::pointer_safety, int>::value, "");
    static_assert(std::is_same<
        std::underlying_type<std::pointer_safety>::type,
        unsigned char
    >::value, "");
  }
  {
    std::pointer_safety r = std::get_pointer_safety();
    assert(r == std::pointer_safety::relaxed ||
           r == std::pointer_safety::preferred ||
           r == std::pointer_safety::strict);
  }
}
