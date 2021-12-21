//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <locale>

// template <> class ctype<char>

// ~ctype();

#include <locale>
#include <cassert>

#include "count_new.hpp"

int main()
{
    {
        std::locale l(std::locale::classic(), new std::ctype<char>);
        assert(globalMemCounter.checkDeleteArrayCalledEq(0));
    }
    assert(globalMemCounter.checkDeleteArrayCalledEq(0));
    {
        std::ctype<char>::mask table[256];
        std::locale l(std::locale::classic(), new std::ctype<char>(table));
        assert(globalMemCounter.checkDeleteArrayCalledEq(0));
    }
    assert(globalMemCounter.checkDeleteArrayCalledEq(0));
    {
        std::locale l(std::locale::classic(),
            new std::ctype<char>(new std::ctype<char>::mask[256], true));
        assert(globalMemCounter.checkDeleteArrayCalledEq(0));
    }
    assert(globalMemCounter.checkDeleteArrayCalledEq(1));
}
