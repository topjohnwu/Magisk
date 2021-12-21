//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <random>

// template <class UIntType, UIntType a, UIntType c, UIntType m>
//   class linear_congruential_engine;

// template<class Sseq> void seed(Sseq& q);

#include <random>
#include <cassert>

int main()
{
    {
        unsigned a[] = {3, 5, 7};
        std::seed_seq sseq(a, a+3);
        std::linear_congruential_engine<unsigned, 5, 7, 11> e1;
        std::linear_congruential_engine<unsigned, 5, 7, 11> e2(4);
        assert(e1 != e2);
        e1.seed(sseq);
        assert(e1 == e2);
    }
    {
        unsigned a[] = {3, 5, 7, 9, 11};
        std::seed_seq sseq(a, a+5);
        typedef std::linear_congruential_engine<unsigned long long, 1, 1, 0x200000001ULL> E;
        E e1(4309005589);
        E e2(sseq);
        assert(e1 == e2);
    }
}
