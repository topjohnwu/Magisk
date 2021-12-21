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

// student_t_distribution(const student_t_distribution&);

#include <random>
#include <cassert>

void
test1()
{
    typedef std::student_t_distribution<> D;
    D d1(21.75);
    D d2 = d1;
    assert(d1 == d2);
}

int main()
{
    test1();
}
