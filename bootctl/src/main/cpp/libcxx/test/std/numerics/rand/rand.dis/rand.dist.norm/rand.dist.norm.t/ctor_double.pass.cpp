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

// explicit student_t_distribution(result_type alpha = 0, result_type beta = 1);

#include <random>
#include <cassert>

int main()
{
    {
        typedef std::student_t_distribution<> D;
        D d;
        assert(d.n() == 1);
    }
    {
        typedef std::student_t_distribution<> D;
        D d(14.5);
        assert(d.n() == 14.5);
    }
}
