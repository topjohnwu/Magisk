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

// basic_ostream<charT,traits>* tie(basic_ostream<charT,traits>* tiestr);

#include <ios>
#include <cassert>

int main()
{
    std::ios ios(0);
    std::ostream* os = (std::ostream*)1;
    std::ostream* r = ios.tie(os);
    assert(r == 0);
    assert(ios.tie() == os);
}
