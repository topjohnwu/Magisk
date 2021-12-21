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

// basic_ostream<charT,traits>* tie() const;

#include <ios>
#include <cassert>

int main()
{
    const std::basic_ios<char> ios(0);
    assert(ios.tie() == 0);
}
