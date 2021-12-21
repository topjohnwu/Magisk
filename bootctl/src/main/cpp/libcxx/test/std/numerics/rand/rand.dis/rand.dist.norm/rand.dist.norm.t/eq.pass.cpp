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

// bool operator=(const student_t_distribution& x,
//                const student_t_distribution& y);
// bool operator!(const student_t_distribution& x,
//                const student_t_distribution& y);

#include <random>
#include <cassert>

int main()
{
    {
        typedef std::student_t_distribution<> D;
        D d1(2.5);
        D d2(2.5);
        assert(d1 == d2);
    }
    {
        typedef std::student_t_distribution<> D;
        D d1(4);
        D d2(4.5);
        assert(d1 != d2);
    }
}
