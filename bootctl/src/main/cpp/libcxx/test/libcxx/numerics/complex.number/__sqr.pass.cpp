//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <complex>

// template<class T>
//   complex<T>
//   __sqr(const complex<T>& x);

#include <complex>
#include <cassert>

template <class T>
void
test()
{
    const T tolerance = std::is_same<T, float>::value ? 1.e-6 : 1.e-14;

    typedef std::complex<T> cplx;
    struct test_case
    {
        cplx value;
        cplx expected;
    };

    const test_case cases[] = {
        {cplx( 0,  0), cplx( 0,  0)},
        {cplx( 1,  0), cplx( 1,  0)},
        {cplx( 2,  0), cplx( 4,  0)},
        {cplx(-1,  0), cplx( 1,  0)},
        {cplx( 0,  1), cplx(-1,  0)},
        {cplx( 0,  2), cplx(-4,  0)},
        {cplx( 0, -1), cplx(-1,  0)},
        {cplx( 1,  1), cplx( 0,  2)},
        {cplx( 1, -1), cplx( 0, -2)},
        {cplx(-1, -1), cplx( 0,  2)},
        {cplx(0.5, 0), cplx(0.25, 0)},
    };

    const unsigned num_cases = sizeof(cases) / sizeof(test_case);
    for (unsigned i = 0; i < num_cases; ++i)
    {
        const test_case& test = cases[i];
        const std::complex<T> actual = std::__sqr(test.value);
        assert(std::abs(actual.real() - test.expected.real()) < tolerance);
        assert(std::abs(actual.imag() - test.expected.imag()) < tolerance);
    }

    const cplx nan1 = std::__sqr(cplx(NAN, 0));
    assert(std::isnan(nan1.real()));
    assert(std::isnan(nan1.imag()));

    const cplx nan2 = std::__sqr(cplx(0, NAN));
    assert(std::isnan(nan2.real()));
    assert(std::isnan(nan2.imag()));

    const cplx nan3 = std::__sqr(cplx(NAN, NAN));
    assert(std::isnan(nan3.real()));
    assert(std::isnan(nan3.imag()));

    const cplx inf1 = std::__sqr(cplx(INFINITY, 0));
    assert(std::isinf(inf1.real()));
    assert(inf1.real() > 0);

    const cplx inf2 = std::__sqr(cplx(0, INFINITY));
    assert(std::isinf(inf2.real()));
    assert(inf2.real() < 0);
}

int main()
{
    test<float>();
    test<double>();
    test<long double>();
}
