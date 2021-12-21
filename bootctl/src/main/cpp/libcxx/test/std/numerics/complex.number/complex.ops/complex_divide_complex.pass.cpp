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
//   operator/(const complex<T>& lhs, const complex<T>& rhs);

#include <complex>
#include <cassert>

#include "../cases.h"

template <class T>
void
test(const std::complex<T>& lhs, const std::complex<T>& rhs, std::complex<T> x)
{
    assert(lhs / rhs == x);
}

template <class T>
void
test()
{
    std::complex<T> lhs(-4.0, 7.5);
    std::complex<T> rhs(1.5, 2.5);
    std::complex<T>   x(1.5, 2.5);
    test(lhs, rhs, x);
}

void test_edges()
{
    const unsigned N = sizeof(testcases) / sizeof(testcases[0]);
    for (unsigned i = 0; i < N; ++i)
    {
        for (unsigned j = 0; j < N; ++j)
        {
            std::complex<double> r = testcases[i] / testcases[j];
            switch (classify(testcases[i]))
            {
            case zero:
                switch (classify(testcases[j]))
                {
                case zero:
                    assert(classify(r) == NaN);
                    break;
                case non_zero:
                    assert(classify(r) == zero);
                    break;
                case inf:
                    assert(classify(r) == zero);
                    break;
                case NaN:
                    assert(classify(r) == NaN);
                    break;
                case non_zero_nan:
                    assert(classify(r) == NaN);
                    break;
                }
                break;
            case non_zero:
                switch (classify(testcases[j]))
                {
                case zero:
                    assert(classify(r) == inf);
                    break;
                case non_zero:
                    assert(classify(r) == non_zero);
                    break;
                case inf:
                    assert(classify(r) == zero);
                    break;
                case NaN:
                    assert(classify(r) == NaN);
                    break;
                case non_zero_nan:
                    assert(classify(r) == NaN);
                    break;
                }
                break;
            case inf:
                switch (classify(testcases[j]))
                {
                case zero:
                    assert(classify(r) == inf);
                    break;
                case non_zero:
                    assert(classify(r) == inf);
                    break;
                case inf:
                    assert(classify(r) == NaN);
                    break;
                case NaN:
                    assert(classify(r) == NaN);
                    break;
                case non_zero_nan:
                    assert(classify(r) == NaN);
                    break;
                }
                break;
            case NaN:
                switch (classify(testcases[j]))
                {
                case zero:
                    assert(classify(r) == NaN);
                    break;
                case non_zero:
                    assert(classify(r) == NaN);
                    break;
                case inf:
                    assert(classify(r) == NaN);
                    break;
                case NaN:
                    assert(classify(r) == NaN);
                    break;
                case non_zero_nan:
                    assert(classify(r) == NaN);
                    break;
                }
                break;
            case non_zero_nan:
                switch (classify(testcases[j]))
                {
                case zero:
                    assert(classify(r) == inf);
                    break;
                case non_zero:
                    assert(classify(r) == NaN);
                    break;
                case inf:
                    assert(classify(r) == NaN);
                    break;
                case NaN:
                    assert(classify(r) == NaN);
                    break;
                case non_zero_nan:
                    assert(classify(r) == NaN);
                    break;
                }
                break;
            }
        }
    }
}

int main()
{
    test<float>();
    test<double>();
    test<long double>();
    test_edges();
}
