//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <random>

// template<class Engine, size_t p, size_t r>
// class discard_block_engine

// template <class charT, class traits,
//           class Engine, size_t p, size_t r>
// basic_ostream<charT, traits>&
// operator<<(basic_ostream<charT, traits>& os,
//            const discard_block_engine<Engine, p, r>& x);
//
// template <class charT, class traits,
//           class Engine, size_t p, size_t r>
// basic_istream<charT, traits>&
// operator>>(basic_istream<charT, traits>& is,
//            discard_block_engine<Engine, p, r>& x);

#include <random>
#include <sstream>
#include <cassert>

void
test1()
{
    typedef std::ranlux24 E;
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
    typedef std::ranlux48 E;
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
