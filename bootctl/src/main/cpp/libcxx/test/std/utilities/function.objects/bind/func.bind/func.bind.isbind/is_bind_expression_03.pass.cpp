//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <functional>

//-----------------------------------------------------------------------------
// TESTING template<class T> struct is_bind_expression
//
// bind is not implemented in C++03 so nothing is a bind expression. However
// for compatibility reasons the trait is_bind_expression should be available
// in C++03 and it should always return false.

#include <functional>

template <class T>
void test() {
    static_assert(!std::is_bind_expression<T>::value, "");
}

struct C {};

int main() {
    test<int>();
    test<void>();
    test<C>();
    test<C&>();
    test<C const&>();
    test<C*>();
    test<void()>();
    test<int(*)()>();
    test<int (C::*)()>();
    test<decltype(std::placeholders::_2)>();
}
