//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <locale>

// wstring_convert<Codecvt, Elem, Wide_alloc, Byte_alloc>

// wstring_convert(Codecvt* pcvt = new Codecvt);

#include <locale>
#include <codecvt>
#include <cassert>

#include "test_macros.h"

int main()
{
    {
        typedef std::codecvt_utf8<wchar_t> Codecvt;
        typedef std::wstring_convert<Codecvt> Myconv;
        Myconv myconv;
        assert(myconv.converted() == 0);
    }
    {
        typedef std::codecvt_utf8<wchar_t> Codecvt;
        typedef std::wstring_convert<Codecvt> Myconv;
        Myconv myconv(new Codecvt);
        assert(myconv.converted() == 0);
#if TEST_STD_VER > 11
        static_assert(!std::is_convertible<Codecvt*, Myconv>::value, "");
        static_assert( std::is_constructible<Myconv, Codecvt*>::value, "");
#endif
    }
}
