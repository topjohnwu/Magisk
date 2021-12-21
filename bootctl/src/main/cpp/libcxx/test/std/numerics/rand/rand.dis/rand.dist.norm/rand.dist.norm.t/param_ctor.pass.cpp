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
// class student_t_distribution
// {
//     class param_type;

#include <random>
#include <limits>
#include <cassert>

int main()
{
    {
        typedef std::student_t_distribution<> D;
        typedef D::param_type param_type;
        param_type p;
        assert(p.n() == 1);
    }
    {
        typedef std::student_t_distribution<> D;
        typedef D::param_type param_type;
        param_type p(10);
        assert(p.n() == 10);
    }
}
