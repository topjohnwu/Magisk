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

// template<class Sseq> explicit linear_congruential_engine(Sseq& q);

#include <random>
#include <cassert>

int main()
{
    {
        unsigned a[] = {3, 5, 7};
        std::seed_seq sseq(a, a+3);
        std::linear_congruential_engine<unsigned, 5, 7, 11> e1(sseq);
        std::linear_congruential_engine<unsigned, 5, 7, 11> e2(4);
        assert(e1 == e2);
    }
}
