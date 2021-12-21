//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <wctype.h>

#include <wctype.h>
#include <type_traits>

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
    wint_t w = 0;
    wctrans_t wctr = 0;
    wctype_t wct = 0;
    static_assert((std::is_same<decltype(iswalnum(w)), int>::value), "");
    static_assert((std::is_same<decltype(iswalpha(w)), int>::value), "");
    static_assert((std::is_same<decltype(iswblank(w)), int>::value), "");
    static_assert((std::is_same<decltype(iswcntrl(w)), int>::value), "");
    static_assert((std::is_same<decltype(iswdigit(w)), int>::value), "");
    static_assert((std::is_same<decltype(iswgraph(w)), int>::value), "");
    static_assert((std::is_same<decltype(iswlower(w)), int>::value), "");
    static_assert((std::is_same<decltype(iswprint(w)), int>::value), "");
    static_assert((std::is_same<decltype(iswpunct(w)), int>::value), "");
    static_assert((std::is_same<decltype(iswspace(w)), int>::value), "");
    static_assert((std::is_same<decltype(iswupper(w)), int>::value), "");
    static_assert((std::is_same<decltype(iswxdigit(w)), int>::value), "");
    static_assert((std::is_same<decltype(iswctype(w, wct)), int>::value), "");
    static_assert((std::is_same<decltype(wctype("")), wctype_t>::value), "");
    static_assert((std::is_same<decltype(towlower(w)), wint_t>::value), "");
    static_assert((std::is_same<decltype(towupper(w)), wint_t>::value), "");
    static_assert((std::is_same<decltype(towctrans(w, wctr)), wint_t>::value), "");
    static_assert((std::is_same<decltype(wctrans("")), wctrans_t>::value), "");
}
