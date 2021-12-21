//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <math.h>

#include <math.h>
#include <type_traits>
#include <cassert>

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
    static_assert((std::is_same<decltype(abs((float)0)), float>::value), "");
    static_assert((std::is_same<decltype(abs((double)0)), double>::value), "");
    static_assert((std::is_same<decltype(abs((long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(abs(Ambiguous())), Ambiguous>::value), "");
    assert(abs(-1.) == 1);
}

void test_acos()
{
    static_assert((std::is_same<decltype(acos((float)0)), float>::value), "");
    static_assert((std::is_same<decltype(acos((bool)0)), double>::value), "");
    static_assert((std::is_same<decltype(acos((unsigned short)0)), double>::value), "");
    static_assert((std::is_same<decltype(acos((int)0)), double>::value), "");
    static_assert((std::is_same<decltype(acos((unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(acos((long)0)), double>::value), "");
    static_assert((std::is_same<decltype(acos((unsigned long)0)), double>::value), "");
    static_assert((std::is_same<decltype(acos((long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(acos((unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(acos((double)0)), double>::value), "");
    static_assert((std::is_same<decltype(acos((long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(acosf(0)), float>::value), "");
    static_assert((std::is_same<decltype(acosl(0)), long double>::value), "");
    static_assert((std::is_same<decltype(acos(Ambiguous())), Ambiguous>::value), "");
    assert(acos(1) == 0);
}

void test_asin()
{
    static_assert((std::is_same<decltype(asin((float)0)), float>::value), "");
    static_assert((std::is_same<decltype(asin((bool)0)), double>::value), "");
    static_assert((std::is_same<decltype(asin((unsigned short)0)), double>::value), "");
    static_assert((std::is_same<decltype(asin((int)0)), double>::value), "");
    static_assert((std::is_same<decltype(asin((unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(asin((long)0)), double>::value), "");
    static_assert((std::is_same<decltype(asin((unsigned long)0)), double>::value), "");
    static_assert((std::is_same<decltype(asin((long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(asin((unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(asin((double)0)), double>::value), "");
    static_assert((std::is_same<decltype(asin((long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(asinf(0)), float>::value), "");
    static_assert((std::is_same<decltype(asinl(0)), long double>::value), "");
    static_assert((std::is_same<decltype(asin(Ambiguous())), Ambiguous>::value), "");
    assert(asin(0) == 0);
}

void test_atan()
{
    static_assert((std::is_same<decltype(atan((float)0)), float>::value), "");
    static_assert((std::is_same<decltype(atan((bool)0)), double>::value), "");
    static_assert((std::is_same<decltype(atan((unsigned short)0)), double>::value), "");
    static_assert((std::is_same<decltype(atan((int)0)), double>::value), "");
    static_assert((std::is_same<decltype(atan((unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(atan((long)0)), double>::value), "");
    static_assert((std::is_same<decltype(atan((unsigned long)0)), double>::value), "");
    static_assert((std::is_same<decltype(atan((long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(atan((unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(atan((double)0)), double>::value), "");
    static_assert((std::is_same<decltype(atan((long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(atanf(0)), float>::value), "");
    static_assert((std::is_same<decltype(atanl(0)), long double>::value), "");
    static_assert((std::is_same<decltype(atan(Ambiguous())), Ambiguous>::value), "");
    assert(atan(0) == 0);
}

void test_atan2()
{
    static_assert((std::is_same<decltype(atan2((float)0, (float)0)), float>::value), "");
    static_assert((std::is_same<decltype(atan2((bool)0, (float)0)), double>::value), "");
    static_assert((std::is_same<decltype(atan2((unsigned short)0, (double)0)), double>::value), "");
    static_assert((std::is_same<decltype(atan2((int)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(atan2((float)0, (unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(atan2((double)0, (long)0)), double>::value), "");
    static_assert((std::is_same<decltype(atan2((long double)0, (unsigned long)0)), long double>::value), "");
    static_assert((std::is_same<decltype(atan2((int)0, (long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(atan2((int)0, (unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(atan2((double)0, (double)0)), double>::value), "");
    static_assert((std::is_same<decltype(atan2((long double)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(atan2((float)0, (double)0)), double>::value), "");
    static_assert((std::is_same<decltype(atan2((float)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(atan2((double)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(atan2f(0,0)), float>::value), "");
    static_assert((std::is_same<decltype(atan2l(0,0)), long double>::value), "");
    static_assert((std::is_same<decltype(atan2((int)0, (int)0)), double>::value), "");
    static_assert((std::is_same<decltype(atan2(Ambiguous(), Ambiguous())), Ambiguous>::value), "");
    assert(atan2(0,1) == 0);
}

void test_ceil()
{
    static_assert((std::is_same<decltype(ceil((float)0)), float>::value), "");
    static_assert((std::is_same<decltype(ceil((bool)0)), double>::value), "");
    static_assert((std::is_same<decltype(ceil((unsigned short)0)), double>::value), "");
    static_assert((std::is_same<decltype(ceil((int)0)), double>::value), "");
    static_assert((std::is_same<decltype(ceil((unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(ceil((long)0)), double>::value), "");
    static_assert((std::is_same<decltype(ceil((unsigned long)0)), double>::value), "");
    static_assert((std::is_same<decltype(ceil((long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(ceil((unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(ceil((double)0)), double>::value), "");
    static_assert((std::is_same<decltype(ceil((long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(ceilf(0)), float>::value), "");
    static_assert((std::is_same<decltype(ceill(0)), long double>::value), "");
    static_assert((std::is_same<decltype(ceil(Ambiguous())), Ambiguous>::value), "");
    assert(ceil(0) == 0);
}

void test_cos()
{
    static_assert((std::is_same<decltype(cos((float)0)), float>::value), "");
    static_assert((std::is_same<decltype(cos((bool)0)), double>::value), "");
    static_assert((std::is_same<decltype(cos((unsigned short)0)), double>::value), "");
    static_assert((std::is_same<decltype(cos((int)0)), double>::value), "");
    static_assert((std::is_same<decltype(cos((unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(cos((long)0)), double>::value), "");
    static_assert((std::is_same<decltype(cos((unsigned long)0)), double>::value), "");
    static_assert((std::is_same<decltype(cos((long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(cos((unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(cos((double)0)), double>::value), "");
    static_assert((std::is_same<decltype(cos((long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(cosf(0)), float>::value), "");
    static_assert((std::is_same<decltype(cosl(0)), long double>::value), "");
    static_assert((std::is_same<decltype(cos(Ambiguous())), Ambiguous>::value), "");
    assert(cos(0) == 1);
}

void test_cosh()
{
    static_assert((std::is_same<decltype(cosh((float)0)), float>::value), "");
    static_assert((std::is_same<decltype(cosh((bool)0)), double>::value), "");
    static_assert((std::is_same<decltype(cosh((unsigned short)0)), double>::value), "");
    static_assert((std::is_same<decltype(cosh((int)0)), double>::value), "");
    static_assert((std::is_same<decltype(cosh((unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(cosh((long)0)), double>::value), "");
    static_assert((std::is_same<decltype(cosh((unsigned long)0)), double>::value), "");
    static_assert((std::is_same<decltype(cosh((long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(cosh((unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(cosh((double)0)), double>::value), "");
    static_assert((std::is_same<decltype(cosh((long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(coshf(0)), float>::value), "");
    static_assert((std::is_same<decltype(coshl(0)), long double>::value), "");
    static_assert((std::is_same<decltype(cosh(Ambiguous())), Ambiguous>::value), "");
    assert(cosh(0) == 1);
}

void test_exp()
{
    static_assert((std::is_same<decltype(exp((float)0)), float>::value), "");
    static_assert((std::is_same<decltype(exp((bool)0)), double>::value), "");
    static_assert((std::is_same<decltype(exp((unsigned short)0)), double>::value), "");
    static_assert((std::is_same<decltype(exp((int)0)), double>::value), "");
    static_assert((std::is_same<decltype(exp((unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(exp((long)0)), double>::value), "");
    static_assert((std::is_same<decltype(exp((unsigned long)0)), double>::value), "");
    static_assert((std::is_same<decltype(exp((long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(exp((unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(exp((double)0)), double>::value), "");
    static_assert((std::is_same<decltype(exp((long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(expf(0)), float>::value), "");
    static_assert((std::is_same<decltype(expl(0)), long double>::value), "");
    static_assert((std::is_same<decltype(exp(Ambiguous())), Ambiguous>::value), "");
    assert(exp(0) == 1);
}

void test_fabs()
{
    static_assert((std::is_same<decltype(fabs((float)0)), float>::value), "");
    static_assert((std::is_same<decltype(fabs((bool)0)), double>::value), "");
    static_assert((std::is_same<decltype(fabs((unsigned short)0)), double>::value), "");
    static_assert((std::is_same<decltype(fabs((int)0)), double>::value), "");
    static_assert((std::is_same<decltype(fabs((unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(fabs((long)0)), double>::value), "");
    static_assert((std::is_same<decltype(fabs((unsigned long)0)), double>::value), "");
    static_assert((std::is_same<decltype(fabs((long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(fabs((unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(fabs((double)0)), double>::value), "");
    static_assert((std::is_same<decltype(fabs((long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(fabsf(0.0f)), float>::value), "");
    static_assert((std::is_same<decltype(fabsl(0.0L)), long double>::value), "");
    static_assert((std::is_same<decltype(fabs(Ambiguous())), Ambiguous>::value), "");
    assert(fabs(-1) == 1);
}

void test_floor()
{
    static_assert((std::is_same<decltype(floor((float)0)), float>::value), "");
    static_assert((std::is_same<decltype(floor((bool)0)), double>::value), "");
    static_assert((std::is_same<decltype(floor((unsigned short)0)), double>::value), "");
    static_assert((std::is_same<decltype(floor((int)0)), double>::value), "");
    static_assert((std::is_same<decltype(floor((unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(floor((long)0)), double>::value), "");
    static_assert((std::is_same<decltype(floor((unsigned long)0)), double>::value), "");
    static_assert((std::is_same<decltype(floor((long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(floor((unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(floor((double)0)), double>::value), "");
    static_assert((std::is_same<decltype(floor((long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(floorf(0)), float>::value), "");
    static_assert((std::is_same<decltype(floorl(0)), long double>::value), "");
    static_assert((std::is_same<decltype(floor(Ambiguous())), Ambiguous>::value), "");
    assert(floor(1) == 1);
}

void test_fmod()
{
    static_assert((std::is_same<decltype(fmod((float)0, (float)0)), float>::value), "");
    static_assert((std::is_same<decltype(fmod((bool)0, (float)0)), double>::value), "");
    static_assert((std::is_same<decltype(fmod((unsigned short)0, (double)0)), double>::value), "");
    static_assert((std::is_same<decltype(fmod((int)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(fmod((float)0, (unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(fmod((double)0, (long)0)), double>::value), "");
    static_assert((std::is_same<decltype(fmod((long double)0, (unsigned long)0)), long double>::value), "");
    static_assert((std::is_same<decltype(fmod((int)0, (long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(fmod((int)0, (unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(fmod((double)0, (double)0)), double>::value), "");
    static_assert((std::is_same<decltype(fmod((long double)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(fmod((float)0, (double)0)), double>::value), "");
    static_assert((std::is_same<decltype(fmod((float)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(fmod((double)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(fmodf(0,0)), float>::value), "");
    static_assert((std::is_same<decltype(fmodl(0,0)), long double>::value), "");
    static_assert((std::is_same<decltype(fmod((int)0, (int)0)), double>::value), "");
    static_assert((std::is_same<decltype(fmod(Ambiguous(), Ambiguous())), Ambiguous>::value), "");
    assert(fmod(1.5,1) == .5);
}

void test_frexp()
{
    int ip;
    static_assert((std::is_same<decltype(frexp((float)0, &ip)), float>::value), "");
    static_assert((std::is_same<decltype(frexp((bool)0, &ip)), double>::value), "");
    static_assert((std::is_same<decltype(frexp((unsigned short)0, &ip)), double>::value), "");
    static_assert((std::is_same<decltype(frexp((int)0, &ip)), double>::value), "");
    static_assert((std::is_same<decltype(frexp((unsigned int)0, &ip)), double>::value), "");
    static_assert((std::is_same<decltype(frexp((long)0, &ip)), double>::value), "");
    static_assert((std::is_same<decltype(frexp((unsigned long)0, &ip)), double>::value), "");
    static_assert((std::is_same<decltype(frexp((long long)0, &ip)), double>::value), "");
    static_assert((std::is_same<decltype(frexp((unsigned long long)0, &ip)), double>::value), "");
    static_assert((std::is_same<decltype(frexp((double)0, &ip)), double>::value), "");
    static_assert((std::is_same<decltype(frexp((long double)0, &ip)), long double>::value), "");
    static_assert((std::is_same<decltype(frexpf(0, &ip)), float>::value), "");
    static_assert((std::is_same<decltype(frexpl(0, &ip)), long double>::value), "");
    static_assert((std::is_same<decltype(frexp(Ambiguous(), &ip)), Ambiguous>::value), "");
    assert(frexp(0, &ip) == 0);
}

void test_ldexp()
{
    int ip = 1;
    static_assert((std::is_same<decltype(ldexp((float)0, ip)), float>::value), "");
    static_assert((std::is_same<decltype(ldexp((bool)0, ip)), double>::value), "");
    static_assert((std::is_same<decltype(ldexp((unsigned short)0, ip)), double>::value), "");
    static_assert((std::is_same<decltype(ldexp((int)0, ip)), double>::value), "");
    static_assert((std::is_same<decltype(ldexp((unsigned int)0, ip)), double>::value), "");
    static_assert((std::is_same<decltype(ldexp((long)0, ip)), double>::value), "");
    static_assert((std::is_same<decltype(ldexp((unsigned long)0, ip)), double>::value), "");
    static_assert((std::is_same<decltype(ldexp((long long)0, ip)), double>::value), "");
    static_assert((std::is_same<decltype(ldexp((unsigned long long)0, ip)), double>::value), "");
    static_assert((std::is_same<decltype(ldexp((double)0, ip)), double>::value), "");
    static_assert((std::is_same<decltype(ldexp((long double)0, ip)), long double>::value), "");
    static_assert((std::is_same<decltype(ldexpf(0, ip)), float>::value), "");
    static_assert((std::is_same<decltype(ldexpl(0, ip)), long double>::value), "");
    static_assert((std::is_same<decltype(ldexp(Ambiguous(), ip)), Ambiguous>::value), "");
    assert(ldexp(1, ip) == 2);
}

void test_log()
{
    static_assert((std::is_same<decltype(log((float)0)), float>::value), "");
    static_assert((std::is_same<decltype(log((bool)0)), double>::value), "");
    static_assert((std::is_same<decltype(log((unsigned short)0)), double>::value), "");
    static_assert((std::is_same<decltype(log((int)0)), double>::value), "");
    static_assert((std::is_same<decltype(log((unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(log((long)0)), double>::value), "");
    static_assert((std::is_same<decltype(log((unsigned long)0)), double>::value), "");
    static_assert((std::is_same<decltype(log((long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(log((unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(log((double)0)), double>::value), "");
    static_assert((std::is_same<decltype(log((long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(logf(0)), float>::value), "");
    static_assert((std::is_same<decltype(logl(0)), long double>::value), "");
    static_assert((std::is_same<decltype(log(Ambiguous())), Ambiguous>::value), "");
    assert(log(1) == 0);
}

void test_log10()
{
    static_assert((std::is_same<decltype(log10((float)0)), float>::value), "");
    static_assert((std::is_same<decltype(log10((bool)0)), double>::value), "");
    static_assert((std::is_same<decltype(log10((unsigned short)0)), double>::value), "");
    static_assert((std::is_same<decltype(log10((int)0)), double>::value), "");
    static_assert((std::is_same<decltype(log10((unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(log10((long)0)), double>::value), "");
    static_assert((std::is_same<decltype(log10((unsigned long)0)), double>::value), "");
    static_assert((std::is_same<decltype(log10((long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(log10((unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(log10((double)0)), double>::value), "");
    static_assert((std::is_same<decltype(log10((long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(log10f(0)), float>::value), "");
    static_assert((std::is_same<decltype(log10l(0)), long double>::value), "");
    static_assert((std::is_same<decltype(log10(Ambiguous())), Ambiguous>::value), "");
    assert(log10(1) == 0);
}

void test_modf()
{
    static_assert((std::is_same<decltype(modf((float)0, (float*)0)), float>::value), "");
    static_assert((std::is_same<decltype(modf((double)0, (double*)0)), double>::value), "");
    static_assert((std::is_same<decltype(modf((long double)0, (long double*)0)), long double>::value), "");
    static_assert((std::is_same<decltype(modff(0, (float*)0)), float>::value), "");
    static_assert((std::is_same<decltype(modfl(0, (long double*)0)), long double>::value), "");
    static_assert((std::is_same<decltype(modf(Ambiguous(), (Ambiguous*)0)), Ambiguous>::value), "");
    double i;
    assert(modf(1., &i) == 0);
}

void test_pow()
{
    static_assert((std::is_same<decltype(pow((float)0, (float)0)), float>::value), "");
    static_assert((std::is_same<decltype(pow((bool)0, (float)0)), double>::value), "");
    static_assert((std::is_same<decltype(pow((unsigned short)0, (double)0)), double>::value), "");
    static_assert((std::is_same<decltype(pow((int)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(pow((float)0, (unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(pow((double)0, (long)0)), double>::value), "");
    static_assert((std::is_same<decltype(pow((long double)0, (unsigned long)0)), long double>::value), "");
    static_assert((std::is_same<decltype(pow((int)0, (long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(pow((int)0, (unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(pow((double)0, (double)0)), double>::value), "");
    static_assert((std::is_same<decltype(pow((long double)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(pow((float)0, (double)0)), double>::value), "");
    static_assert((std::is_same<decltype(pow((float)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(pow((double)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(powf(0,0)), float>::value), "");
    static_assert((std::is_same<decltype(powl(0,0)), long double>::value), "");
    static_assert((std::is_same<decltype(pow((int)0, (int)0)), double>::value), "");
//     static_assert((std::is_same<decltype(pow(Value<int>(), (int)0)), double>::value), "");
//     static_assert((std::is_same<decltype(pow(Value<long double>(), (float)0)), long double>::value), "");
//     static_assert((std::is_same<decltype(pow((float) 0, Value<float>())), float>::value), "");
    static_assert((std::is_same<decltype(pow(Ambiguous(), Ambiguous())), Ambiguous>::value), "");
    assert(pow(1,1) == 1);
//     assert(pow(Value<int,1>(), Value<float,1>())  == 1);
//     assert(pow(1.0f, Value<double,1>()) == 1);
//     assert(pow(1.0, Value<int,1>()) == 1);
//     assert(pow(Value<long double,1>(), 1LL) == 1);
}

void test_sin()
{
    static_assert((std::is_same<decltype(sin((float)0)), float>::value), "");
    static_assert((std::is_same<decltype(sin((bool)0)), double>::value), "");
    static_assert((std::is_same<decltype(sin((unsigned short)0)), double>::value), "");
    static_assert((std::is_same<decltype(sin((int)0)), double>::value), "");
    static_assert((std::is_same<decltype(sin((unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(sin((long)0)), double>::value), "");
    static_assert((std::is_same<decltype(sin((unsigned long)0)), double>::value), "");
    static_assert((std::is_same<decltype(sin((long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(sin((unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(sin((double)0)), double>::value), "");
    static_assert((std::is_same<decltype(sin((long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(sinf(0)), float>::value), "");
    static_assert((std::is_same<decltype(sinl(0)), long double>::value), "");
    static_assert((std::is_same<decltype(sin(Ambiguous())), Ambiguous>::value), "");
    assert(sin(0) == 0);
}

void test_sinh()
{
    static_assert((std::is_same<decltype(sinh((float)0)), float>::value), "");
    static_assert((std::is_same<decltype(sinh((bool)0)), double>::value), "");
    static_assert((std::is_same<decltype(sinh((unsigned short)0)), double>::value), "");
    static_assert((std::is_same<decltype(sinh((int)0)), double>::value), "");
    static_assert((std::is_same<decltype(sinh((unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(sinh((long)0)), double>::value), "");
    static_assert((std::is_same<decltype(sinh((unsigned long)0)), double>::value), "");
    static_assert((std::is_same<decltype(sinh((long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(sinh((unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(sinh((double)0)), double>::value), "");
    static_assert((std::is_same<decltype(sinh((long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(sinhf(0)), float>::value), "");
    static_assert((std::is_same<decltype(sinhl(0)), long double>::value), "");
    static_assert((std::is_same<decltype(sinh(Ambiguous())), Ambiguous>::value), "");
    assert(sinh(0) == 0);
}

void test_sqrt()
{
    static_assert((std::is_same<decltype(sqrt((float)0)), float>::value), "");
    static_assert((std::is_same<decltype(sqrt((bool)0)), double>::value), "");
    static_assert((std::is_same<decltype(sqrt((unsigned short)0)), double>::value), "");
    static_assert((std::is_same<decltype(sqrt((int)0)), double>::value), "");
    static_assert((std::is_same<decltype(sqrt((unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(sqrt((long)0)), double>::value), "");
    static_assert((std::is_same<decltype(sqrt((unsigned long)0)), double>::value), "");
    static_assert((std::is_same<decltype(sqrt((long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(sqrt((unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(sqrt((double)0)), double>::value), "");
    static_assert((std::is_same<decltype(sqrt((long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(sqrtf(0)), float>::value), "");
    static_assert((std::is_same<decltype(sqrtl(0)), long double>::value), "");
    static_assert((std::is_same<decltype(sqrt(Ambiguous())), Ambiguous>::value), "");
    assert(sqrt(4) == 2);
}

void test_tan()
{
    static_assert((std::is_same<decltype(tan((float)0)), float>::value), "");
    static_assert((std::is_same<decltype(tan((bool)0)), double>::value), "");
    static_assert((std::is_same<decltype(tan((unsigned short)0)), double>::value), "");
    static_assert((std::is_same<decltype(tan((int)0)), double>::value), "");
    static_assert((std::is_same<decltype(tan((unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(tan((long)0)), double>::value), "");
    static_assert((std::is_same<decltype(tan((unsigned long)0)), double>::value), "");
    static_assert((std::is_same<decltype(tan((long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(tan((unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(tan((double)0)), double>::value), "");
    static_assert((std::is_same<decltype(tan((long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(tanf(0)), float>::value), "");
    static_assert((std::is_same<decltype(tanl(0)), long double>::value), "");
    static_assert((std::is_same<decltype(tan(Ambiguous())), Ambiguous>::value), "");
    assert(tan(0) == 0);
}

void test_tanh()
{
    static_assert((std::is_same<decltype(tanh((float)0)), float>::value), "");
    static_assert((std::is_same<decltype(tanh((bool)0)), double>::value), "");
    static_assert((std::is_same<decltype(tanh((unsigned short)0)), double>::value), "");
    static_assert((std::is_same<decltype(tanh((int)0)), double>::value), "");
    static_assert((std::is_same<decltype(tanh((unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(tanh((long)0)), double>::value), "");
    static_assert((std::is_same<decltype(tanh((unsigned long)0)), double>::value), "");
    static_assert((std::is_same<decltype(tanh((long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(tanh((unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(tanh((double)0)), double>::value), "");
    static_assert((std::is_same<decltype(tanh((long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(tanhf(0)), float>::value), "");
    static_assert((std::is_same<decltype(tanhl(0)), long double>::value), "");
    static_assert((std::is_same<decltype(tanh(Ambiguous())), Ambiguous>::value), "");
    assert(tanh(0) == 0);
}

void test_signbit()
{
#ifdef signbit
#error signbit defined
#endif
    static_assert((std::is_same<decltype(signbit((float)0)), bool>::value), "");
    static_assert((std::is_same<decltype(signbit((double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(signbit(0)), bool>::value), "");
    static_assert((std::is_same<decltype(signbit((long double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(signbit(Ambiguous())), Ambiguous>::value), "");
    assert(signbit(-1.0) == true);
}

void test_fpclassify()
{
#ifdef fpclassify
#error fpclassify defined
#endif
    static_assert((std::is_same<decltype(fpclassify((float)0)), int>::value), "");
    static_assert((std::is_same<decltype(fpclassify((double)0)), int>::value), "");
    static_assert((std::is_same<decltype(fpclassify(0)), int>::value), "");
    static_assert((std::is_same<decltype(fpclassify((long double)0)), int>::value), "");
    static_assert((std::is_same<decltype(fpclassify(Ambiguous())), Ambiguous>::value), "");
    assert(fpclassify(-1.0) == FP_NORMAL);
}

void test_isfinite()
{
#ifdef isfinite
#error isfinite defined
#endif
    static_assert((std::is_same<decltype(isfinite((float)0)), bool>::value), "");
    static_assert((std::is_same<decltype(isfinite((double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(isfinite(0)), bool>::value), "");
    static_assert((std::is_same<decltype(isfinite((long double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(isfinite(Ambiguous())), Ambiguous>::value), "");
    assert(isfinite(-1.0) == true);
}

void test_isnormal()
{
#ifdef isnormal
#error isnormal defined
#endif
    static_assert((std::is_same<decltype(isnormal((float)0)), bool>::value), "");
    static_assert((std::is_same<decltype(isnormal((double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(isnormal(0)), bool>::value), "");
    static_assert((std::is_same<decltype(isnormal((long double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(isnormal(Ambiguous())), Ambiguous>::value), "");
    assert(isnormal(-1.0) == true);
}

void test_isgreater()
{
#ifdef isgreater
#error isgreater defined
#endif
    static_assert((std::is_same<decltype(isgreater((float)0, (float)0)), bool>::value), "");
    static_assert((std::is_same<decltype(isgreater((float)0, (double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(isgreater((float)0, (long double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(isgreater((double)0, (float)0)), bool>::value), "");
    static_assert((std::is_same<decltype(isgreater((double)0, (double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(isgreater(0, (double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(isgreater((double)0, (long double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(isgreater((long double)0, (float)0)), bool>::value), "");
    static_assert((std::is_same<decltype(isgreater((long double)0, (double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(isgreater((long double)0, (long double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(isgreater(Ambiguous(), Ambiguous())), Ambiguous>::value), "");
    assert(isgreater(-1.0, 0.F) == false);
}

void test_isgreaterequal()
{
#ifdef isgreaterequal
#error isgreaterequal defined
#endif
    static_assert((std::is_same<decltype(isgreaterequal((float)0, (float)0)), bool>::value), "");
    static_assert((std::is_same<decltype(isgreaterequal((float)0, (double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(isgreaterequal((float)0, (long double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(isgreaterequal((double)0, (float)0)), bool>::value), "");
    static_assert((std::is_same<decltype(isgreaterequal((double)0, (double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(isgreaterequal(0, (double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(isgreaterequal((double)0, (long double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(isgreaterequal((long double)0, (float)0)), bool>::value), "");
    static_assert((std::is_same<decltype(isgreaterequal((long double)0, (double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(isgreaterequal((long double)0, (long double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(isgreaterequal(Ambiguous(), Ambiguous())), Ambiguous>::value), "");
    assert(isgreaterequal(-1.0, 0.F) == false);
}

void test_isinf()
{
#ifdef isinf
#error isinf defined
#endif
    static_assert((std::is_same<decltype(isinf((float)0)), bool>::value), "");

    typedef decltype(isinf((double)0)) DoubleRetType;
#ifndef __linux__
    static_assert((std::is_same<DoubleRetType, bool>::value), "");
#else
    // GLIBC < 2.26 defines 'isinf(double)' with a return type of 'int' in
    // all C++ dialects. The test should tolerate this.
    // See: https://sourceware.org/bugzilla/show_bug.cgi?id=19439
    static_assert((std::is_same<DoubleRetType, bool>::value
                || std::is_same<DoubleRetType, int>::value), "");
#endif

    static_assert((std::is_same<decltype(isinf(0)), bool>::value), "");
    static_assert((std::is_same<decltype(isinf((long double)0)), bool>::value), "");
    assert(isinf(-1.0) == false);
}

void test_isless()
{
#ifdef isless
#error isless defined
#endif
    static_assert((std::is_same<decltype(isless((float)0, (float)0)), bool>::value), "");
    static_assert((std::is_same<decltype(isless((float)0, (double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(isless((float)0, (long double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(isless((double)0, (float)0)), bool>::value), "");
    static_assert((std::is_same<decltype(isless((double)0, (double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(isless(0, (double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(isless((double)0, (long double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(isless((long double)0, (float)0)), bool>::value), "");
    static_assert((std::is_same<decltype(isless((long double)0, (double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(isless((long double)0, (long double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(isless(Ambiguous(), Ambiguous())), Ambiguous>::value), "");
    assert(isless(-1.0, 0.F) == true);
}

void test_islessequal()
{
#ifdef islessequal
#error islessequal defined
#endif
    static_assert((std::is_same<decltype(islessequal((float)0, (float)0)), bool>::value), "");
    static_assert((std::is_same<decltype(islessequal((float)0, (double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(islessequal((float)0, (long double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(islessequal((double)0, (float)0)), bool>::value), "");
    static_assert((std::is_same<decltype(islessequal((double)0, (double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(islessequal(0, (double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(islessequal((double)0, (long double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(islessequal((long double)0, (float)0)), bool>::value), "");
    static_assert((std::is_same<decltype(islessequal((long double)0, (double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(islessequal((long double)0, (long double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(islessequal(Ambiguous(), Ambiguous())), Ambiguous>::value), "");
    assert(islessequal(-1.0, 0.F) == true);
}

void test_islessgreater()
{
#ifdef islessgreater
#error islessgreater defined
#endif
    static_assert((std::is_same<decltype(islessgreater((float)0, (float)0)), bool>::value), "");
    static_assert((std::is_same<decltype(islessgreater((float)0, (double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(islessgreater((float)0, (long double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(islessgreater((double)0, (float)0)), bool>::value), "");
    static_assert((std::is_same<decltype(islessgreater((double)0, (double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(islessgreater(0, (double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(islessgreater((double)0, (long double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(islessgreater((long double)0, (float)0)), bool>::value), "");
    static_assert((std::is_same<decltype(islessgreater((long double)0, (double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(islessgreater((long double)0, (long double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(islessgreater(Ambiguous(), Ambiguous())), Ambiguous>::value), "");
    assert(islessgreater(-1.0, 0.F) == true);
}

void test_isnan()
{
#ifdef isnan
#error isnan defined
#endif
    static_assert((std::is_same<decltype(isnan((float)0)), bool>::value), "");

    typedef decltype(isnan((double)0)) DoubleRetType;
#ifndef __linux__
    static_assert((std::is_same<DoubleRetType, bool>::value), "");
#else
    // GLIBC < 2.26 defines 'isnan(double)' with a return type of 'int' in
    // all C++ dialects. The test should tolerate this.
    // See: https://sourceware.org/bugzilla/show_bug.cgi?id=19439
    static_assert((std::is_same<DoubleRetType, bool>::value
                || std::is_same<DoubleRetType, int>::value), "");
#endif

    static_assert((std::is_same<decltype(isnan(0)), bool>::value), "");
    static_assert((std::is_same<decltype(isnan((long double)0)), bool>::value), "");
    assert(isnan(-1.0) == false);
}

void test_isunordered()
{
#ifdef isunordered
#error isunordered defined
#endif
    static_assert((std::is_same<decltype(isunordered((float)0, (float)0)), bool>::value), "");
    static_assert((std::is_same<decltype(isunordered((float)0, (double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(isunordered((float)0, (long double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(isunordered((double)0, (float)0)), bool>::value), "");
    static_assert((std::is_same<decltype(isunordered((double)0, (double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(isunordered(0, (double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(isunordered((double)0, (long double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(isunordered((long double)0, (float)0)), bool>::value), "");
    static_assert((std::is_same<decltype(isunordered((long double)0, (double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(isunordered((long double)0, (long double)0)), bool>::value), "");
    static_assert((std::is_same<decltype(isunordered(Ambiguous(), Ambiguous())), Ambiguous>::value), "");
    assert(isunordered(-1.0, 0.F) == false);
}

void test_acosh()
{
    static_assert((std::is_same<decltype(acosh((float)0)), float>::value), "");
    static_assert((std::is_same<decltype(acosh((bool)0)), double>::value), "");
    static_assert((std::is_same<decltype(acosh((unsigned short)0)), double>::value), "");
    static_assert((std::is_same<decltype(acosh((int)0)), double>::value), "");
    static_assert((std::is_same<decltype(acosh((unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(acosh((long)0)), double>::value), "");
    static_assert((std::is_same<decltype(acosh((unsigned long)0)), double>::value), "");
    static_assert((std::is_same<decltype(acosh((long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(acosh((unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(acosh((double)0)), double>::value), "");
    static_assert((std::is_same<decltype(acosh((long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(acoshf(0)), float>::value), "");
    static_assert((std::is_same<decltype(acoshl(0)), long double>::value), "");
    static_assert((std::is_same<decltype(acosh(Ambiguous())), Ambiguous>::value), "");
    assert(acosh(1) == 0);
}

void test_asinh()
{
    static_assert((std::is_same<decltype(asinh((float)0)), float>::value), "");
    static_assert((std::is_same<decltype(asinh((bool)0)), double>::value), "");
    static_assert((std::is_same<decltype(asinh((unsigned short)0)), double>::value), "");
    static_assert((std::is_same<decltype(asinh((int)0)), double>::value), "");
    static_assert((std::is_same<decltype(asinh((unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(asinh((long)0)), double>::value), "");
    static_assert((std::is_same<decltype(asinh((unsigned long)0)), double>::value), "");
    static_assert((std::is_same<decltype(asinh((long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(asinh((unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(asinh((double)0)), double>::value), "");
    static_assert((std::is_same<decltype(asinh((long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(asinhf(0)), float>::value), "");
    static_assert((std::is_same<decltype(asinhl(0)), long double>::value), "");
    static_assert((std::is_same<decltype(asinh(Ambiguous())), Ambiguous>::value), "");
    assert(asinh(0) == 0);
}

void test_atanh()
{
    static_assert((std::is_same<decltype(atanh((float)0)), float>::value), "");
    static_assert((std::is_same<decltype(atanh((bool)0)), double>::value), "");
    static_assert((std::is_same<decltype(atanh((unsigned short)0)), double>::value), "");
    static_assert((std::is_same<decltype(atanh((int)0)), double>::value), "");
    static_assert((std::is_same<decltype(atanh((unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(atanh((long)0)), double>::value), "");
    static_assert((std::is_same<decltype(atanh((unsigned long)0)), double>::value), "");
    static_assert((std::is_same<decltype(atanh((long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(atanh((unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(atanh((double)0)), double>::value), "");
    static_assert((std::is_same<decltype(atanh((long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(atanhf(0)), float>::value), "");
    static_assert((std::is_same<decltype(atanhl(0)), long double>::value), "");
    static_assert((std::is_same<decltype(atanh(Ambiguous())), Ambiguous>::value), "");
    assert(atanh(0) == 0);
}

void test_cbrt() {
    static_assert((std::is_same<decltype(cbrt((float) 0)), float>::value), "");
    static_assert((std::is_same<decltype(cbrt((bool) 0)), double>::value), "");
    static_assert((std::is_same<decltype(cbrt((unsigned short) 0)),
                                double>::value), "");
    static_assert((std::is_same<decltype(cbrt((int) 0)), double>::value), "");
    static_assert((std::is_same<decltype(cbrt((unsigned int) 0)),
                                double>::value), "");
    static_assert((std::is_same<decltype(cbrt((long) 0)), double>::value), "");
    static_assert((std::is_same<decltype(cbrt((unsigned long) 0)),
                                double>::value), "");
    static_assert((std::is_same<decltype(cbrt((long long) 0)), double>::value),
                  "");
    static_assert((std::is_same<decltype(cbrt((unsigned long long) 0)),
                                double>::value), "");
    static_assert((std::is_same<decltype(cbrt((double) 0)), double>::value),
                  "");
    static_assert((std::is_same<decltype(cbrt((long double) 0)),
                                long double>::value), "");
    static_assert((std::is_same<decltype(cbrtf(0)), float>::value), "");
    static_assert((std::is_same<decltype(cbrtl(0)), long double>::value), "");
    static_assert((std::is_same<decltype(cbrt(Ambiguous())), Ambiguous>::value),
                  "");
    assert(truncate_fp(cbrt(1)) == 1);

}

void test_copysign()
{
    static_assert((std::is_same<decltype(copysign((float)0, (float)0)), float>::value), "");
    static_assert((std::is_same<decltype(copysign((bool)0, (float)0)), double>::value), "");
    static_assert((std::is_same<decltype(copysign((unsigned short)0, (double)0)), double>::value), "");
    static_assert((std::is_same<decltype(copysign((int)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(copysign((float)0, (unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(copysign((double)0, (long)0)), double>::value), "");
    static_assert((std::is_same<decltype(copysign((long double)0, (unsigned long)0)), long double>::value), "");
    static_assert((std::is_same<decltype(copysign((int)0, (long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(copysign((int)0, (unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(copysign((double)0, (double)0)), double>::value), "");
    static_assert((std::is_same<decltype(copysign((long double)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(copysign((float)0, (double)0)), double>::value), "");
    static_assert((std::is_same<decltype(copysign((float)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(copysign((double)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(copysignf(0,0)), float>::value), "");
    static_assert((std::is_same<decltype(copysignl(0,0)), long double>::value), "");
    static_assert((std::is_same<decltype(copysign((int)0, (int)0)), double>::value), "");
    static_assert((std::is_same<decltype(copysign(Ambiguous(), Ambiguous())), Ambiguous>::value), "");
    assert(copysign(1,1) == 1);
}

void test_erf()
{
    static_assert((std::is_same<decltype(erf((float)0)), float>::value), "");
    static_assert((std::is_same<decltype(erf((bool)0)), double>::value), "");
    static_assert((std::is_same<decltype(erf((unsigned short)0)), double>::value), "");
    static_assert((std::is_same<decltype(erf((int)0)), double>::value), "");
    static_assert((std::is_same<decltype(erf((unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(erf((long)0)), double>::value), "");
    static_assert((std::is_same<decltype(erf((unsigned long)0)), double>::value), "");
    static_assert((std::is_same<decltype(erf((long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(erf((unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(erf((double)0)), double>::value), "");
    static_assert((std::is_same<decltype(erf((long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(erff(0)), float>::value), "");
    static_assert((std::is_same<decltype(erfl(0)), long double>::value), "");
    static_assert((std::is_same<decltype(erf(Ambiguous())), Ambiguous>::value), "");
    assert(erf(0) == 0);
}

void test_erfc()
{
    static_assert((std::is_same<decltype(erfc((float)0)), float>::value), "");
    static_assert((std::is_same<decltype(erfc((bool)0)), double>::value), "");
    static_assert((std::is_same<decltype(erfc((unsigned short)0)), double>::value), "");
    static_assert((std::is_same<decltype(erfc((int)0)), double>::value), "");
    static_assert((std::is_same<decltype(erfc((unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(erfc((long)0)), double>::value), "");
    static_assert((std::is_same<decltype(erfc((unsigned long)0)), double>::value), "");
    static_assert((std::is_same<decltype(erfc((long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(erfc((unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(erfc((double)0)), double>::value), "");
    static_assert((std::is_same<decltype(erfc((long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(erfcf(0)), float>::value), "");
    static_assert((std::is_same<decltype(erfcl(0)), long double>::value), "");
    static_assert((std::is_same<decltype(erfc(Ambiguous())), Ambiguous>::value), "");
    assert(erfc(0) == 1);
}

void test_exp2()
{
    static_assert((std::is_same<decltype(exp2((float)0)), float>::value), "");
    static_assert((std::is_same<decltype(exp2((bool)0)), double>::value), "");
    static_assert((std::is_same<decltype(exp2((unsigned short)0)), double>::value), "");
    static_assert((std::is_same<decltype(exp2((int)0)), double>::value), "");
    static_assert((std::is_same<decltype(exp2((unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(exp2((long)0)), double>::value), "");
    static_assert((std::is_same<decltype(exp2((unsigned long)0)), double>::value), "");
    static_assert((std::is_same<decltype(exp2((long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(exp2((unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(exp2((double)0)), double>::value), "");
    static_assert((std::is_same<decltype(exp2((long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(exp2f(0)), float>::value), "");
    static_assert((std::is_same<decltype(exp2l(0)), long double>::value), "");
    static_assert((std::is_same<decltype(exp2(Ambiguous())), Ambiguous>::value), "");
    assert(exp2(1) == 2);
}

void test_expm1()
{
    static_assert((std::is_same<decltype(expm1((float)0)), float>::value), "");
    static_assert((std::is_same<decltype(expm1((bool)0)), double>::value), "");
    static_assert((std::is_same<decltype(expm1((unsigned short)0)), double>::value), "");
    static_assert((std::is_same<decltype(expm1((int)0)), double>::value), "");
    static_assert((std::is_same<decltype(expm1((unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(expm1((long)0)), double>::value), "");
    static_assert((std::is_same<decltype(expm1((unsigned long)0)), double>::value), "");
    static_assert((std::is_same<decltype(expm1((long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(expm1((unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(expm1((double)0)), double>::value), "");
    static_assert((std::is_same<decltype(expm1((long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(expm1f(0)), float>::value), "");
    static_assert((std::is_same<decltype(expm1l(0)), long double>::value), "");
    static_assert((std::is_same<decltype(expm1(Ambiguous())), Ambiguous>::value), "");
    assert(expm1(0) == 0);
}

void test_fdim()
{
    static_assert((std::is_same<decltype(fdim((float)0, (float)0)), float>::value), "");
    static_assert((std::is_same<decltype(fdim((bool)0, (float)0)), double>::value), "");
    static_assert((std::is_same<decltype(fdim((unsigned short)0, (double)0)), double>::value), "");
    static_assert((std::is_same<decltype(fdim((int)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(fdim((float)0, (unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(fdim((double)0, (long)0)), double>::value), "");
    static_assert((std::is_same<decltype(fdim((long double)0, (unsigned long)0)), long double>::value), "");
    static_assert((std::is_same<decltype(fdim((int)0, (long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(fdim((int)0, (unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(fdim((double)0, (double)0)), double>::value), "");
    static_assert((std::is_same<decltype(fdim((long double)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(fdim((float)0, (double)0)), double>::value), "");
    static_assert((std::is_same<decltype(fdim((float)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(fdim((double)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(fdimf(0,0)), float>::value), "");
    static_assert((std::is_same<decltype(fdiml(0,0)), long double>::value), "");
    static_assert((std::is_same<decltype(fdim((int)0, (int)0)), double>::value), "");
    static_assert((std::is_same<decltype(fdim(Ambiguous(), Ambiguous())), Ambiguous>::value), "");
    assert(fdim(1,0) == 1);
}

void test_fma()
{
    static_assert((std::is_same<decltype(fma((bool)0, (float)0, (float)0)), double>::value), "");
    static_assert((std::is_same<decltype(fma((char)0, (float)0, (float)0)), double>::value), "");
    static_assert((std::is_same<decltype(fma((unsigned)0, (float)0, (float)0)), double>::value), "");
    static_assert((std::is_same<decltype(fma((float)0, (int)0, (float)0)), double>::value), "");
    static_assert((std::is_same<decltype(fma((float)0, (long)0, (float)0)), double>::value), "");
    static_assert((std::is_same<decltype(fma((float)0, (float)0, (unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(fma((float)0, (float)0, (double)0)), double>::value), "");
    static_assert((std::is_same<decltype(fma((float)0, (float)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(fma((float)0, (float)0, (float)0)), float>::value), "");

    static_assert((std::is_same<decltype(fma((bool)0, (double)0, (double)0)), double>::value), "");
    static_assert((std::is_same<decltype(fma((char)0, (double)0, (double)0)), double>::value), "");
    static_assert((std::is_same<decltype(fma((unsigned)0, (double)0, (double)0)), double>::value), "");
    static_assert((std::is_same<decltype(fma((double)0, (int)0, (double)0)), double>::value), "");
    static_assert((std::is_same<decltype(fma((double)0, (long)0, (double)0)), double>::value), "");
    static_assert((std::is_same<decltype(fma((double)0, (double)0, (unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(fma((double)0, (double)0, (float)0)), double>::value), "");
    static_assert((std::is_same<decltype(fma((double)0, (double)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(fma((double)0, (double)0,  (double)0)), double>::value), "");

    static_assert((std::is_same<decltype(fma((bool)0, (long double)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(fma((char)0, (long double)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(fma((unsigned)0, (long double)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(fma((long double)0, (int)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(fma((long double)0, (long)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(fma((long double)0, (long double)0, (unsigned long long)0)), long double>::value), "");
    static_assert((std::is_same<decltype(fma((long double)0, (long double)0, (float)0)), long double>::value), "");
    static_assert((std::is_same<decltype(fma((double)0, (long double)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(fma((long double)0, (long double)0, (long double)0)), long double>::value), "");

    static_assert((std::is_same<decltype(fmaf(0,0,0)), float>::value), "");
    static_assert((std::is_same<decltype(fmal(0,0,0)), long double>::value), "");
    static_assert((std::is_same<decltype(fma(Ambiguous(), Ambiguous(), Ambiguous())), Ambiguous>::value), "");
    assert(fma(1,1,1) == 2);
}

void test_fmax()
{
    static_assert((std::is_same<decltype(fmax((float)0, (float)0)), float>::value), "");
    static_assert((std::is_same<decltype(fmax((bool)0, (float)0)), double>::value), "");
    static_assert((std::is_same<decltype(fmax((unsigned short)0, (double)0)), double>::value), "");
    static_assert((std::is_same<decltype(fmax((int)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(fmax((float)0, (unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(fmax((double)0, (long)0)), double>::value), "");
    static_assert((std::is_same<decltype(fmax((long double)0, (unsigned long)0)), long double>::value), "");
    static_assert((std::is_same<decltype(fmax((int)0, (long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(fmax((int)0, (unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(fmax((double)0, (double)0)), double>::value), "");
    static_assert((std::is_same<decltype(fmax((long double)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(fmax((float)0, (double)0)), double>::value), "");
    static_assert((std::is_same<decltype(fmax((float)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(fmax((double)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(fmaxf(0,0)), float>::value), "");
    static_assert((std::is_same<decltype(fmaxl(0,0)), long double>::value), "");
    static_assert((std::is_same<decltype(fmax((int)0, (int)0)), double>::value), "");
    static_assert((std::is_same<decltype(fmax(Ambiguous(), Ambiguous())), Ambiguous>::value), "");
    assert(fmax(1,0) == 1);
}

void test_fmin()
{
    static_assert((std::is_same<decltype(fmin((float)0, (float)0)), float>::value), "");
    static_assert((std::is_same<decltype(fmin((bool)0, (float)0)), double>::value), "");
    static_assert((std::is_same<decltype(fmin((unsigned short)0, (double)0)), double>::value), "");
    static_assert((std::is_same<decltype(fmin((int)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(fmin((float)0, (unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(fmin((double)0, (long)0)), double>::value), "");
    static_assert((std::is_same<decltype(fmin((long double)0, (unsigned long)0)), long double>::value), "");
    static_assert((std::is_same<decltype(fmin((int)0, (long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(fmin((int)0, (unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(fmin((double)0, (double)0)), double>::value), "");
    static_assert((std::is_same<decltype(fmin((long double)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(fmin((float)0, (double)0)), double>::value), "");
    static_assert((std::is_same<decltype(fmin((float)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(fmin((double)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(fminf(0,0)), float>::value), "");
    static_assert((std::is_same<decltype(fminl(0,0)), long double>::value), "");
    static_assert((std::is_same<decltype(fmin((int)0, (int)0)), double>::value), "");
    static_assert((std::is_same<decltype(fmin(Ambiguous(), Ambiguous())), Ambiguous>::value), "");
    assert(fmin(1,0) == 0);
}

void test_hypot()
{
    static_assert((std::is_same<decltype(hypot((float)0, (float)0)), float>::value), "");
    static_assert((std::is_same<decltype(hypot((bool)0, (float)0)), double>::value), "");
    static_assert((std::is_same<decltype(hypot((unsigned short)0, (double)0)), double>::value), "");
    static_assert((std::is_same<decltype(hypot((int)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(hypot((float)0, (unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(hypot((double)0, (long)0)), double>::value), "");
    static_assert((std::is_same<decltype(hypot((long double)0, (unsigned long)0)), long double>::value), "");
    static_assert((std::is_same<decltype(hypot((int)0, (long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(hypot((int)0, (unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(hypot((double)0, (double)0)), double>::value), "");
    static_assert((std::is_same<decltype(hypot((long double)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(hypot((float)0, (double)0)), double>::value), "");
    static_assert((std::is_same<decltype(hypot((float)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(hypot((double)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(hypotf(0,0)), float>::value), "");
    static_assert((std::is_same<decltype(hypotl(0,0)), long double>::value), "");
    static_assert((std::is_same<decltype(hypot((int)0, (int)0)), double>::value), "");
    static_assert((std::is_same<decltype(hypot(Ambiguous(), Ambiguous())), Ambiguous>::value), "");
    assert(hypot(3,4) == 5);
}

void test_ilogb()
{
    static_assert((std::is_same<decltype(ilogb((float)0)), int>::value), "");
    static_assert((std::is_same<decltype(ilogb((bool)0)), int>::value), "");
    static_assert((std::is_same<decltype(ilogb((unsigned short)0)), int>::value), "");
    static_assert((std::is_same<decltype(ilogb((int)0)), int>::value), "");
    static_assert((std::is_same<decltype(ilogb((unsigned int)0)), int>::value), "");
    static_assert((std::is_same<decltype(ilogb((long)0)), int>::value), "");
    static_assert((std::is_same<decltype(ilogb((unsigned long)0)), int>::value), "");
    static_assert((std::is_same<decltype(ilogb((long long)0)), int>::value), "");
    static_assert((std::is_same<decltype(ilogb((unsigned long long)0)), int>::value), "");
    static_assert((std::is_same<decltype(ilogb((double)0)), int>::value), "");
    static_assert((std::is_same<decltype(ilogb((long double)0)), int>::value), "");
    static_assert((std::is_same<decltype(ilogbf(0)), int>::value), "");
    static_assert((std::is_same<decltype(ilogbl(0)), int>::value), "");
    static_assert((std::is_same<decltype(ilogb(Ambiguous())), Ambiguous>::value), "");
    assert(ilogb(1) == 0);
}

void test_lgamma()
{
    static_assert((std::is_same<decltype(lgamma((float)0)), float>::value), "");
    static_assert((std::is_same<decltype(lgamma((bool)0)), double>::value), "");
    static_assert((std::is_same<decltype(lgamma((unsigned short)0)), double>::value), "");
    static_assert((std::is_same<decltype(lgamma((int)0)), double>::value), "");
    static_assert((std::is_same<decltype(lgamma((unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(lgamma((long)0)), double>::value), "");
    static_assert((std::is_same<decltype(lgamma((unsigned long)0)), double>::value), "");
    static_assert((std::is_same<decltype(lgamma((long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(lgamma((unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(lgamma((double)0)), double>::value), "");
    static_assert((std::is_same<decltype(lgamma((long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(lgammaf(0)), float>::value), "");
    static_assert((std::is_same<decltype(lgammal(0)), long double>::value), "");
    static_assert((std::is_same<decltype(lgamma(Ambiguous())), Ambiguous>::value), "");
    assert(lgamma(1) == 0);
}

void test_llrint()
{
    static_assert((std::is_same<decltype(llrint((float)0)), long long>::value), "");
    static_assert((std::is_same<decltype(llrint((bool)0)), long long>::value), "");
    static_assert((std::is_same<decltype(llrint((unsigned short)0)), long long>::value), "");
    static_assert((std::is_same<decltype(llrint((int)0)), long long>::value), "");
    static_assert((std::is_same<decltype(llrint((unsigned int)0)), long long>::value), "");
    static_assert((std::is_same<decltype(llrint((long)0)), long long>::value), "");
    static_assert((std::is_same<decltype(llrint((unsigned long)0)), long long>::value), "");
    static_assert((std::is_same<decltype(llrint((long long)0)), long long>::value), "");
    static_assert((std::is_same<decltype(llrint((unsigned long long)0)), long long>::value), "");
    static_assert((std::is_same<decltype(llrint((double)0)), long long>::value), "");
    static_assert((std::is_same<decltype(llrint((long double)0)), long long>::value), "");
    static_assert((std::is_same<decltype(llrintf(0)), long long>::value), "");
    static_assert((std::is_same<decltype(llrintl(0)), long long>::value), "");
    static_assert((std::is_same<decltype(llrint(Ambiguous())), Ambiguous>::value), "");
    assert(llrint(1) == 1LL);
}

void test_llround()
{
    static_assert((std::is_same<decltype(llround((float)0)), long long>::value), "");
    static_assert((std::is_same<decltype(llround((bool)0)), long long>::value), "");
    static_assert((std::is_same<decltype(llround((unsigned short)0)), long long>::value), "");
    static_assert((std::is_same<decltype(llround((int)0)), long long>::value), "");
    static_assert((std::is_same<decltype(llround((unsigned int)0)), long long>::value), "");
    static_assert((std::is_same<decltype(llround((long)0)), long long>::value), "");
    static_assert((std::is_same<decltype(llround((unsigned long)0)), long long>::value), "");
    static_assert((std::is_same<decltype(llround((long long)0)), long long>::value), "");
    static_assert((std::is_same<decltype(llround((unsigned long long)0)), long long>::value), "");
    static_assert((std::is_same<decltype(llround((double)0)), long long>::value), "");
    static_assert((std::is_same<decltype(llround((long double)0)), long long>::value), "");
    static_assert((std::is_same<decltype(llroundf(0)), long long>::value), "");
    static_assert((std::is_same<decltype(llroundl(0)), long long>::value), "");
    static_assert((std::is_same<decltype(llround(Ambiguous())), Ambiguous>::value), "");
    assert(llround(1) == 1LL);
}

void test_log1p()
{
    static_assert((std::is_same<decltype(log1p((float)0)), float>::value), "");
    static_assert((std::is_same<decltype(log1p((bool)0)), double>::value), "");
    static_assert((std::is_same<decltype(log1p((unsigned short)0)), double>::value), "");
    static_assert((std::is_same<decltype(log1p((int)0)), double>::value), "");
    static_assert((std::is_same<decltype(log1p((unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(log1p((long)0)), double>::value), "");
    static_assert((std::is_same<decltype(log1p((unsigned long)0)), double>::value), "");
    static_assert((std::is_same<decltype(log1p((long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(log1p((unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(log1p((double)0)), double>::value), "");
    static_assert((std::is_same<decltype(log1p((long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(log1pf(0)), float>::value), "");
    static_assert((std::is_same<decltype(log1pl(0)), long double>::value), "");
    static_assert((std::is_same<decltype(log1p(Ambiguous())), Ambiguous>::value), "");
    assert(log1p(0) == 0);
}

void test_log2()
{
    static_assert((std::is_same<decltype(log2((float)0)), float>::value), "");
    static_assert((std::is_same<decltype(log2((bool)0)), double>::value), "");
    static_assert((std::is_same<decltype(log2((unsigned short)0)), double>::value), "");
    static_assert((std::is_same<decltype(log2((int)0)), double>::value), "");
    static_assert((std::is_same<decltype(log2((unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(log2((long)0)), double>::value), "");
    static_assert((std::is_same<decltype(log2((unsigned long)0)), double>::value), "");
    static_assert((std::is_same<decltype(log2((long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(log2((unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(log2((double)0)), double>::value), "");
    static_assert((std::is_same<decltype(log2((long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(log2f(0)), float>::value), "");
    static_assert((std::is_same<decltype(log2l(0)), long double>::value), "");
    static_assert((std::is_same<decltype(log2(Ambiguous())), Ambiguous>::value), "");
    assert(log2(1) == 0);
}

void test_logb()
{
    static_assert((std::is_same<decltype(logb((float)0)), float>::value), "");
    static_assert((std::is_same<decltype(logb((bool)0)), double>::value), "");
    static_assert((std::is_same<decltype(logb((unsigned short)0)), double>::value), "");
    static_assert((std::is_same<decltype(logb((int)0)), double>::value), "");
    static_assert((std::is_same<decltype(logb((unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(logb((long)0)), double>::value), "");
    static_assert((std::is_same<decltype(logb((unsigned long)0)), double>::value), "");
    static_assert((std::is_same<decltype(logb((long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(logb((unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(logb((double)0)), double>::value), "");
    static_assert((std::is_same<decltype(logb((long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(logbf(0)), float>::value), "");
    static_assert((std::is_same<decltype(logbl(0)), long double>::value), "");
    static_assert((std::is_same<decltype(logb(Ambiguous())), Ambiguous>::value), "");
    assert(logb(1) == 0);
}

void test_lrint()
{
    static_assert((std::is_same<decltype(lrint((float)0)), long>::value), "");
    static_assert((std::is_same<decltype(lrint((bool)0)), long>::value), "");
    static_assert((std::is_same<decltype(lrint((unsigned short)0)), long>::value), "");
    static_assert((std::is_same<decltype(lrint((int)0)), long>::value), "");
    static_assert((std::is_same<decltype(lrint((unsigned int)0)), long>::value), "");
    static_assert((std::is_same<decltype(lrint((long)0)), long>::value), "");
    static_assert((std::is_same<decltype(lrint((unsigned long)0)), long>::value), "");
    static_assert((std::is_same<decltype(lrint((long long)0)), long>::value), "");
    static_assert((std::is_same<decltype(lrint((unsigned long long)0)), long>::value), "");
    static_assert((std::is_same<decltype(lrint((double)0)), long>::value), "");
    static_assert((std::is_same<decltype(lrint((long double)0)), long>::value), "");
    static_assert((std::is_same<decltype(lrintf(0)), long>::value), "");
    static_assert((std::is_same<decltype(lrintl(0)), long>::value), "");
    static_assert((std::is_same<decltype(lrint(Ambiguous())), Ambiguous>::value), "");
    assert(lrint(1) == 1L);
}

void test_lround()
{
    static_assert((std::is_same<decltype(lround((float)0)), long>::value), "");
    static_assert((std::is_same<decltype(lround((bool)0)), long>::value), "");
    static_assert((std::is_same<decltype(lround((unsigned short)0)), long>::value), "");
    static_assert((std::is_same<decltype(lround((int)0)), long>::value), "");
    static_assert((std::is_same<decltype(lround((unsigned int)0)), long>::value), "");
    static_assert((std::is_same<decltype(lround((long)0)), long>::value), "");
    static_assert((std::is_same<decltype(lround((unsigned long)0)), long>::value), "");
    static_assert((std::is_same<decltype(lround((long long)0)), long>::value), "");
    static_assert((std::is_same<decltype(lround((unsigned long long)0)), long>::value), "");
    static_assert((std::is_same<decltype(lround((double)0)), long>::value), "");
    static_assert((std::is_same<decltype(lround((long double)0)), long>::value), "");
    static_assert((std::is_same<decltype(lroundf(0)), long>::value), "");
    static_assert((std::is_same<decltype(lroundl(0)), long>::value), "");
    static_assert((std::is_same<decltype(lround(Ambiguous())), Ambiguous>::value), "");
    assert(lround(1) == 1L);
}

void test_nan()
{
    static_assert((std::is_same<decltype(nan("")), double>::value), "");
    static_assert((std::is_same<decltype(nanf("")), float>::value), "");
    static_assert((std::is_same<decltype(nanl("")), long double>::value), "");
}

void test_nearbyint()
{
    static_assert((std::is_same<decltype(nearbyint((float)0)), float>::value), "");
    static_assert((std::is_same<decltype(nearbyint((bool)0)), double>::value), "");
    static_assert((std::is_same<decltype(nearbyint((unsigned short)0)), double>::value), "");
    static_assert((std::is_same<decltype(nearbyint((int)0)), double>::value), "");
    static_assert((std::is_same<decltype(nearbyint((unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(nearbyint((long)0)), double>::value), "");
    static_assert((std::is_same<decltype(nearbyint((unsigned long)0)), double>::value), "");
    static_assert((std::is_same<decltype(nearbyint((long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(nearbyint((unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(nearbyint((double)0)), double>::value), "");
    static_assert((std::is_same<decltype(nearbyint((long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(nearbyintf(0)), float>::value), "");
    static_assert((std::is_same<decltype(nearbyintl(0)), long double>::value), "");
    static_assert((std::is_same<decltype(nearbyint(Ambiguous())), Ambiguous>::value), "");
    assert(nearbyint(1) == 1);
}

void test_nextafter()
{
    static_assert((std::is_same<decltype(nextafter((float)0, (float)0)), float>::value), "");
    static_assert((std::is_same<decltype(nextafter((bool)0, (float)0)), double>::value), "");
    static_assert((std::is_same<decltype(nextafter((unsigned short)0, (double)0)), double>::value), "");
    static_assert((std::is_same<decltype(nextafter((int)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(nextafter((float)0, (unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(nextafter((double)0, (long)0)), double>::value), "");
    static_assert((std::is_same<decltype(nextafter((long double)0, (unsigned long)0)), long double>::value), "");
    static_assert((std::is_same<decltype(nextafter((int)0, (long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(nextafter((int)0, (unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(nextafter((double)0, (double)0)), double>::value), "");
    static_assert((std::is_same<decltype(nextafter((long double)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(nextafter((float)0, (double)0)), double>::value), "");
    static_assert((std::is_same<decltype(nextafter((float)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(nextafter((double)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(nextafterf(0,0)), float>::value), "");
    static_assert((std::is_same<decltype(nextafterl(0,0)), long double>::value), "");
    static_assert((std::is_same<decltype(nextafter((int)0, (int)0)), double>::value), "");
    static_assert((std::is_same<decltype(nextafter(Ambiguous(), Ambiguous())), Ambiguous>::value), "");
    assert(nextafter(0,1) == hexfloat<double>(0x1, 0, -1074));
}

void test_nexttoward()
{
    static_assert((std::is_same<decltype(nexttoward((float)0, (long double)0)), float>::value), "");
    static_assert((std::is_same<decltype(nexttoward((bool)0, (long double)0)), double>::value), "");
    static_assert((std::is_same<decltype(nexttoward((unsigned short)0, (long double)0)), double>::value), "");
    static_assert((std::is_same<decltype(nexttoward((int)0, (long double)0)), double>::value), "");
    static_assert((std::is_same<decltype(nexttoward((unsigned int)0, (long double)0)), double>::value), "");
    static_assert((std::is_same<decltype(nexttoward((long)0, (long double)0)), double>::value), "");
    static_assert((std::is_same<decltype(nexttoward((unsigned long)0, (long double)0)), double>::value), "");
    static_assert((std::is_same<decltype(nexttoward((long long)0, (long double)0)), double>::value), "");
    static_assert((std::is_same<decltype(nexttoward((unsigned long long)0, (long double)0)), double>::value), "");
    static_assert((std::is_same<decltype(nexttoward((double)0, (long double)0)), double>::value), "");
    static_assert((std::is_same<decltype(nexttoward((long double)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(nexttowardf(0, (long double)0)), float>::value), "");
    static_assert((std::is_same<decltype(nexttowardl(0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(nexttoward(Ambiguous(), Ambiguous())), Ambiguous>::value), "");
    assert(nexttoward(0, 1) == hexfloat<double>(0x1, 0, -1074));
}

void test_remainder()
{
    static_assert((std::is_same<decltype(remainder((float)0, (float)0)), float>::value), "");
    static_assert((std::is_same<decltype(remainder((bool)0, (float)0)), double>::value), "");
    static_assert((std::is_same<decltype(remainder((unsigned short)0, (double)0)), double>::value), "");
    static_assert((std::is_same<decltype(remainder((int)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(remainder((float)0, (unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(remainder((double)0, (long)0)), double>::value), "");
    static_assert((std::is_same<decltype(remainder((long double)0, (unsigned long)0)), long double>::value), "");
    static_assert((std::is_same<decltype(remainder((int)0, (long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(remainder((int)0, (unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(remainder((double)0, (double)0)), double>::value), "");
    static_assert((std::is_same<decltype(remainder((long double)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(remainder((float)0, (double)0)), double>::value), "");
    static_assert((std::is_same<decltype(remainder((float)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(remainder((double)0, (long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(remainderf(0,0)), float>::value), "");
    static_assert((std::is_same<decltype(remainderl(0,0)), long double>::value), "");
    static_assert((std::is_same<decltype(remainder((int)0, (int)0)), double>::value), "");
    static_assert((std::is_same<decltype(remainder(Ambiguous(), Ambiguous())), Ambiguous>::value), "");
    assert(remainder(0.5,1) == 0.5);
}

void test_remquo()
{
    int ip;
    static_assert((std::is_same<decltype(remquo((float)0, (float)0, &ip)), float>::value), "");
    static_assert((std::is_same<decltype(remquo((bool)0, (float)0, &ip)), double>::value), "");
    static_assert((std::is_same<decltype(remquo((unsigned short)0, (double)0, &ip)), double>::value), "");
    static_assert((std::is_same<decltype(remquo((int)0, (long double)0, &ip)), long double>::value), "");
    static_assert((std::is_same<decltype(remquo((float)0, (unsigned int)0, &ip)), double>::value), "");
    static_assert((std::is_same<decltype(remquo((double)0, (long)0, &ip)), double>::value), "");
    static_assert((std::is_same<decltype(remquo((long double)0, (unsigned long)0, &ip)), long double>::value), "");
    static_assert((std::is_same<decltype(remquo((int)0, (long long)0, &ip)), double>::value), "");
    static_assert((std::is_same<decltype(remquo((int)0, (unsigned long long)0, &ip)), double>::value), "");
    static_assert((std::is_same<decltype(remquo((double)0, (double)0, &ip)), double>::value), "");
    static_assert((std::is_same<decltype(remquo((long double)0, (long double)0, &ip)), long double>::value), "");
    static_assert((std::is_same<decltype(remquo((float)0, (double)0, &ip)), double>::value), "");
    static_assert((std::is_same<decltype(remquo((float)0, (long double)0, &ip)), long double>::value), "");
    static_assert((std::is_same<decltype(remquo((double)0, (long double)0, &ip)), long double>::value), "");
    static_assert((std::is_same<decltype(remquof(0,0, &ip)), float>::value), "");
    static_assert((std::is_same<decltype(remquol(0,0, &ip)), long double>::value), "");
    static_assert((std::is_same<decltype(remquo((int)0, (int)0, &ip)), double>::value), "");
    static_assert((std::is_same<decltype(remquo(Ambiguous(), Ambiguous(), &ip)), Ambiguous>::value), "");
    assert(remquo(0.5,1, &ip) == 0.5);
}

void test_rint()
{
    static_assert((std::is_same<decltype(rint((float)0)), float>::value), "");
    static_assert((std::is_same<decltype(rint((bool)0)), double>::value), "");
    static_assert((std::is_same<decltype(rint((unsigned short)0)), double>::value), "");
    static_assert((std::is_same<decltype(rint((int)0)), double>::value), "");
    static_assert((std::is_same<decltype(rint((unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(rint((long)0)), double>::value), "");
    static_assert((std::is_same<decltype(rint((unsigned long)0)), double>::value), "");
    static_assert((std::is_same<decltype(rint((long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(rint((unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(rint((double)0)), double>::value), "");
    static_assert((std::is_same<decltype(rint((long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(rintf(0)), float>::value), "");
    static_assert((std::is_same<decltype(rintl(0)), long double>::value), "");
    static_assert((std::is_same<decltype(rint(Ambiguous())), Ambiguous>::value), "");
    assert(rint(1) == 1);
}

void test_round()
{
    static_assert((std::is_same<decltype(round((float)0)), float>::value), "");
    static_assert((std::is_same<decltype(round((bool)0)), double>::value), "");
    static_assert((std::is_same<decltype(round((unsigned short)0)), double>::value), "");
    static_assert((std::is_same<decltype(round((int)0)), double>::value), "");
    static_assert((std::is_same<decltype(round((unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(round((long)0)), double>::value), "");
    static_assert((std::is_same<decltype(round((unsigned long)0)), double>::value), "");
    static_assert((std::is_same<decltype(round((long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(round((unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(round((double)0)), double>::value), "");
    static_assert((std::is_same<decltype(round((long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(roundf(0)), float>::value), "");
    static_assert((std::is_same<decltype(roundl(0)), long double>::value), "");
    static_assert((std::is_same<decltype(round(Ambiguous())), Ambiguous>::value), "");
    assert(round(1) == 1);
}

void test_scalbln()
{
    static_assert((std::is_same<decltype(scalbln((float)0, (long)0)), float>::value), "");
    static_assert((std::is_same<decltype(scalbln((bool)0, (long)0)), double>::value), "");
    static_assert((std::is_same<decltype(scalbln((unsigned short)0, (long)0)), double>::value), "");
    static_assert((std::is_same<decltype(scalbln((int)0, (long)0)), double>::value), "");
    static_assert((std::is_same<decltype(scalbln((unsigned int)0, (long)0)), double>::value), "");
    static_assert((std::is_same<decltype(scalbln((long)0, (long)0)), double>::value), "");
    static_assert((std::is_same<decltype(scalbln((unsigned long)0, (long)0)), double>::value), "");
    static_assert((std::is_same<decltype(scalbln((long long)0, (long)0)), double>::value), "");
    static_assert((std::is_same<decltype(scalbln((unsigned long long)0, (long)0)), double>::value), "");
    static_assert((std::is_same<decltype(scalbln((double)0, (long)0)), double>::value), "");
    static_assert((std::is_same<decltype(scalbln((long double)0, (long)0)), long double>::value), "");
    static_assert((std::is_same<decltype(scalblnf(0, (long)0)), float>::value), "");
    static_assert((std::is_same<decltype(scalblnl(0, (long)0)), long double>::value), "");
    static_assert((std::is_same<decltype(scalbln(Ambiguous(), Ambiguous())), Ambiguous>::value), "");
    assert(scalbln(1, 1) == 2);
}

void test_scalbn()
{
    static_assert((std::is_same<decltype(scalbn((float)0, (int)0)), float>::value), "");
    static_assert((std::is_same<decltype(scalbn((bool)0, (int)0)), double>::value), "");
    static_assert((std::is_same<decltype(scalbn((unsigned short)0, (int)0)), double>::value), "");
    static_assert((std::is_same<decltype(scalbn((int)0, (int)0)), double>::value), "");
    static_assert((std::is_same<decltype(scalbn((unsigned int)0, (int)0)), double>::value), "");
    static_assert((std::is_same<decltype(scalbn((long)0, (int)0)), double>::value), "");
    static_assert((std::is_same<decltype(scalbn((unsigned long)0, (int)0)), double>::value), "");
    static_assert((std::is_same<decltype(scalbn((long long)0, (int)0)), double>::value), "");
    static_assert((std::is_same<decltype(scalbn((unsigned long long)0, (int)0)), double>::value), "");
    static_assert((std::is_same<decltype(scalbn((double)0, (int)0)), double>::value), "");
    static_assert((std::is_same<decltype(scalbn((long double)0, (int)0)), long double>::value), "");
    static_assert((std::is_same<decltype(scalbnf(0, (int)0)), float>::value), "");
    static_assert((std::is_same<decltype(scalbnl(0, (int)0)), long double>::value), "");
    static_assert((std::is_same<decltype(scalbn(Ambiguous(), Ambiguous())), Ambiguous>::value), "");
    assert(scalbn(1, 1) == 2);
}

void test_tgamma()
{
    static_assert((std::is_same<decltype(tgamma((float)0)), float>::value), "");
    static_assert((std::is_same<decltype(tgamma((bool)0)), double>::value), "");
    static_assert((std::is_same<decltype(tgamma((unsigned short)0)), double>::value), "");
    static_assert((std::is_same<decltype(tgamma((int)0)), double>::value), "");
    static_assert((std::is_same<decltype(tgamma((unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(tgamma((long)0)), double>::value), "");
    static_assert((std::is_same<decltype(tgamma((unsigned long)0)), double>::value), "");
    static_assert((std::is_same<decltype(tgamma((long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(tgamma((unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(tgamma((double)0)), double>::value), "");
    static_assert((std::is_same<decltype(tgamma((long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(tgammaf(0)), float>::value), "");
    static_assert((std::is_same<decltype(tgammal(0)), long double>::value), "");
    static_assert((std::is_same<decltype(tgamma(Ambiguous())), Ambiguous>::value), "");
    assert(tgamma(1) == 1);
}

void test_trunc()
{
    static_assert((std::is_same<decltype(trunc((float)0)), float>::value), "");
    static_assert((std::is_same<decltype(trunc((bool)0)), double>::value), "");
    static_assert((std::is_same<decltype(trunc((unsigned short)0)), double>::value), "");
    static_assert((std::is_same<decltype(trunc((int)0)), double>::value), "");
    static_assert((std::is_same<decltype(trunc((unsigned int)0)), double>::value), "");
    static_assert((std::is_same<decltype(trunc((long)0)), double>::value), "");
    static_assert((std::is_same<decltype(trunc((unsigned long)0)), double>::value), "");
    static_assert((std::is_same<decltype(trunc((long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(trunc((unsigned long long)0)), double>::value), "");
    static_assert((std::is_same<decltype(trunc((double)0)), double>::value), "");
    static_assert((std::is_same<decltype(trunc((long double)0)), long double>::value), "");
    static_assert((std::is_same<decltype(truncf(0)), float>::value), "");
    static_assert((std::is_same<decltype(truncl(0)), long double>::value), "");
    static_assert((std::is_same<decltype(trunc(Ambiguous())), Ambiguous>::value), "");
    assert(trunc(1) == 1);
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
