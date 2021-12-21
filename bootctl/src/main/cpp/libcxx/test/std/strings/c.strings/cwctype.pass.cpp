//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <cwctype>

#include <cwctype>
#include <type_traits>

#include "test_macros.h"


#ifndef WEOF
#error WEOF not defined
#endif

#ifdef iswalnum
#error iswalnum defined
#endif

#ifdef iswalpha
#error iswalpha defined
#endif

#ifdef iswblank
#error iswblank defined
#endif

#ifdef iswcntrl
#error iswcntrl defined
#endif

#ifdef iswdigit
#error iswdigit defined
#endif

#ifdef iswgraph
#error iswgraph defined
#endif

#ifdef iswlower
#error iswlower defined
#endif

#ifdef iswprint
#error iswprint defined
#endif

#ifdef iswpunct
#error iswpunct defined
#endif

#ifdef iswspace
#error iswspace defined
#endif

#ifdef iswupper
#error iswupper defined
#endif

#ifdef iswxdigit
#error iswxdigit defined
#endif

#ifdef iswctype
#error iswctype defined
#endif

#ifdef wctype
#error wctype defined
#endif

#ifdef towlower
#error towlower defined
#endif

#ifdef towupper
#error towupper defined
#endif

#ifdef towctrans
#error towctrans defined
#endif

#ifdef wctrans
#error wctrans defined
#endif

int main()
{
    std::wint_t w = 0;
    ASSERT_SAME_TYPE(int, decltype(std::iswalnum(w)));
    ASSERT_SAME_TYPE(int, decltype(std::iswalpha(w)));
    ASSERT_SAME_TYPE(int, decltype(std::iswblank(w)));
    ASSERT_SAME_TYPE(int, decltype(std::iswcntrl(w)));
    ASSERT_SAME_TYPE(int, decltype(std::iswdigit(w)));
    ASSERT_SAME_TYPE(int, decltype(std::iswgraph(w)));
    ASSERT_SAME_TYPE(int, decltype(std::iswlower(w)));
    ASSERT_SAME_TYPE(int, decltype(std::iswprint(w)));
    ASSERT_SAME_TYPE(int, decltype(std::iswpunct(w)));
    ASSERT_SAME_TYPE(int, decltype(std::iswspace(w)));
    ASSERT_SAME_TYPE(int, decltype(std::iswupper(w)));
    ASSERT_SAME_TYPE(int, decltype(std::iswxdigit(w)));

    ASSERT_SAME_TYPE(int, decltype(std::iswctype(w, std::wctype_t())));
    
    ASSERT_SAME_TYPE(std::wctype_t,  decltype(std::wctype("")));
    ASSERT_SAME_TYPE(std::wint_t,    decltype(std::towlower(w)));
    ASSERT_SAME_TYPE(std::wint_t,    decltype(std::towupper(w)));
    ASSERT_SAME_TYPE(std::wint_t,    decltype(std::towctrans(w, std::wctrans_t())));
    ASSERT_SAME_TYPE(std::wctrans_t, decltype(std::wctrans("")));
}
