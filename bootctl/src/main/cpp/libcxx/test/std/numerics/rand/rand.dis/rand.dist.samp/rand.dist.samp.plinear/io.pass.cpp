//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <random>

// template<class RealType = double>
// class piecewise_linear_distribution

// template <class charT, class traits>
// basic_ostream<charT, traits>&
// operator<<(basic_ostream<charT, traits>& os,
//            const piecewise_linear_distribution& x);
//
// template <class charT, class traits>
// basic_istream<charT, traits>&
// operator>>(basic_istream<charT, traits>& is,
//            piecewise_linear_distribution& x);

#include <random>
#include <sstream>
#include <cassert>

int main()
{
    {
        typedef std::piecewise_linear_distribution<> D;
        double b[] = {10, 14, 16, 17};
        double p[] = {25, 62.5, 12.5, 25};
        const size_t Np = sizeof(p) / sizeof(p[0]);
        D d1(b, b+Np, p);
        std::ostringstream os;
        os << d1;
        std::istringstream is(os.str());
        D d2;
        is >> d2;
        assert(d1 == d2);
    }
}
