//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <random>

// template<class _IntType = int>
// class uniform_int_distribution

// void param(const param_type& parm);

#include <random>
#include <cassert>

int main()
{
    {
        typedef std::uniform_int_distribution<> D;
        typedef D::param_type P;
        P p(3, 8);
        D d(6, 7);
        d.param(p);
        assert(d.param() == p);
    }
}
