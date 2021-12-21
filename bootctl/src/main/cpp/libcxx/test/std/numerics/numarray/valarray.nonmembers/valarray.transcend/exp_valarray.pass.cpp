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
//   exp(const valarray<T>& x);

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
        T a1[] = {-.9, -.5, 0., .5, .75};
        T a3[] = {4.0656965974059911e-01,
                  6.0653065971263342e-01,
                  1.0000000000000000e+00,
                  1.6487212707001282e+00,
                  2.1170000166126748e+00};
        const unsigned N = sizeof(a1)/sizeof(a1[0]);
        std::valarray<T> v1(a1, N);
        std::valarray<T> v3 = exp(v1);
        assert(v3.size() == v1.size());
        for (std::size_t i = 0; i < v3.size(); ++i)
            assert(is_about(v3[i], a3[i], 10));
    }
}
