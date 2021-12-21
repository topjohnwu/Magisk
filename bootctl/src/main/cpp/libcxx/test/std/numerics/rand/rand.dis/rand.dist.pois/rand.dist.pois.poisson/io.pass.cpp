//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <random>

// template<class IntType = int>
// class poisson_distribution

// template <class CharT, class Traits, class IntType>
// basic_ostream<CharT, Traits>&
// operator<<(basic_ostream<CharT, Traits>& os,
//            const poisson_distribution<RealType>& x);

// template <class CharT, class Traits, class IntType>
// basic_istream<CharT, Traits>&
// operator>>(basic_istream<CharT, Traits>& is,
//            poisson_distribution<RealType>& x);

#include <random>
#include <sstream>
#include <cassert>

int main()
{
    {
        typedef std::poisson_distribution<> D;
        D d1(7);
        std::ostringstream os;
        os << d1;
        std::istringstream is(os.str());
        D d2;
        is >> d2;
        assert(d1 == d2);
    }
}
