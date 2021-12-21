//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <random>

// template<class Engine, size_t k>
// class shuffle_order_engine

// template <class charT, class traits,
//           class Engine, size_t k>
// basic_ostream<charT, traits>&
// operator<<(basic_ostream<charT, traits>& os,
//            const shuffle_order_engine<Engine, k>& x);
//
// template <class charT, class traits,
//           class Engine, size_t k>
// basic_istream<charT, traits>&
// operator>>(basic_istream<charT, traits>& is,
//            shuffle_order_engine<Engine, k>& x);

#include <random>
#include <sstream>
#include <cassert>

void
test1()
{
    typedef std::knuth_b E;
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
}
