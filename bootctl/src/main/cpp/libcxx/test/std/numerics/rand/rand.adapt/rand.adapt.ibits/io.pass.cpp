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

// template <class charT, class traits,
//           class Engine, size_t w, class UIntType>
// basic_ostream<charT, traits>&
// operator<<(basic_ostream<charT, traits>& os,
//            const independent_bits_engine<Engine, w, UIntType>& x);
//
// template <class charT, class traits,
//           class Engine, size_t w, class UIntType>
// basic_istream<charT, traits>&
// operator>>(basic_istream<charT, traits>& is,
//            independent_bits_engine<Engine, w, UIntType>& x);

#include <random>
#include <sstream>
#include <cassert>

void
test1()
{
    typedef std::independent_bits_engine<std::ranlux24, 32, unsigned> E;
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
    typedef std::independent_bits_engine<std::ranlux48, 64, unsigned long long> E;
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
