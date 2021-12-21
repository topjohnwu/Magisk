//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// REQUIRES: locale.en_US.UTF-8

// <locale>

// basic_string<char> name() const;

#include <locale>
#include <cassert>

#include "platform_support.h" // locale name macros

int main()
{
    {
        std::locale loc;
        assert(loc.name() == "C");
    }
    {
        std::locale loc(LOCALE_en_US_UTF_8);
        assert(loc.name() == LOCALE_en_US_UTF_8);
    }
}
