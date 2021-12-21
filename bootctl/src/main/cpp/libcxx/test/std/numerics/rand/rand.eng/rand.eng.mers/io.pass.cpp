//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <random>

// template <class UIntType, size_t w, size_t n, size_t m, size_t r,
//           UIntType a, size_t u, UIntType d, size_t s,
//           UIntType b, size_t t, UIntType c, size_t l, UIntType f>
// class mersenne_twister_engine;

// template <class charT, class traits,
//           class UIntType, size_t w, size_t n, size_t m, size_t r,
//           UIntType a, size_t u, UIntType d, size_t s,
//           UIntType b, size_t t, UIntType c, size_t l, UIntType f>
// basic_ostream<charT, traits>&
// operator<<(basic_ostream<charT, traits>& os,
//            const mersenne_twister_engine<UIntType, w, n, m, r, a, u, d, s, b, t, c, l, f>& x);
//
// template <class charT, class traits,
//           class UIntType, size_t w, size_t n, size_t m, size_t r,
//           UIntType a, size_t u, UIntType d, size_t s,
//           UIntType b, size_t t, UIntType c, size_t l, UIntType f>
// basic_ostream<charT, traits>&
// operator>>(basic_istream<charT, traits>& is,
//            mersenne_twister_engine<UIntType, w, n, m, r, a, u, d, s, b, t, c, l, f>& x);

#include <random>
#include <sstream>
#include <cassert>

void
test1()
{
    typedef std::mt19937 E;
    E e1;
    e1.discard(100);
    std::ostringstream os;
    os << e1;
    std::istringstream is(os.str());
    E e2;
    is >> e2;
    assert(e1 == e2);
}

void
test2()
{
    typedef std::mt19937_64 E;
    E e1;
    e1.discard(100);
    std::ostringstream os;
    os << e1;
    std::istringstream is(os.str());
    E e2;
    is >> e2;
    assert(e1 == e2);
}

int main()
{
    test1();
    test2();
}
