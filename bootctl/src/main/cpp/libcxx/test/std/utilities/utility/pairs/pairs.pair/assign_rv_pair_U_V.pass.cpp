//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <utility>

// template <class T1, class T2> struct pair

// template<class U, class V> pair& operator=(pair<U, V>&& p);

#include <utility>
#include <memory>
#include <cassert>
#include <archetypes.hpp>

struct Base
{
    virtual ~Base() {}
};

struct Derived
    : public Base
{
};

int main()
{
    {
        typedef std::pair<std::unique_ptr<Derived>, short> P1;
        typedef std::pair<std::unique_ptr<Base>, long> P2;
        P1 p1(std::unique_ptr<Derived>(), static_cast<short>(4));
        P2 p2;
        p2 = std::move(p1);
        assert(p2.first == nullptr);
        assert(p2.second == 4);
    }
    {
       using C = TestTypes::TestType;
       using P = std::pair<int, C>;
       using T = std::pair<long, C>;
       T t(42, -42);
       P p(101, 101);
       C::reset_constructors();
       p = std::move(t);
       assert(C::constructed == 0);
       assert(C::assigned == 1);
       assert(C::copy_assigned == 0);
       assert(C::move_assigned == 1);
       assert(p.first == 42);
       assert(p.second.value == -42);
    }
}
