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
// class gamma_distribution
// {
//     class param_type;

#include <random>
#include <limits>
#include <cassert>

int main()
{
    {
        typedef std::gamma_distribution<> D;
        typedef D::param_type param_type;
        param_type p0(.75, 6);
        param_type p;
        p = p0;
        assert(p.alpha() == .75);
        assert(p.beta() == 6);
    }
}
