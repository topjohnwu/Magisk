//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <functional>

// template<CopyConstructible Fn, CopyConstructible... Types>
//   unspecified bind(Fn, Types...);
// template<Returnable R, CopyConstructible Fn, CopyConstructible... Types>
//   unspecified bind(Fn, Types...);

#include <functional>
#include <cassert>

template <class R, class F>
void
test(F f, R expected)
{
    assert(f() == expected);
}

template <class R, class F>
void
test_const(const F& f, R expected)
{
    assert(f() == expected);
}

int f() {return 1;}

struct A_int_0
{
    int operator()() {return 4;}
    int operator()() const {return 5;}
};

int main()
{
    test(std::bind(f), 1);
    test(std::bind(&f), 1);
    test(std::bind(A_int_0()), 4);
    test_const(std::bind(A_int_0()), 5);

    test(std::bind<int>(f), 1);
    test(std::bind<int>(&f), 1);
    test(std::bind<int>(A_int_0()), 4);
    test_const(std::bind<int>(A_int_0()), 5);
}
