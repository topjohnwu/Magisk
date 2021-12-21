//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// XFAIL: newlib

// <fenv.h>

#include <fenv.h>
#include <type_traits>

#ifndef FE_DIVBYZERO
#error FE_DIVBYZERO not defined
#endif

#ifndef FE_INEXACT
#error FE_INEXACT not defined
#endif

#ifndef FE_INVALID
#error FE_INVALID not defined
#endif

#ifndef FE_OVERFLOW
#error FE_OVERFLOW not defined
#endif

#ifndef FE_UNDERFLOW
#error FE_UNDERFLOW not defined
#endif

#ifndef FE_ALL_EXCEPT
#error FE_ALL_EXCEPT not defined
#endif

#ifndef FE_DOWNWARD
#error FE_DOWNWARD not defined
#endif

#ifndef FE_TONEAREST
#error FE_TONEAREST not defined
#endif

#ifndef FE_TOWARDZERO
#error FE_TOWARDZERO not defined
#endif

#ifndef FE_UPWARD
#error FE_UPWARD not defined
#endif

#ifndef FE_DFL_ENV
#error FE_DFL_ENV not defined
#endif

int main()
{
    fenv_t fenv = {};
    fexcept_t fex = 0;
    static_assert((std::is_same<decltype(feclearexcept(0)), int>::value), "");
    static_assert((std::is_same<decltype(fegetexceptflag(&fex, 0)), int>::value), "");
    static_assert((std::is_same<decltype(feraiseexcept(0)), int>::value), "");
    static_assert((std::is_same<decltype(fesetexceptflag(&fex, 0)), int>::value), "");
    static_assert((std::is_same<decltype(fetestexcept(0)), int>::value), "");
    static_assert((std::is_same<decltype(fegetround()), int>::value), "");
    static_assert((std::is_same<decltype(fesetround(0)), int>::value), "");
    static_assert((std::is_same<decltype(fegetenv(&fenv)), int>::value), "");
    static_assert((std::is_same<decltype(feholdexcept(&fenv)), int>::value), "");
    static_assert((std::is_same<decltype(fesetenv(&fenv)), int>::value), "");
    static_assert((std::is_same<decltype(feupdateenv(&fenv)), int>::value), "");
}
