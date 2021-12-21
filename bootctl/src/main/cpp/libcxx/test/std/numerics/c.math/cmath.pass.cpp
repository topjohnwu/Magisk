//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <cmath>

#include <cmath>
#include <limits>
#include <type_traits>
#include <cassert>

#include "test_macros.h"
#include "hexfloat.h"
#include "truncate_fp.h"

// convertible to int/float/double/etc
template <class T, int N=0>
struct Value {
    operator T () { return T(N); }
};

// See PR21083
// Ambiguous is a user-defined type that defines its own overloads of cmath
// functions. When the std overloads are candidates too (by using or adl),
// they should not interfere.
struct Ambiguous : std::true_type { // ADL
    operator float () { return 0.f; }
    operator double () { return 0.; }
};
Ambiguous abs(Ambiguous){ return Ambiguous(); }
Ambiguous acos(Ambiguous){ return Ambiguous(); }
Ambiguous asin(Ambiguous){ return Ambiguous(); }
Ambiguous atan(Ambiguous){ return Ambiguous(); }
Ambiguous atan2(Ambiguous, Ambiguous){ return Ambiguous(); }
Ambiguous ceil(Ambiguous){ return Ambiguous(); }
Ambiguous cos(Ambiguous){ return Ambiguous(); }
Ambiguous cosh(Ambiguous){ return Ambiguous(); }
Ambiguous exp(Ambiguous){ return Ambiguous(); }
Ambiguous fabs(Ambiguous){ return Ambiguous(); }
Ambiguous floor(Ambiguous){ return Ambiguous(); }
Ambiguous fmod(Ambiguous, Ambiguous){ return Ambiguous(); }
Ambiguous frexp(Ambiguous, int*){ return Ambiguous(); }
Ambiguous ldexp(Ambiguous, int){ return Ambiguous(); }
Ambiguous log(Ambiguous){ return Ambiguous(); }
Ambiguous log10(Ambiguous){ return Ambiguous(); }
Ambiguous modf(Ambiguous, Ambiguous*){ return Ambiguous(); }
Ambiguous pow(Ambiguous, Ambiguous){ return Ambiguous(); }
Ambiguous sin(Ambiguous){ return Ambiguous(); }
Ambiguous sinh(Ambiguous){ return Ambiguous(); }
Ambiguous sqrt(Ambiguous){ return Ambiguous(); }
Ambiguous tan(Ambiguous){ return Ambiguous(); }
Ambiguous tanh(Ambiguous){ return Ambiguous(); }
Ambiguous signbit(Ambiguous){ return Ambiguous(); }
Ambiguous fpclassify(Ambiguous){ return Ambiguous(); }
Ambiguous isfinite(Ambiguous){ return Ambiguous(); }
Ambiguous isnormal(Ambiguous){ return Ambiguous(); }
Ambiguous isgreater(Ambiguous, Ambiguous){ return Ambiguous(); }
Ambiguous isgreaterequal(Ambiguous, Ambiguous){ return Ambiguous(); }
Ambiguous isless(Ambiguous, Ambiguous){ return Ambiguous(); }
Ambiguous islessequal(Ambiguous, Ambiguous){ return Ambiguous(); }
Ambiguous islessgreater(Ambiguous, Ambiguous){ return Ambiguous(); }
Ambiguous isunordered(Ambiguous, Ambiguous){ return Ambiguous(); }
Ambiguous acosh(Ambiguous){ return Ambiguous(); }
Ambiguous asinh(Ambiguous){ return Ambiguous(); }
Ambiguous atanh(Ambiguous){ return Ambiguous(); }
Ambiguous cbrt(Ambiguous){ return Ambiguous(); }
Ambiguous copysign(Ambiguous, Ambiguous){ return Ambiguous(); }
Ambiguous erf(Ambiguous){ return Ambiguous(); }
Ambiguous erfc(Ambiguous){ return Ambiguous(); }
Ambiguous exp2(Ambiguous){ return Ambiguous(); }
Ambiguous expm1(Ambiguous){ return Ambiguous(); }
Ambiguous fdim(Ambiguous, Ambiguous){ return Ambiguous(); }
Ambiguous fma(Ambiguous, Ambiguous, Ambiguous){ return Ambiguous(); }
Ambiguous fmax(Ambiguous, Ambiguous){ return Ambiguous(); }
Ambiguous fmin(Ambiguous, Ambiguous){ return Ambiguous(); }
Ambiguous hypot(Ambiguous, Ambiguous){ return Ambiguous(); }
Ambiguous hypot(Ambiguous, Ambiguous, Ambiguous){ return Ambiguous(); }
Ambiguous ilogb(Ambiguous){ return Ambiguous(); }
Ambiguous lgamma(Ambiguous){ return Ambiguous(); }
Ambiguous llrint(Ambiguous){ return Ambiguous(); }
Ambiguous llround(Ambiguous){ return Ambiguous(); }
Ambiguous log1p(Ambiguous){ return Ambiguous(); }
Ambiguous log2(Ambiguous){ return Ambiguous(); }
Ambiguous logb(Ambiguous){ return Ambiguous(); }
Ambiguous lrint(Ambiguous){ return Ambiguous(); }
Ambiguous lround(Ambiguous){ return Ambiguous(); }
Ambiguous nearbyint(Ambiguous){ return Ambiguous(); }
Ambiguous nextafter(Ambiguous, Ambiguous){ return Ambiguous(); }
Ambiguous nexttoward(Ambiguous, Ambiguous){ return Ambiguous(); }
Ambiguous remainder(Ambiguous, Ambiguous){ return Ambiguous(); }
Ambiguous remquo(Ambiguous, Ambiguous, int*){ return Ambiguous(); }
Ambiguous rint(Ambiguous){ return Ambiguous(); }
Ambiguous round(Ambiguous){ return Ambiguous(); }
Ambiguous scalbln(Ambiguous, Ambiguous){ return Ambiguous(); }
Ambiguous scalbn(Ambiguous, Ambiguous){ return Ambiguous(); }
Ambiguous tgamma(Ambiguous){ return Ambiguous(); }
Ambiguous trunc(Ambiguous){ return Ambiguous(); }

void test_abs()
{
    static_assert((std::is_same<decltype(std::abs((float)0)), float>::value), "");
    static_assert((std::is_same<decltype(std::abs((double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::abs((long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(abs(Ambiguous())), Ambiguous>::value), "");
    assert(std::abs(-1.) == 1);
}

void test_acos()
{
    static_assert((std::is_same<decltype(std::acos((float)0)), float>::value), "");
    static_assert((std::is_same<decltype(std::acos((bool)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::acos((unsigned short)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::acos((int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::acos((unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::acos((long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::acos((unsigned long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::acos((long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::acos((unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::acos((double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::acos((long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::acosf(0)), float>::value), "");
    static_assert((std::is_same<decltype(std::acosl(0)), long double>::value), "");
    static_assert((std::is_same<decltype(acos(Ambiguous())), Ambiguous>::value), "");
    assert(std::acos(1) == 0);
}

void test_asin()
{
    static_assert((std::is_same<decltype(std::asin((float)0)), float>::value), "");
    static_assert((std::is_same<decltype(std::asin((bool)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::asin((unsigned short)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::asin((int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::asin((unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::asin((long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::asin((unsigned long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::asin((long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::asin((unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::asin((double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::asin((long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::asinf(0)), float>::value), "");
    static_assert((std::is_same<decltype(std::asinl(0)), long double>::value), "");
    static_assert((std::is_same<decltype(asin(Ambiguous())), Ambiguous>::value), "");
    assert(std::asin(0) == 0);
}

void test_atan()
{
    static_assert((std::is_same<decltype(std::atan((float)0)), float>::value), "");
    static_assert((std::is_same<decltype(std::atan((bool)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::atan((unsigned short)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::atan((int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::atan((unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::atan((long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::atan((unsigned long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::atan((long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::atan((unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::atan((double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::atan((long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::atanf(0)), float>::value), "");
    static_assert((std::is_same<decltype(std::atanl(0)), long double>::value), "");
    static_assert((std::is_same<decltype(atan(Ambiguous())), Ambiguous>::value), "");
    assert(std::atan(0) == 0);
}

void test_atan2()
{
    static_assert((std::is_same<decltype(std::atan2((float)0, (float)0)), float>::value), "");
    static_assert((std::is_same<decltype(std::atan2((bool)0, (float)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::atan2((unsigned short)0, (double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::atan2((int)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::atan2((float)0, (unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::atan2((double)0, (long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::atan2((long double)0, (unsigned long)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::atan2((int)0, (long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::atan2((int)0, (unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::atan2((double)0, (double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::atan2((long double)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::atan2((float)0, (double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::atan2((float)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::atan2((double)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::atan2f(0,0)), float>::value), "");
    static_assert((std::is_same<decltype(std::atan2l(0,0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::atan2((int)0, (int)0)), double>::value), "");
    static_assert((std::is_same<decltype(atan2(Ambiguous(), Ambiguous())), Ambiguous>::value), "");
    assert(std::atan2(0,1) == 0);
}

void test_ceil()
{
    static_assert((std::is_same<decltype(std::ceil((float)0)), float>::value), "");
    static_assert((std::is_same<decltype(std::ceil((bool)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::ceil((unsigned short)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::ceil((int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::ceil((unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::ceil((long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::ceil((unsigned long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::ceil((long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::ceil((unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::ceil((double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::ceil((long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::ceilf(0)), float>::value), "");
    static_assert((std::is_same<decltype(std::ceill(0)), long double>::value), "");
    static_assert((std::is_same<decltype(ceil(Ambiguous())), Ambiguous>::value), "");
    assert(std::ceil(0) == 0);
}

void test_cos()
{
    static_assert((std::is_same<decltype(std::cos((float)0)), float>::value), "");
    static_assert((std::is_same<decltype(std::cos((bool)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::cos((unsigned short)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::cos((int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::cos((unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::cos((long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::cos((unsigned long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::cos((long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::cos((unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::cos((double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::cos((long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::cosf(0)), float>::value), "");
    static_assert((std::is_same<decltype(std::cosl(0)), long double>::value), "");
    static_assert((std::is_same<decltype(cos(Ambiguous())), Ambiguous>::value), "");
    assert(std::cos(0) == 1);
}

void test_cosh()
{
    static_assert((std::is_same<decltype(std::cosh((float)0)), float>::value), "");
    static_assert((std::is_same<decltype(std::cosh((bool)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::cosh((unsigned short)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::cosh((int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::cosh((unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::cosh((long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::cosh((unsigned long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::cosh((long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::cosh((unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::cosh((double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::cosh((long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::coshf(0)), float>::value), "");
    static_assert((std::is_same<decltype(std::coshl(0)), long double>::value), "");
    static_assert((std::is_same<decltype(cosh(Ambiguous())), Ambiguous>::value), "");
    assert(std::cosh(0) == 1);
}

void test_exp()
{
    static_assert((std::is_same<decltype(std::exp((float)0)), float>::value), "");
    static_assert((std::is_same<decltype(std::exp((bool)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::exp((unsigned short)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::exp((int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::exp((unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::exp((long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::exp((unsigned long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::exp((long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::exp((unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::exp((double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::exp((long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::expf(0)), float>::value), "");
    static_assert((std::is_same<decltype(std::expl(0)), long double>::value), "");
    static_assert((std::is_same<decltype(exp(Ambiguous())), Ambiguous>::value), "");
    assert(std::exp(0) == 1);
}

void test_fabs()
{
    static_assert((std::is_same<decltype(std::fabs((float)0)), float>::value), "");
    static_assert((std::is_same<decltype(std::fabs((bool)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::fabs((unsigned short)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::fabs((int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::fabs((unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::fabs((long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::fabs((unsigned long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::fabs((long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::fabs((unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::fabs((double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::fabs((long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::fabsf(0.0f)), float>::value), "");
    static_assert((std::is_same<decltype(std::fabsl(0.0L)), long double>::value), "");
    static_assert((std::is_same<decltype(fabs(Ambiguous())), Ambiguous>::value), "");
    assert(std::fabs(-1) == 1);
}

void test_floor()
{
    static_assert((std::is_same<decltype(std::floor((float)0)), float>::value), "");
    static_assert((std::is_same<decltype(std::floor((bool)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::floor((unsigned short)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::floor((int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::floor((unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::floor((long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::floor((unsigned long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::floor((long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::floor((unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::floor((double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::floor((long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::floorf(0)), float>::value), "");
    static_assert((std::is_same<decltype(std::floorl(0)), long double>::value), "");
    static_assert((std::is_same<decltype(floor(Ambiguous())), Ambiguous>::value), "");
    assert(std::floor(1) == 1);
}

void test_fmod()
{
    static_assert((std::is_same<decltype(std::fmod((float)0, (float)0)), float>::value), "");
    static_assert((std::is_same<decltype(std::fmod((bool)0, (float)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::fmod((unsigned short)0, (double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::fmod((int)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::fmod((float)0, (unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::fmod((double)0, (long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::fmod((long double)0, (unsigned long)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::fmod((int)0, (long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::fmod((int)0, (unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::fmod((double)0, (double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::fmod((long double)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::fmod((float)0, (double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::fmod((float)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::fmod((double)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::fmodf(0,0)), float>::value), "");
    static_assert((std::is_same<decltype(std::fmodl(0,0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::fmod((int)0, (int)0)), double>::value), "");
    static_assert((std::is_same<decltype(fmod(Ambiguous(), Ambiguous())), Ambiguous>::value), "");
    assert(std::fmod(1.5,1) == .5);
}

void test_frexp()
{
    int ip;
    static_assert((std::is_same<decltype(std::frexp((float)0, &ip)), float>::value), "");
    static_assert((std::is_same<decltype(std::frexp((bool)0, &ip)), double>::value), "");
    static_assert((std::is_same<decltype(std::frexp((unsigned short)0, &ip)), double>::value), "");
    static_assert((std::is_same<decltype(std::frexp((int)0, &ip)), double>::value), "");
    static_assert((std::is_same<decltype(std::frexp((unsigned int)0, &ip)), double>::value), "");
    static_assert((std::is_same<decltype(std::frexp((long)0, &ip)), double>::value), "");
    static_assert((std::is_same<decltype(std::frexp((unsigned long)0, &ip)), double>::value), "");
    static_assert((std::is_same<decltype(std::frexp((long long)0, &ip)), double>::value), "");
    static_assert((std::is_same<decltype(std::frexp((unsigned long long)0, &ip)), double>::value), "");
    static_assert((std::is_same<decltype(std::frexp((double)0, &ip)), double>::value), "");
    static_assert((std::is_same<decltype(std::frexp((long double)0, &ip)), long double>::value), "");
    static_assert((std::is_same<decltype(std::frexpf(0, &ip)), float>::value), "");
    static_assert((std::is_same<decltype(std::frexpl(0, &ip)), long double>::value), "");
    static_assert((std::is_same<decltype(frexp(Ambiguous(), &ip)), Ambiguous>::value), "");
    assert(std::frexp(0, &ip) == 0);
}

void test_ldexp()
{
    int ip = 1;
    static_assert((std::is_same<decltype(std::ldexp((float)0, ip)), float>::value), "");
    static_assert((std::is_same<decltype(std::ldexp((bool)0, ip)), double>::value), "");
    static_assert((std::is_same<decltype(std::ldexp((unsigned short)0, ip)), double>::value), "");
    static_assert((std::is_same<decltype(std::ldexp((int)0, ip)), double>::value), "");
    static_assert((std::is_same<decltype(std::ldexp((unsigned int)0, ip)), double>::value), "");
    static_assert((std::is_same<decltype(std::ldexp((long)0, ip)), double>::value), "");
    static_assert((std::is_same<decltype(std::ldexp((unsigned long)0, ip)), double>::value), "");
    static_assert((std::is_same<decltype(std::ldexp((long long)0, ip)), double>::value), "");
    static_assert((std::is_same<decltype(std::ldexp((unsigned long long)0, ip)), double>::value), "");
    static_assert((std::is_same<decltype(std::ldexp((double)0, ip)), double>::value), "");
    static_assert((std::is_same<decltype(std::ldexp((long double)0, ip)), long double>::value), "");
    static_assert((std::is_same<decltype(std::ldexpf(0, ip)), float>::value), "");
    static_assert((std::is_same<decltype(std::ldexpl(0, ip)), long double>::value), "");
    static_assert((std::is_same<decltype(ldexp(Ambiguous(), ip)), Ambiguous>::value), "");
    assert(std::ldexp(1, ip) == 2);
}

void test_log()
{
    static_assert((std::is_same<decltype(std::log((float)0)), float>::value), "");
    static_assert((std::is_same<decltype(std::log((bool)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::log((unsigned short)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::log((int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::log((unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::log((long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::log((unsigned long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::log((long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::log((unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::log((double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::log((long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::logf(0)), float>::value), "");
    static_assert((std::is_same<decltype(std::logl(0)), long double>::value), "");
    static_assert((std::is_same<decltype(log(Ambiguous())), Ambiguous>::value), "");
    assert(std::log(1) == 0);
}

void test_log10()
{
    static_assert((std::is_same<decltype(std::log10((float)0)), float>::value), "");
    static_assert((std::is_same<decltype(std::log10((bool)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::log10((unsigned short)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::log10((int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::log10((unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::log10((long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::log10((unsigned long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::log10((long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::log10((unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::log10((double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::log10((long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::log10f(0)), float>::value), "");
    static_assert((std::is_same<decltype(std::log10l(0)), long double>::value), "");
    static_assert((std::is_same<decltype(log10(Ambiguous())), Ambiguous>::value), "");
    assert(std::log10(1) == 0);
}

void test_modf()
{
    static_assert((std::is_same<decltype(std::modf((float)0, (float*)0)), float>::value), "");
    static_assert((std::is_same<decltype(std::modf((double)0, (double*)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::modf((long double)0, (long double*)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::modff(0, (float*)0)), float>::value), "");
    static_assert((std::is_same<decltype(std::modfl(0, (long double*)0)), long double>::value), "");
    static_assert((std::is_same<decltype(modf(Ambiguous(), (Ambiguous*)0)), Ambiguous>::value), "");
    double i;
    assert(std::modf(1., &i) == 0);
}

void test_pow()
{
    static_assert((std::is_same<decltype(std::pow((float)0, (float)0)), float>::value), "");
    static_assert((std::is_same<decltype(std::pow((bool)0, (float)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::pow((unsigned short)0, (double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::pow((int)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::pow((float)0, (unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::pow((double)0, (long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::pow((long double)0, (unsigned long)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::pow((int)0, (long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::pow((int)0, (unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::pow((double)0, (double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::pow((long double)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::pow((float)0, (double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::pow((float)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::pow((double)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::powf(0,0)), float>::value), "");
    static_assert((std::is_same<decltype(std::powl(0,0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::pow((int)0, (int)0)), double>::value), "");
//     static_assert((std::is_same<decltype(std::pow(Value<int>(), (int)0)), double>::value), "");
//     static_assert((std::is_same<decltype(std::pow(Value<long double>(), (float)0)), long double>::value), "");
//     static_assert((std::is_same<decltype(std::pow((float) 0, Value<float>())), float>::value), "");
    static_assert((std::is_same<decltype(pow(Ambiguous(), Ambiguous())), Ambiguous>::value), "");
    assert(std::pow(1,1) == 1);
//     assert(std::pow(Value<int,1>(), Value<float,1>())  == 1);
//     assert(std::pow(1.0f, Value<double,1>()) == 1);
//     assert(std::pow(1.0, Value<int,1>()) == 1);
//     assert(std::pow(Value<long double,1>(), 1LL) == 1);
}

void test_sin()
{
    static_assert((std::is_same<decltype(std::sin((float)0)), float>::value), "");
    static_assert((std::is_same<decltype(std::sin((bool)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::sin((unsigned short)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::sin((int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::sin((unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::sin((long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::sin((unsigned long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::sin((long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::sin((unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::sin((double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::sin((long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::sinf(0)), float>::value), "");
    static_assert((std::is_same<decltype(std::sinl(0)), long double>::value), "");
    static_assert((std::is_same<decltype(sin(Ambiguous())), Ambiguous>::value), "");
    assert(std::sin(0) == 0);
}

void test_sinh()
{
    static_assert((std::is_same<decltype(std::sinh((float)0)), float>::value), "");
    static_assert((std::is_same<decltype(std::sinh((bool)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::sinh((unsigned short)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::sinh((int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::sinh((unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::sinh((long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::sinh((unsigned long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::sinh((long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::sinh((unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::sinh((double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::sinh((long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::sinhf(0)), float>::value), "");
    static_assert((std::is_same<decltype(std::sinhl(0)), long double>::value), "");
    static_assert((std::is_same<decltype(sinh(Ambiguous())), Ambiguous>::value), "");
    assert(std::sinh(0) == 0);
}

void test_sqrt()
{
    static_assert((std::is_same<decltype(std::sqrt((float)0)), float>::value), "");
    static_assert((std::is_same<decltype(std::sqrt((bool)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::sqrt((unsigned short)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::sqrt((int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::sqrt((unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::sqrt((long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::sqrt((unsigned long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::sqrt((long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::sqrt((unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::sqrt((double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::sqrt((long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::sqrtf(0)), float>::value), "");
    static_assert((std::is_same<decltype(std::sqrtl(0)), long double>::value), "");
    static_assert((std::is_same<decltype(sqrt(Ambiguous())), Ambiguous>::value), "");
    assert(std::sqrt(4) == 2);
}

void test_tan()
{
    static_assert((std::is_same<decltype(std::tan((float)0)), float>::value), "");
    static_assert((std::is_same<decltype(std::tan((bool)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::tan((unsigned short)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::tan((int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::tan((unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::tan((long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::tan((unsigned long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::tan((long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::tan((unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::tan((double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::tan((long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::tanf(0)), float>::value), "");
    static_assert((std::is_same<decltype(std::tanl(0)), long double>::value), "");
    static_assert((std::is_same<decltype(tan(Ambiguous())), Ambiguous>::value), "");
    assert(std::tan(0) == 0);
}

void test_tanh()
{
    static_assert((std::is_same<decltype(std::tanh((float)0)), float>::value), "");
    static_assert((std::is_same<decltype(std::tanh((bool)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::tanh((unsigned short)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::tanh((int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::tanh((unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::tanh((long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::tanh((unsigned long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::tanh((long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::tanh((unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::tanh((double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::tanh((long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::tanhf(0)), float>::value), "");
    static_assert((std::is_same<decltype(std::tanhl(0)), long double>::value), "");
    static_assert((std::is_same<decltype(tanh(Ambiguous())), Ambiguous>::value), "");
    assert(std::tanh(0) == 0);
}

void test_signbit()
{
#ifdef signbit
#error signbit defined
#endif
    static_assert((std::is_same<decltype(std::signbit((float)0)), bool>::value), "");
    static_assert((std::is_same<decltype(std::signbit((double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(std::signbit(0)), bool>::value), "");
    static_assert((std::is_same<decltype(std::signbit((long double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(signbit(Ambiguous())), Ambiguous>::value), "");
    assert(std::signbit(-1.0) == true);
    assert(std::signbit(0u) == false);
    assert(std::signbit(std::numeric_limits<unsigned>::max()) == false);
    assert(std::signbit(0) == false);
    assert(std::signbit(1) == false);
    assert(std::signbit(-1) == true);
    assert(std::signbit(std::numeric_limits<int>::max()) == false);
    assert(std::signbit(std::numeric_limits<int>::min()) == true);
}

void test_fpclassify()
{
#ifdef fpclassify
#error fpclassify defined
#endif
    static_assert((std::is_same<decltype(std::fpclassify((float)0)), int>::value), "");
    static_assert((std::is_same<decltype(std::fpclassify((double)0)), int>::value), "");
    static_assert((std::is_same<decltype(std::fpclassify(0)), int>::value), "");
    static_assert((std::is_same<decltype(std::fpclassify((long double)0)), int>::value), "");
    static_assert((std::is_same<decltype(fpclassify(Ambiguous())), Ambiguous>::value), "");
    assert(std::fpclassify(-1.0) == FP_NORMAL);
    assert(std::fpclassify(0) == FP_ZERO);
    assert(std::fpclassify(1) == FP_NORMAL);
    assert(std::fpclassify(-1) == FP_NORMAL);
    assert(std::fpclassify(std::numeric_limits<int>::max()) == FP_NORMAL);
    assert(std::fpclassify(std::numeric_limits<int>::min()) == FP_NORMAL);
}

void test_isfinite()
{
#ifdef isfinite
#error isfinite defined
#endif
    static_assert((std::is_same<decltype(std::isfinite((float)0)), bool>::value), "");
    static_assert((std::is_same<decltype(std::isfinite((double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(std::isfinite(0)), bool>::value), "");
    static_assert((std::is_same<decltype(std::isfinite((long double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(isfinite(Ambiguous())), Ambiguous>::value), "");
    assert(std::isfinite(-1.0) == true);
    assert(std::isfinite(0) == true);
    assert(std::isfinite(1) == true);
    assert(std::isfinite(-1) == true);
    assert(std::isfinite(std::numeric_limits<int>::max()) == true);
    assert(std::isfinite(std::numeric_limits<int>::min()) == true);
}

void test_isnormal()
{
#ifdef isnormal
#error isnormal defined
#endif
    static_assert((std::is_same<decltype(std::isnormal((float)0)), bool>::value), "");
    static_assert((std::is_same<decltype(std::isnormal((double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(std::isnormal(0)), bool>::value), "");
    static_assert((std::is_same<decltype(std::isnormal((long double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(isnormal(Ambiguous())), Ambiguous>::value), "");
    assert(std::isnormal(-1.0) == true);
    assert(std::isnormal(0) == false);
    assert(std::isnormal(1) == true);
    assert(std::isnormal(-1) == true);
    assert(std::isnormal(std::numeric_limits<int>::max()) == true);
    assert(std::isnormal(std::numeric_limits<int>::min()) == true);
}

void test_isgreater()
{
#ifdef isgreater
#error isgreater defined
#endif
    static_assert((std::is_same<decltype(std::isgreater((float)0, (float)0)), bool>::value), "");
    static_assert((std::is_same<decltype(std::isgreater((float)0, (double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(std::isgreater((float)0, (long double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(std::isgreater((double)0, (float)0)), bool>::value), "");
    static_assert((std::is_same<decltype(std::isgreater((double)0, (double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(std::isgreater(0, (double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(std::isgreater((double)0, (long double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(std::isgreater((long double)0, (float)0)), bool>::value), "");
    static_assert((std::is_same<decltype(std::isgreater((long double)0, (double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(std::isgreater((long double)0, (long double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(isgreater(Ambiguous(), Ambiguous())), Ambiguous>::value), "");
    assert(std::isgreater(-1.0, 0.F) == false);
}

void test_isgreaterequal()
{
#ifdef isgreaterequal
#error isgreaterequal defined
#endif
    static_assert((std::is_same<decltype(std::isgreaterequal((float)0, (float)0)), bool>::value), "");
    static_assert((std::is_same<decltype(std::isgreaterequal((float)0, (double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(std::isgreaterequal((float)0, (long double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(std::isgreaterequal((double)0, (float)0)), bool>::value), "");
    static_assert((std::is_same<decltype(std::isgreaterequal((double)0, (double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(std::isgreaterequal(0, (double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(std::isgreaterequal((double)0, (long double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(std::isgreaterequal((long double)0, (float)0)), bool>::value), "");
    static_assert((std::is_same<decltype(std::isgreaterequal((long double)0, (double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(std::isgreaterequal((long double)0, (long double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(isgreaterequal(Ambiguous(), Ambiguous())), Ambiguous>::value), "");
    assert(std::isgreaterequal(-1.0, 0.F) == false);
}

void test_isinf()
{
#ifdef isinf
#error isinf defined
#endif
    static_assert((std::is_same<decltype(std::isinf((float)0)), bool>::value), "");

    typedef decltype(std::isinf((double)0)) DoubleRetType;
#if !defined(__linux__) || defined(__clang__)
    static_assert((std::is_same<DoubleRetType, bool>::value), "");
#else
    // GLIBC < 2.23 defines 'isinf(double)' with a return type of 'int' in
    // all C++ dialects. The test should tolerate this when libc++ can't work
    // around it.
    // See: https://sourceware.org/bugzilla/show_bug.cgi?id=19439
    static_assert((std::is_same<DoubleRetType, bool>::value
                || std::is_same<DoubleRetType, int>::value), "");
#endif

    static_assert((std::is_same<decltype(std::isinf(0)), bool>::value), "");
    static_assert((std::is_same<decltype(std::isinf((long double)0)), bool>::value), "");
    assert(std::isinf(-1.0) == false);
    assert(std::isinf(0) == false);
    assert(std::isinf(1) == false);
    assert(std::isinf(-1) == false);
    assert(std::isinf(std::numeric_limits<int>::max()) == false);
    assert(std::isinf(std::numeric_limits<int>::min()) == false);
}

void test_isless()
{
#ifdef isless
#error isless defined
#endif
    static_assert((std::is_same<decltype(std::isless((float)0, (float)0)), bool>::value), "");
    static_assert((std::is_same<decltype(std::isless((float)0, (double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(std::isless((float)0, (long double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(std::isless((double)0, (float)0)), bool>::value), "");
    static_assert((std::is_same<decltype(std::isless((double)0, (double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(std::isless(0, (double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(std::isless((double)0, (long double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(std::isless((long double)0, (float)0)), bool>::value), "");
    static_assert((std::is_same<decltype(std::isless((long double)0, (double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(std::isless((long double)0, (long double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(isless(Ambiguous(), Ambiguous())), Ambiguous>::value), "");
    assert(std::isless(-1.0, 0.F) == true);
}

void test_islessequal()
{
#ifdef islessequal
#error islessequal defined
#endif
    static_assert((std::is_same<decltype(std::islessequal((float)0, (float)0)), bool>::value), "");
    static_assert((std::is_same<decltype(std::islessequal((float)0, (double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(std::islessequal((float)0, (long double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(std::islessequal((double)0, (float)0)), bool>::value), "");
    static_assert((std::is_same<decltype(std::islessequal((double)0, (double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(std::islessequal(0, (double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(std::islessequal((double)0, (long double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(std::islessequal((long double)0, (float)0)), bool>::value), "");
    static_assert((std::is_same<decltype(std::islessequal((long double)0, (double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(std::islessequal((long double)0, (long double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(islessequal(Ambiguous(), Ambiguous())), Ambiguous>::value), "");
    assert(std::islessequal(-1.0, 0.F) == true);
}

void test_islessgreater()
{
#ifdef islessgreater
#error islessgreater defined
#endif
    static_assert((std::is_same<decltype(std::islessgreater((float)0, (float)0)), bool>::value), "");
    static_assert((std::is_same<decltype(std::islessgreater((float)0, (double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(std::islessgreater((float)0, (long double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(std::islessgreater((double)0, (float)0)), bool>::value), "");
    static_assert((std::is_same<decltype(std::islessgreater((double)0, (double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(std::islessgreater(0, (double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(std::islessgreater((double)0, (long double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(std::islessgreater((long double)0, (float)0)), bool>::value), "");
    static_assert((std::is_same<decltype(std::islessgreater((long double)0, (double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(std::islessgreater((long double)0, (long double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(islessgreater(Ambiguous(), Ambiguous())), Ambiguous>::value), "");
    assert(std::islessgreater(-1.0, 0.F) == true);
}

void test_isnan()
{
#ifdef isnan
#error isnan defined
#endif
    static_assert((std::is_same<decltype(std::isnan((float)0)), bool>::value), "");

    typedef decltype(std::isnan((double)0)) DoubleRetType;
#if !defined(__linux__) || defined(__clang__)
    static_assert((std::is_same<DoubleRetType, bool>::value), "");
#else
    // GLIBC < 2.23 defines 'isinf(double)' with a return type of 'int' in
    // all C++ dialects. The test should tolerate this when libc++ can't work
    // around it.
    // See: https://sourceware.org/bugzilla/show_bug.cgi?id=19439
    static_assert((std::is_same<DoubleRetType, bool>::value
                || std::is_same<DoubleRetType, int>::value), "");
#endif

    static_assert((std::is_same<decltype(std::isnan(0)), bool>::value), "");
    static_assert((std::is_same<decltype(std::isnan((long double)0)), bool>::value), "");
    assert(std::isnan(-1.0) == false);
    assert(std::isnan(0) == false);
    assert(std::isnan(1) == false);
    assert(std::isnan(-1) == false);
    assert(std::isnan(std::numeric_limits<int>::max()) == false);
    assert(std::isnan(std::numeric_limits<int>::min()) == false);
}

void test_isunordered()
{
#ifdef isunordered
#error isunordered defined
#endif
    static_assert((std::is_same<decltype(std::isunordered((float)0, (float)0)), bool>::value), "");
    static_assert((std::is_same<decltype(std::isunordered((float)0, (double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(std::isunordered((float)0, (long double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(std::isunordered((double)0, (float)0)), bool>::value), "");
    static_assert((std::is_same<decltype(std::isunordered((double)0, (double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(std::isunordered(0, (double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(std::isunordered((double)0, (long double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(std::isunordered((long double)0, (float)0)), bool>::value), "");
    static_assert((std::is_same<decltype(std::isunordered((long double)0, (double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(std::isunordered((long double)0, (long double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(isunordered(Ambiguous(), Ambiguous())), Ambiguous>::value), "");
    assert(std::isunordered(-1.0, 0.F) == false);
}

void test_acosh()
{
    static_assert((std::is_same<decltype(std::acosh((float)0)), float>::value), "");
    static_assert((std::is_same<decltype(std::acosh((bool)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::acosh((unsigned short)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::acosh((int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::acosh((unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::acosh((long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::acosh((unsigned long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::acosh((long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::acosh((unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::acosh((double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::acosh((long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::acoshf(0)), float>::value), "");
    static_assert((std::is_same<decltype(std::acoshl(0)), long double>::value), "");
    static_assert((std::is_same<decltype(acosh(Ambiguous())), Ambiguous>::value), "");
    assert(std::acosh(1) == 0);
}

void test_asinh()
{
    static_assert((std::is_same<decltype(std::asinh((float)0)), float>::value), "");
    static_assert((std::is_same<decltype(std::asinh((bool)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::asinh((unsigned short)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::asinh((int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::asinh((unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::asinh((long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::asinh((unsigned long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::asinh((long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::asinh((unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::asinh((double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::asinh((long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::asinhf(0)), float>::value), "");
    static_assert((std::is_same<decltype(std::asinhl(0)), long double>::value), "");
    static_assert((std::is_same<decltype(asinh(Ambiguous())), Ambiguous>::value), "");
    assert(std::asinh(0) == 0);
}

void test_atanh()
{
    static_assert((std::is_same<decltype(std::atanh((float)0)), float>::value), "");
    static_assert((std::is_same<decltype(std::atanh((bool)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::atanh((unsigned short)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::atanh((int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::atanh((unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::atanh((long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::atanh((unsigned long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::atanh((long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::atanh((unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::atanh((double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::atanh((long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::atanhf(0)), float>::value), "");
    static_assert((std::is_same<decltype(std::atanhl(0)), long double>::value), "");
    static_assert((std::is_same<decltype(atanh(Ambiguous())), Ambiguous>::value), "");
    assert(std::atanh(0) == 0);
}

void test_cbrt()
{
    static_assert((std::is_same<decltype(std::cbrt((float)0)), float>::value), "");
    static_assert((std::is_same<decltype(std::cbrt((bool)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::cbrt((unsigned short)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::cbrt((int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::cbrt((unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::cbrt((long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::cbrt((unsigned long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::cbrt((long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::cbrt((unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::cbrt((double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::cbrt((long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::cbrtf(0)), float>::value), "");
    static_assert((std::is_same<decltype(std::cbrtl(0)), long double>::value), "");
    static_assert((std::is_same<decltype(cbrt(Ambiguous())), Ambiguous>::value), "");
    assert(truncate_fp(std::cbrt(1)) == 1);
}

void test_copysign()
{
    static_assert((std::is_same<decltype(std::copysign((float)0, (float)0)), float>::value), "");
    static_assert((std::is_same<decltype(std::copysign((bool)0, (float)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::copysign((unsigned short)0, (double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::copysign((int)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::copysign((float)0, (unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::copysign((double)0, (long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::copysign((long double)0, (unsigned long)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::copysign((int)0, (long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::copysign((int)0, (unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::copysign((double)0, (double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::copysign((long double)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::copysign((float)0, (double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::copysign((float)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::copysign((double)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::copysignf(0,0)), float>::value), "");
    static_assert((std::is_same<decltype(std::copysignl(0,0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::copysign((int)0, (int)0)), double>::value), "");
    static_assert((std::is_same<decltype(copysign(Ambiguous(), Ambiguous())), Ambiguous>::value), "");
    assert(std::copysign(1,1) == 1);
}

void test_erf()
{
    static_assert((std::is_same<decltype(std::erf((float)0)), float>::value), "");
    static_assert((std::is_same<decltype(std::erf((bool)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::erf((unsigned short)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::erf((int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::erf((unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::erf((long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::erf((unsigned long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::erf((long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::erf((unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::erf((double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::erf((long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::erff(0)), float>::value), "");
    static_assert((std::is_same<decltype(std::erfl(0)), long double>::value), "");
    static_assert((std::is_same<decltype(erf(Ambiguous())), Ambiguous>::value), "");
    assert(std::erf(0) == 0);
}

void test_erfc()
{
    static_assert((std::is_same<decltype(std::erfc((float)0)), float>::value), "");
    static_assert((std::is_same<decltype(std::erfc((bool)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::erfc((unsigned short)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::erfc((int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::erfc((unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::erfc((long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::erfc((unsigned long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::erfc((long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::erfc((unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::erfc((double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::erfc((long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::erfcf(0)), float>::value), "");
    static_assert((std::is_same<decltype(std::erfcl(0)), long double>::value), "");
    static_assert((std::is_same<decltype(erfc(Ambiguous())), Ambiguous>::value), "");
    assert(std::erfc(0) == 1);
}

void test_exp2()
{
    static_assert((std::is_same<decltype(std::exp2((float)0)), float>::value), "");
    static_assert((std::is_same<decltype(std::exp2((bool)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::exp2((unsigned short)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::exp2((int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::exp2((unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::exp2((long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::exp2((unsigned long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::exp2((long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::exp2((unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::exp2((double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::exp2((long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::exp2f(0)), float>::value), "");
    static_assert((std::is_same<decltype(std::exp2l(0)), long double>::value), "");
    static_assert((std::is_same<decltype(exp2(Ambiguous())), Ambiguous>::value), "");
    assert(std::exp2(1) == 2);
}

void test_expm1()
{
    static_assert((std::is_same<decltype(std::expm1((float)0)), float>::value), "");
    static_assert((std::is_same<decltype(std::expm1((bool)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::expm1((unsigned short)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::expm1((int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::expm1((unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::expm1((long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::expm1((unsigned long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::expm1((long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::expm1((unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::expm1((double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::expm1((long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::expm1f(0)), float>::value), "");
    static_assert((std::is_same<decltype(std::expm1l(0)), long double>::value), "");
    static_assert((std::is_same<decltype(expm1(Ambiguous())), Ambiguous>::value), "");
    assert(std::expm1(0) == 0);
}

void test_fdim()
{
    static_assert((std::is_same<decltype(std::fdim((float)0, (float)0)), float>::value), "");
    static_assert((std::is_same<decltype(std::fdim((bool)0, (float)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::fdim((unsigned short)0, (double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::fdim((int)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::fdim((float)0, (unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::fdim((double)0, (long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::fdim((long double)0, (unsigned long)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::fdim((int)0, (long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::fdim((int)0, (unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::fdim((double)0, (double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::fdim((long double)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::fdim((float)0, (double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::fdim((float)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::fdim((double)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::fdimf(0,0)), float>::value), "");
    static_assert((std::is_same<decltype(std::fdiml(0,0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::fdim((int)0, (int)0)), double>::value), "");
    static_assert((std::is_same<decltype(fdim(Ambiguous(), Ambiguous())), Ambiguous>::value), "");
    assert(std::fdim(1,0) == 1);
}

void test_fma()
{
    static_assert((std::is_same<decltype(std::fma((bool)0, (float)0, (float)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::fma((char)0, (float)0, (float)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::fma((unsigned)0, (float)0, (float)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::fma((float)0, (int)0, (float)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::fma((float)0, (long)0, (float)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::fma((float)0, (float)0, (unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::fma((float)0, (float)0, (double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::fma((float)0, (float)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::fma((float)0, (float)0, (float)0)), float>::value), "");

    static_assert((std::is_same<decltype(std::fma((bool)0, (double)0, (double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::fma((char)0, (double)0, (double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::fma((unsigned)0, (double)0, (double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::fma((double)0, (int)0, (double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::fma((double)0, (long)0, (double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::fma((double)0, (double)0, (unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::fma((double)0, (double)0, (float)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::fma((double)0, (double)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::fma((double)0, (double)0,  (double)0)), double>::value), "");

    static_assert((std::is_same<decltype(std::fma((bool)0, (long double)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::fma((char)0, (long double)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::fma((unsigned)0, (long double)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::fma((long double)0, (int)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::fma((long double)0, (long)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::fma((long double)0, (long double)0, (unsigned long long)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::fma((long double)0, (long double)0, (float)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::fma((double)0, (long double)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::fma((long double)0, (long double)0, (long double)0)), long double>::value), "");

    static_assert((std::is_same<decltype(std::fmaf(0,0,0)), float>::value), "");
    static_assert((std::is_same<decltype(std::fmal(0,0,0)), long double>::value), "");
    static_assert((std::is_same<decltype(fma(Ambiguous(), Ambiguous(), Ambiguous())), Ambiguous>::value), "");
    assert(std::fma(1,1,1) == 2);
}

void test_fmax()
{
    static_assert((std::is_same<decltype(std::fmax((float)0, (float)0)), float>::value), "");
    static_assert((std::is_same<decltype(std::fmax((bool)0, (float)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::fmax((unsigned short)0, (double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::fmax((int)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::fmax((float)0, (unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::fmax((double)0, (long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::fmax((long double)0, (unsigned long)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::fmax((int)0, (long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::fmax((int)0, (unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::fmax((double)0, (double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::fmax((long double)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::fmax((float)0, (double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::fmax((float)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::fmax((double)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::fmaxf(0,0)), float>::value), "");
    static_assert((std::is_same<decltype(std::fmaxl(0,0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::fmax((int)0, (int)0)), double>::value), "");
    static_assert((std::is_same<decltype(fmax(Ambiguous(), Ambiguous())), Ambiguous>::value), "");
    assert(std::fmax(1,0) == 1);
}

void test_fmin()
{
    static_assert((std::is_same<decltype(std::fmin((float)0, (float)0)), float>::value), "");
    static_assert((std::is_same<decltype(std::fmin((bool)0, (float)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::fmin((unsigned short)0, (double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::fmin((int)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::fmin((float)0, (unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::fmin((double)0, (long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::fmin((long double)0, (unsigned long)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::fmin((int)0, (long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::fmin((int)0, (unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::fmin((double)0, (double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::fmin((long double)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::fmin((float)0, (double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::fmin((float)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::fmin((double)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::fminf(0,0)), float>::value), "");
    static_assert((std::is_same<decltype(std::fminl(0,0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::fmin((int)0, (int)0)), double>::value), "");
    static_assert((std::is_same<decltype(fmin(Ambiguous(), Ambiguous())), Ambiguous>::value), "");
    assert(std::fmin(1,0) == 0);
}

void test_hypot()
{
    static_assert((std::is_same<decltype(std::hypot((float)0, (float)0)), float>::value), "");
    static_assert((std::is_same<decltype(std::hypot((bool)0, (float)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::hypot((unsigned short)0, (double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::hypot((int)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::hypot((float)0, (unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::hypot((double)0, (long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::hypot((long double)0, (unsigned long)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::hypot((int)0, (long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::hypot((int)0, (unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::hypot((double)0, (double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::hypot((long double)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::hypot((float)0, (double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::hypot((float)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::hypot((double)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::hypotf(0,0)), float>::value), "");
    static_assert((std::is_same<decltype(std::hypotl(0,0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::hypot((int)0, (int)0)), double>::value), "");
    static_assert((std::is_same<decltype(hypot(Ambiguous(), Ambiguous())), Ambiguous>::value), "");
    assert(std::hypot(3,4) == 5);

#if TEST_STD_VER > 14
    static_assert((std::is_same<decltype(std::hypot((float)0, (float)0, (float)0)), float>::value), "");
    static_assert((std::is_same<decltype(std::hypot((float)0, (bool)0, (float)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::hypot((float)0, (unsigned short)0, (double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::hypot((float)0, (int)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::hypot((float)0, (unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::hypot((float)0, (double)0, (long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::hypot((float)0, (long double)0, (unsigned long)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::hypot((float)0, (int)0, (long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::hypot((float)0, (int)0, (unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::hypot((float)0, (double)0, (double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::hypot((float)0, (long double)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::hypot((float)0, (float)0, (double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::hypot((float)0, (float)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::hypot((float)0, (double)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::hypot((int)0, (int)0, (int)0)), double>::value), "");
    static_assert((std::is_same<decltype(hypot(Ambiguous(), Ambiguous(), Ambiguous())), Ambiguous>::value), "");

    assert(std::hypot(2,3,6) == 7);
    assert(std::hypot(1,4,8) == 9);
#endif
}

void test_ilogb()
{
    static_assert((std::is_same<decltype(std::ilogb((float)0)), int>::value), "");
    static_assert((std::is_same<decltype(std::ilogb((bool)0)), int>::value), "");
    static_assert((std::is_same<decltype(std::ilogb((unsigned short)0)), int>::value), "");
    static_assert((std::is_same<decltype(std::ilogb((int)0)), int>::value), "");
    static_assert((std::is_same<decltype(std::ilogb((unsigned int)0)), int>::value), "");
    static_assert((std::is_same<decltype(std::ilogb((long)0)), int>::value), "");
    static_assert((std::is_same<decltype(std::ilogb((unsigned long)0)), int>::value), "");
    static_assert((std::is_same<decltype(std::ilogb((long long)0)), int>::value), "");
    static_assert((std::is_same<decltype(std::ilogb((unsigned long long)0)), int>::value), "");
    static_assert((std::is_same<decltype(std::ilogb((double)0)), int>::value), "");
    static_assert((std::is_same<decltype(std::ilogb((long double)0)), int>::value), "");
    static_assert((std::is_same<decltype(std::ilogbf(0)), int>::value), "");
    static_assert((std::is_same<decltype(std::ilogbl(0)), int>::value), "");
    static_assert((std::is_same<decltype(ilogb(Ambiguous())), Ambiguous>::value), "");
    assert(std::ilogb(1) == 0);
}

void test_lgamma()
{
    static_assert((std::is_same<decltype(std::lgamma((float)0)), float>::value), "");
    static_assert((std::is_same<decltype(std::lgamma((bool)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::lgamma((unsigned short)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::lgamma((int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::lgamma((unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::lgamma((long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::lgamma((unsigned long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::lgamma((long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::lgamma((unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::lgamma((double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::lgamma((long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::lgammaf(0)), float>::value), "");
    static_assert((std::is_same<decltype(std::lgammal(0)), long double>::value), "");
    static_assert((std::is_same<decltype(lgamma(Ambiguous())), Ambiguous>::value), "");
    assert(std::lgamma(1) == 0);
}

void test_llrint()
{
    static_assert((std::is_same<decltype(std::llrint((float)0)), long long>::value), "");
    static_assert((std::is_same<decltype(std::llrint((bool)0)), long long>::value), "");
    static_assert((std::is_same<decltype(std::llrint((unsigned short)0)), long long>::value), "");
    static_assert((std::is_same<decltype(std::llrint((int)0)), long long>::value), "");
    static_assert((std::is_same<decltype(std::llrint((unsigned int)0)), long long>::value), "");
    static_assert((std::is_same<decltype(std::llrint((long)0)), long long>::value), "");
    static_assert((std::is_same<decltype(std::llrint((unsigned long)0)), long long>::value), "");
    static_assert((std::is_same<decltype(std::llrint((long long)0)), long long>::value), "");
    static_assert((std::is_same<decltype(std::llrint((unsigned long long)0)), long long>::value), "");
    static_assert((std::is_same<decltype(std::llrint((double)0)), long long>::value), "");
    static_assert((std::is_same<decltype(std::llrint((long double)0)), long long>::value), "");
    static_assert((std::is_same<decltype(std::llrintf(0)), long long>::value), "");
    static_assert((std::is_same<decltype(std::llrintl(0)), long long>::value), "");
    static_assert((std::is_same<decltype(llrint(Ambiguous())), Ambiguous>::value), "");
    assert(std::llrint(1) == 1LL);
}

void test_llround()
{
    static_assert((std::is_same<decltype(std::llround((float)0)), long long>::value), "");
    static_assert((std::is_same<decltype(std::llround((bool)0)), long long>::value), "");
    static_assert((std::is_same<decltype(std::llround((unsigned short)0)), long long>::value), "");
    static_assert((std::is_same<decltype(std::llround((int)0)), long long>::value), "");
    static_assert((std::is_same<decltype(std::llround((unsigned int)0)), long long>::value), "");
    static_assert((std::is_same<decltype(std::llround((long)0)), long long>::value), "");
    static_assert((std::is_same<decltype(std::llround((unsigned long)0)), long long>::value), "");
    static_assert((std::is_same<decltype(std::llround((long long)0)), long long>::value), "");
    static_assert((std::is_same<decltype(std::llround((unsigned long long)0)), long long>::value), "");
    static_assert((std::is_same<decltype(std::llround((double)0)), long long>::value), "");
    static_assert((std::is_same<decltype(std::llround((long double)0)), long long>::value), "");
    static_assert((std::is_same<decltype(std::llroundf(0)), long long>::value), "");
    static_assert((std::is_same<decltype(std::llroundl(0)), long long>::value), "");
    static_assert((std::is_same<decltype(llround(Ambiguous())), Ambiguous>::value), "");
    assert(std::llround(1) == 1LL);
}

void test_log1p()
{
    static_assert((std::is_same<decltype(std::log1p((float)0)), float>::value), "");
    static_assert((std::is_same<decltype(std::log1p((bool)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::log1p((unsigned short)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::log1p((int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::log1p((unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::log1p((long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::log1p((unsigned long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::log1p((long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::log1p((unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::log1p((double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::log1p((long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::log1pf(0)), float>::value), "");
    static_assert((std::is_same<decltype(std::log1pl(0)), long double>::value), "");
    static_assert((std::is_same<decltype(log1p(Ambiguous())), Ambiguous>::value), "");
    assert(std::log1p(0) == 0);
}

void test_log2()
{
    static_assert((std::is_same<decltype(std::log2((float)0)), float>::value), "");
    static_assert((std::is_same<decltype(std::log2((bool)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::log2((unsigned short)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::log2((int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::log2((unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::log2((long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::log2((unsigned long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::log2((long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::log2((unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::log2((double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::log2((long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::log2f(0)), float>::value), "");
    static_assert((std::is_same<decltype(std::log2l(0)), long double>::value), "");
    static_assert((std::is_same<decltype(log2(Ambiguous())), Ambiguous>::value), "");
    assert(std::log2(1) == 0);
}

void test_logb()
{
    static_assert((std::is_same<decltype(std::logb((float)0)), float>::value), "");
    static_assert((std::is_same<decltype(std::logb((bool)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::logb((unsigned short)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::logb((int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::logb((unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::logb((long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::logb((unsigned long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::logb((long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::logb((unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::logb((double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::logb((long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::logbf(0)), float>::value), "");
    static_assert((std::is_same<decltype(std::logbl(0)), long double>::value), "");
    static_assert((std::is_same<decltype(logb(Ambiguous())), Ambiguous>::value), "");
    assert(std::logb(1) == 0);
}

void test_lrint()
{
    static_assert((std::is_same<decltype(std::lrint((float)0)), long>::value), "");
    static_assert((std::is_same<decltype(std::lrint((bool)0)), long>::value), "");
    static_assert((std::is_same<decltype(std::lrint((unsigned short)0)), long>::value), "");
    static_assert((std::is_same<decltype(std::lrint((int)0)), long>::value), "");
    static_assert((std::is_same<decltype(std::lrint((unsigned int)0)), long>::value), "");
    static_assert((std::is_same<decltype(std::lrint((long)0)), long>::value), "");
    static_assert((std::is_same<decltype(std::lrint((unsigned long)0)), long>::value), "");
    static_assert((std::is_same<decltype(std::lrint((long long)0)), long>::value), "");
    static_assert((std::is_same<decltype(std::lrint((unsigned long long)0)), long>::value), "");
    static_assert((std::is_same<decltype(std::lrint((double)0)), long>::value), "");
    static_assert((std::is_same<decltype(std::lrint((long double)0)), long>::value), "");
    static_assert((std::is_same<decltype(std::lrintf(0)), long>::value), "");
    static_assert((std::is_same<decltype(std::lrintl(0)), long>::value), "");
    static_assert((std::is_same<decltype(lrint(Ambiguous())), Ambiguous>::value), "");
    assert(std::lrint(1) == 1L);
}

void test_lround()
{
    static_assert((std::is_same<decltype(std::lround((float)0)), long>::value), "");
    static_assert((std::is_same<decltype(std::lround((bool)0)), long>::value), "");
    static_assert((std::is_same<decltype(std::lround((unsigned short)0)), long>::value), "");
    static_assert((std::is_same<decltype(std::lround((int)0)), long>::value), "");
    static_assert((std::is_same<decltype(std::lround((unsigned int)0)), long>::value), "");
    static_assert((std::is_same<decltype(std::lround((long)0)), long>::value), "");
    static_assert((std::is_same<decltype(std::lround((unsigned long)0)), long>::value), "");
    static_assert((std::is_same<decltype(std::lround((long long)0)), long>::value), "");
    static_assert((std::is_same<decltype(std::lround((unsigned long long)0)), long>::value), "");
    static_assert((std::is_same<decltype(std::lround((double)0)), long>::value), "");
    static_assert((std::is_same<decltype(std::lround((long double)0)), long>::value), "");
    static_assert((std::is_same<decltype(std::lroundf(0)), long>::value), "");
    static_assert((std::is_same<decltype(std::lroundl(0)), long>::value), "");
    static_assert((std::is_same<decltype(lround(Ambiguous())), Ambiguous>::value), "");
    assert(std::lround(1) == 1L);
}

void test_nan()
{
    static_assert((std::is_same<decltype(std::nan("")), double>::value), "");
    static_assert((std::is_same<decltype(std::nanf("")), float>::value), "");
    static_assert((std::is_same<decltype(std::nanl("")), long double>::value), "");
}

void test_nearbyint()
{
    static_assert((std::is_same<decltype(std::nearbyint((float)0)), float>::value), "");
    static_assert((std::is_same<decltype(std::nearbyint((bool)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::nearbyint((unsigned short)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::nearbyint((int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::nearbyint((unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::nearbyint((long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::nearbyint((unsigned long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::nearbyint((long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::nearbyint((unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::nearbyint((double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::nearbyint((long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::nearbyintf(0)), float>::value), "");
    static_assert((std::is_same<decltype(std::nearbyintl(0)), long double>::value), "");
    static_assert((std::is_same<decltype(nearbyint(Ambiguous())), Ambiguous>::value), "");
    assert(std::nearbyint(1) == 1);
}

void test_nextafter()
{
    static_assert((std::is_same<decltype(std::nextafter((float)0, (float)0)), float>::value), "");
    static_assert((std::is_same<decltype(std::nextafter((bool)0, (float)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::nextafter((unsigned short)0, (double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::nextafter((int)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::nextafter((float)0, (unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::nextafter((double)0, (long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::nextafter((long double)0, (unsigned long)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::nextafter((int)0, (long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::nextafter((int)0, (unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::nextafter((double)0, (double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::nextafter((long double)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::nextafter((float)0, (double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::nextafter((float)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::nextafter((double)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::nextafterf(0,0)), float>::value), "");
    static_assert((std::is_same<decltype(std::nextafterl(0,0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::nextafter((int)0, (int)0)), double>::value), "");
    static_assert((std::is_same<decltype(nextafter(Ambiguous(), Ambiguous())), Ambiguous>::value), "");
    assert(std::nextafter(0,1) == hexfloat<double>(0x1, 0, -1074));
}

void test_nexttoward()
{
    static_assert((std::is_same<decltype(std::nexttoward((float)0, (long double)0)), float>::value), "");
    static_assert((std::is_same<decltype(std::nexttoward((bool)0, (long double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::nexttoward((unsigned short)0, (long double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::nexttoward((int)0, (long double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::nexttoward((unsigned int)0, (long double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::nexttoward((long)0, (long double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::nexttoward((unsigned long)0, (long double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::nexttoward((long long)0, (long double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::nexttoward((unsigned long long)0, (long double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::nexttoward((double)0, (long double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::nexttoward((long double)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::nexttowardf(0, (long double)0)), float>::value), "");
    static_assert((std::is_same<decltype(std::nexttowardl(0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(nexttoward(Ambiguous(), Ambiguous())), Ambiguous>::value), "");
    assert(std::nexttoward(0, 1) == hexfloat<double>(0x1, 0, -1074));
}

void test_remainder()
{
    static_assert((std::is_same<decltype(std::remainder((float)0, (float)0)), float>::value), "");
    static_assert((std::is_same<decltype(std::remainder((bool)0, (float)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::remainder((unsigned short)0, (double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::remainder((int)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::remainder((float)0, (unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::remainder((double)0, (long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::remainder((long double)0, (unsigned long)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::remainder((int)0, (long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::remainder((int)0, (unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::remainder((double)0, (double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::remainder((long double)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::remainder((float)0, (double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::remainder((float)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::remainder((double)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::remainderf(0,0)), float>::value), "");
    static_assert((std::is_same<decltype(std::remainderl(0,0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::remainder((int)0, (int)0)), double>::value), "");
    static_assert((std::is_same<decltype(remainder(Ambiguous(), Ambiguous())), Ambiguous>::value), "");
    assert(std::remainder(0.5,1) == 0.5);
}

void test_remquo()
{
    int ip;
    static_assert((std::is_same<decltype(std::remquo((float)0, (float)0, &ip)), float>::value), "");
    static_assert((std::is_same<decltype(std::remquo((bool)0, (float)0, &ip)), double>::value), "");
    static_assert((std::is_same<decltype(std::remquo((unsigned short)0, (double)0, &ip)), double>::value), "");
    static_assert((std::is_same<decltype(std::remquo((int)0, (long double)0, &ip)), long double>::value), "");
    static_assert((std::is_same<decltype(std::remquo((float)0, (unsigned int)0, &ip)), double>::value), "");
    static_assert((std::is_same<decltype(std::remquo((double)0, (long)0, &ip)), double>::value), "");
    static_assert((std::is_same<decltype(std::remquo((long double)0, (unsigned long)0, &ip)), long double>::value), "");
    static_assert((std::is_same<decltype(std::remquo((int)0, (long long)0, &ip)), double>::value), "");
    static_assert((std::is_same<decltype(std::remquo((int)0, (unsigned long long)0, &ip)), double>::value), "");
    static_assert((std::is_same<decltype(std::remquo((double)0, (double)0, &ip)), double>::value), "");
    static_assert((std::is_same<decltype(std::remquo((long double)0, (long double)0, &ip)), long double>::value), "");
    static_assert((std::is_same<decltype(std::remquo((float)0, (double)0, &ip)), double>::value), "");
    static_assert((std::is_same<decltype(std::remquo((float)0, (long double)0, &ip)), long double>::value), "");
    static_assert((std::is_same<decltype(std::remquo((double)0, (long double)0, &ip)), long double>::value), "");
    static_assert((std::is_same<decltype(std::remquof(0,0, &ip)), float>::value), "");
    static_assert((std::is_same<decltype(std::remquol(0,0, &ip)), long double>::value), "");
    static_assert((std::is_same<decltype(std::remquo((int)0, (int)0, &ip)), double>::value), "");
    static_assert((std::is_same<decltype(remquo(Ambiguous(), Ambiguous(), &ip)), Ambiguous>::value), "");
    assert(std::remquo(0.5,1, &ip) == 0.5);
}

void test_rint()
{
    static_assert((std::is_same<decltype(std::rint((float)0)), float>::value), "");
    static_assert((std::is_same<decltype(std::rint((bool)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::rint((unsigned short)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::rint((int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::rint((unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::rint((long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::rint((unsigned long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::rint((long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::rint((unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::rint((double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::rint((long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::rintf(0)), float>::value), "");
    static_assert((std::is_same<decltype(std::rintl(0)), long double>::value), "");
    static_assert((std::is_same<decltype(rint(Ambiguous())), Ambiguous>::value), "");
    assert(std::rint(1) == 1);
}

void test_round()
{
    static_assert((std::is_same<decltype(std::round((float)0)), float>::value), "");
    static_assert((std::is_same<decltype(std::round((bool)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::round((unsigned short)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::round((int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::round((unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::round((long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::round((unsigned long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::round((long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::round((unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::round((double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::round((long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::roundf(0)), float>::value), "");
    static_assert((std::is_same<decltype(std::roundl(0)), long double>::value), "");
    static_assert((std::is_same<decltype(round(Ambiguous())), Ambiguous>::value), "");
    assert(std::round(1) == 1);
}

void test_scalbln()
{
    static_assert((std::is_same<decltype(std::scalbln((float)0, (long)0)), float>::value), "");
    static_assert((std::is_same<decltype(std::scalbln((bool)0, (long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::scalbln((unsigned short)0, (long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::scalbln((int)0, (long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::scalbln((unsigned int)0, (long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::scalbln((long)0, (long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::scalbln((unsigned long)0, (long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::scalbln((long long)0, (long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::scalbln((unsigned long long)0, (long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::scalbln((double)0, (long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::scalbln((long double)0, (long)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::scalblnf(0, (long)0)), float>::value), "");
    static_assert((std::is_same<decltype(std::scalblnl(0, (long)0)), long double>::value), "");
    static_assert((std::is_same<decltype(scalbln(Ambiguous(), Ambiguous())), Ambiguous>::value), "");
    assert(std::scalbln(1, 1) == 2);
}

void test_scalbn()
{
    static_assert((std::is_same<decltype(std::scalbn((float)0, (int)0)), float>::value), "");
    static_assert((std::is_same<decltype(std::scalbn((bool)0, (int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::scalbn((unsigned short)0, (int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::scalbn((int)0, (int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::scalbn((unsigned int)0, (int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::scalbn((long)0, (int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::scalbn((unsigned long)0, (int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::scalbn((long long)0, (int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::scalbn((unsigned long long)0, (int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::scalbn((double)0, (int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::scalbn((long double)0, (int)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::scalbnf(0, (int)0)), float>::value), "");
    static_assert((std::is_same<decltype(std::scalbnl(0, (int)0)), long double>::value), "");
    static_assert((std::is_same<decltype(scalbn(Ambiguous(), Ambiguous())), Ambiguous>::value), "");
    assert(std::scalbn(1, 1) == 2);
}

void test_tgamma()
{
    static_assert((std::is_same<decltype(std::tgamma((float)0)), float>::value), "");
    static_assert((std::is_same<decltype(std::tgamma((bool)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::tgamma((unsigned short)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::tgamma((int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::tgamma((unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::tgamma((long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::tgamma((unsigned long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::tgamma((long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::tgamma((unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::tgamma((double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::tgamma((long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::tgammaf(0)), float>::value), "");
    static_assert((std::is_same<decltype(std::tgammal(0)), long double>::value), "");
    static_assert((std::is_same<decltype(tgamma(Ambiguous())), Ambiguous>::value), "");
    assert(std::tgamma(1) == 1);
}

void test_trunc()
{
    static_assert((std::is_same<decltype(std::trunc((float)0)), float>::value), "");
    static_assert((std::is_same<decltype(std::trunc((bool)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::trunc((unsigned short)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::trunc((int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::trunc((unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::trunc((long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::trunc((unsigned long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::trunc((long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::trunc((unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::trunc((double)0)), double>::value), "");
    static_assert((std::is_same<decltype(std::trunc((long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(std::truncf(0)), float>::value), "");
    static_assert((std::is_same<decltype(std::truncl(0)), long double>::value), "");
    static_assert((std::is_same<decltype(trunc(Ambiguous())), Ambiguous>::value), "");
    assert(std::trunc(1) == 1);
}

int main()
{
    test_abs();
    test_acos();
    test_asin();
    test_atan();
    test_atan2();
    test_ceil();
    test_cos();
    test_cosh();
    test_exp();
    test_fabs();
    test_floor();
    test_fmod();
    test_frexp();
    test_ldexp();
    test_log();
    test_log10();
    test_modf();
    test_pow();
    test_sin();
    test_sinh();
    test_sqrt();
    test_tan();
    test_tanh();
    test_signbit();
    test_fpclassify();
    test_isfinite();
    test_isnormal();
    test_isgreater();
    test_isgreaterequal();
    test_isinf();
    test_isless();
    test_islessequal();
    test_islessgreater();
    test_isnan();
    test_isunordered();
    test_acosh();
    test_asinh();
    test_atanh();
    test_cbrt();
    test_copysign();
    test_erf();
    test_erfc();
    test_exp2();
    test_expm1();
    test_fdim();
    test_fma();
    test_fmax();
    test_fmin();
    test_hypot();
    test_ilogb();
    test_lgamma();
    test_llrint();
    test_llround();
    test_log1p();
    test_log2();
    test_logb();
    test_lrint();
    test_lround();
    test_nan();
    test_nearbyint();
    test_nextafter();
    test_nexttoward();
    test_remainder();
    test_remquo();
    test_rint();
    test_round();
    test_scalbln();
    test_scalbn();
    test_tgamma();
    test_trunc();
}
