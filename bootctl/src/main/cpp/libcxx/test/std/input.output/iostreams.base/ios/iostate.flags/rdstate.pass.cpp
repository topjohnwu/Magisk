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

// iostate rdstate() const;

#include <ios>
#include <cassert>

int main()
{
    std::ios ios(0);
    assert(ios.rdstate() == std::ios::badbit);
    ios.setstate(std::ios::failbit);
    assert(ios.rdstate() == (std::ios::failbit | std::ios::badbit));
}
