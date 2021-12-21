//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <valarray>

// template<class T> class valarray;

// template<class T>
//   valarray<T>
//   sqrt(const valarray<T>& x);

#include <valarray>
#include <cassert>
#include <sstream>
#include <cstddef>

bool is_about(double x, double y, int p)
{
    std::ostringstream o;
    o.precision(p);
    scientific(o);
    o << x;
    std::string a = o.str();
    o.str("");
    o << y;
    return a == o.str();
}

int main()
{
    {
        typedef double T;
        T a1[] = {.5, .75, 1, 3, 7};
        T a3[] = {7.0710678118654757e-01,
                  8.6602540378443860e-01,
                  1.0000000000000000e+00,
                  1.7320508075688772e+00,
                  2.6457513110645907e+00};
        const unsigned N = sizeof(a1)/sizeof(a1[0]);
        std::valarray<T> v1(a1, N);
        std::valarray<T> v3 = sqrt(v1);
        assert(v3.size() == v1.size());
        for (std::size_t i = 0; i < v3.size(); ++i)
            assert(is_about(v3[i], a3[i], 10));
    }
}
