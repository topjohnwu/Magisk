//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <random>

// template<class Engine, size_t w, class UIntType>
// class independent_bits_engine

// template<class Sseq> explicit independent_bits_engine(Sseq& q);

#include <random>
#include <sstream>
#include <cassert>

void
test1()
{
    const char* a = "13604817 711567 9760686 13278398 3323440 175548 5553651 "
    "3028863 10748297 2216688 275779 14778841 14438394 9483441 4229545 "
    "14657301 12636508 15978210 1653340 1718567 9272421 14302862 7940348 "
    "889045 0 0";
    unsigned as[] = {3, 5, 7};
    std::seed_seq sseq(as, as+3);
    std::independent_bits_engine<std::ranlux24, 32, unsigned> e1(sseq);
    std::ostringstream os;
    os << e1;
    assert(os.str() == a);
}

void
test2()
{
    const char* a = "241408498702289 172342669275054 191026374555184 "
    "61020585639411 231929771458953 142769679250755 198672786411514 "
    "183712717244841 227473912549724 62843577252444 68782400568421 "
    "159248704678140 0 0";
    unsigned as[] = {3, 5, 7};
    std::seed_seq sseq(as, as+3);
    std::independent_bits_engine<std::ranlux48, 64, unsigned long long> e1(sseq);
    std::ostringstream os;
    os << e1;
    assert(os.str() == a);
}

int main()
{
    test1();
    test2();
}
