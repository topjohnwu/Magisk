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
    std::locale cloc;
    std::locale copy(cloc);
    std::locale n1(LOCALE_en_US_UTF_8);
    std::locale n2(LOCALE_en_US_UTF_8);
    std::locale noname1 = n1.combine<std::ctype<char> >(cloc);
    std::locale nonamec = noname1;
    std::locale noname2 = n1.combine<std::ctype<char> >(cloc);

    assert(cloc == cloc);
    assert(cloc == copy);
    assert(cloc != n1);
    assert(cloc != n2);
    assert(cloc != noname1);
    assert(cloc != nonamec);
    assert(cloc != noname2);

    assert(copy == cloc);
    assert(copy == copy);
    assert(copy != n1);
    assert(copy != n2);
    assert(copy != noname1);
    assert(copy != nonamec);
    assert(copy != noname2);

    assert(n1 != cloc);
    assert(n1 != copy);
    assert(n1 == n1);
    assert(n1 == n2);
    assert(n1 != noname1);
    assert(n1 != nonamec);
    assert(n1 != noname2);

    assert(n2 != cloc);
    assert(n2 != copy);
    assert(n2 == n1);
    assert(n2 == n2);
    assert(n2 != noname1);
    assert(n2 != nonamec);
    assert(n2 != noname2);

    assert(noname1 != cloc);
    assert(noname1 != copy);
    assert(noname1 != n1);
    assert(noname1 != n2);
    assert(noname1 == noname1);
    assert(noname1 == nonamec);
    assert(noname1 != noname2);

    assert(nonamec != cloc);
    assert(nonamec != copy);
    assert(nonamec != n1);
    assert(nonamec != n2);
    assert(nonamec == noname1);
    assert(nonamec == nonamec);
    assert(nonamec != noname2);

    assert(noname2 != cloc);
    assert(noname2 != copy);
    assert(noname2 != n1);
    assert(noname2 != n2);
    assert(noname2 != noname1);
    assert(noname2 != nonamec);
    assert(noname2 == noname2);
}
