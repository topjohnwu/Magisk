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

// char_type widen(char c) const;

#include <ios>
#include <cassert>

int main()
{
    const std::ios ios(0);
    assert(ios.widen('c') == 'c');
}
