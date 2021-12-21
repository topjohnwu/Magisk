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

// basic_streambuf<charT,traits>* rdbuf(basic_streambuf<charT,traits>* sb);

#include <ios>
#include <streambuf>
#include <cassert>

int main()
{
    std::ios ios(0);
    assert(ios.rdbuf() == 0);
    assert(!ios.good());
    std::streambuf* sb = (std::streambuf*)1;
    std::streambuf* sb2 = ios.rdbuf(sb);
    assert(sb2 == 0);
    assert(ios.rdbuf() == sb);
    assert(ios.good());
    sb2 = ios.rdbuf(0);
    assert(sb2 == (std::streambuf*)1);
    assert(ios.rdbuf() == 0);
    assert(ios.bad());
}
