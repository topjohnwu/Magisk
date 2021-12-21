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

// basic_streambuf<charT,traits>* rdbuf() const;

#include <ios>
#include <streambuf>
#include <cassert>

int main()
{
    {
        const std::ios ios(0);
        assert(ios.rdbuf() == 0);
    }
    {
        std::streambuf* sb = (std::streambuf*)1;
        const std::ios ios(sb);
        assert(ios.rdbuf() == sb);
    }
}
