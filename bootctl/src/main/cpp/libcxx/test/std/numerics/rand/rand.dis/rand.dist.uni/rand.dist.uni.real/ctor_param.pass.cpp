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
// class uniform_real_distribution

// explicit uniform_real_distribution(const param_type& parm);

#include <random>
#include <cassert>

int main()
{
    {
        typedef std::uniform_real_distribution<> D;
        typedef D::param_type P;
        P p(3.5, 8);
        D d(p);
        assert(d.a() == 3.5);
        assert(d.b() == 8);
    }
}
