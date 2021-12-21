//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <random>

// template<class UIntType, size_t w, size_t s, size_t r>
// class subtract_with_carry_engine;

// template <class charT, class traits,
//           class UIntType, size_t w, size_t s, size_t r>
// basic_ostream<charT, traits>&
// operator<<(basic_ostream<charT, traits>& os,
//            const subtract_with_carry_engine<UIntType, w, s, r>& x);
//
// template <class charT, class traits,
//           class UIntType, size_t w, size_t s, size_t r>
// basic_istream<charT, traits>&
// operator>>(basic_istream<charT, traits>& is,
//            subtract_with_carry_engine<UIntType, w, s, r>& x);

#include <random>
#include <sstream>
#include <cassert>

void
test1()
{
    typedef std::ranlux24_base E;
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
    typedef std::ranlux48_base E;
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
