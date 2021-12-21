//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <ios>

// template <class charT, class traits> class basic_ios

// operator unspecified-bool-type() const;

#include <ios>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

int main()
{
    std::ios ios(0);
    assert(static_cast<bool>(ios) == !ios.fail());
    ios.setstate(std::ios::failbit);
    assert(static_cast<bool>(ios) == !ios.fail());
    static_assert((!std::is_convertible<std::ios, void*>::value), "");
    static_assert((!std::is_convertible<std::ios, int>::value), "");
    static_assert((!std::is_convertible<std::ios const&, int>::value), "");
#if TEST_STD_VER >= 11
    static_assert((!std::is_convertible<std::ios, bool>::value), "");
#endif
}
