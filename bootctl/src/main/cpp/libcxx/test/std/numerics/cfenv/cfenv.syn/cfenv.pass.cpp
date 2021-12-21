//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// UNSUPPORTED: newlib

// <cfenv>

#include <cfenv>
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
    std::fenv_t fenv;
    std::fexcept_t fex;
    ((void)fenv); // Prevent unused warning
    ((void)fex); // Prevent unused warning
    static_assert((std::is_same<decltype(std::feclearexcept(0)), int>::value), "");
    static_assert((std::is_same<decltype(std::fegetexceptflag(&fex, 0)), int>::value), "");
    static_assert((std::is_same<decltype(std::feraiseexcept(0)), int>::value), "");
    static_assert((std::is_same<decltype(std::fesetexceptflag(&fex, 0)), int>::value), "");
    static_assert((std::is_same<decltype(std::fetestexcept(0)), int>::value), "");
    static_assert((std::is_same<decltype(std::fegetround()), int>::value), "");
    static_assert((std::is_same<decltype(std::fesetround(0)), int>::value), "");
    static_assert((std::is_same<decltype(std::fegetenv(&fenv)), int>::value), "");
    static_assert((std::is_same<decltype(std::feholdexcept(&fenv)), int>::value), "");
    static_assert((std::is_same<decltype(std::fesetenv(&fenv)), int>::value), "");
    static_assert((std::is_same<decltype(std::feupdateenv(&fenv)), int>::value), "");
}
