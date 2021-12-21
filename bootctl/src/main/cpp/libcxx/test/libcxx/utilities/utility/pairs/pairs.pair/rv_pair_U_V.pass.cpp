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

// template <class U, class V> pair(pair<U, V>&& p);

#include <type_traits>
#include <utility>


struct ExplicitT {
    constexpr explicit ExplicitT(int x) : value(x) {}
    int value;
};

struct ImplicitT {
    constexpr ImplicitT(int x) : value(x) {}
    int value;
};

struct ExplicitNothrowT {
    explicit ExplicitNothrowT(int x) noexcept : value(x) {}
    int value;
};

struct ImplicitNothrowT {
    ImplicitNothrowT(int x) noexcept : value(x) {}
    int value;
};

int main() {
    { // explicit noexcept test
        static_assert(!std::is_nothrow_constructible<std::pair<ExplicitT, ExplicitT>,
                                                     std::pair<int, int>&&>::value, "");
        static_assert(!std::is_nothrow_constructible<std::pair<ExplicitNothrowT, ExplicitT>,
                                                     std::pair<int, int>&&>::value, "");
        static_assert(!std::is_nothrow_constructible<std::pair<ExplicitT, ExplicitNothrowT>,
                                                     std::pair<int, int>&&>::value, "");
        static_assert( std::is_nothrow_constructible<std::pair<ExplicitNothrowT, ExplicitNothrowT>,
                                                     std::pair<int, int>&&>::value, "");
    }
    { // implicit noexcept test
        static_assert(!std::is_nothrow_constructible<std::pair<ImplicitT, ImplicitT>,
                                                     std::pair<int, int>&&>::value, "");
        static_assert(!std::is_nothrow_constructible<std::pair<ImplicitNothrowT, ImplicitT>,
                                                     std::pair<int, int>&&>::value, "");
        static_assert(!std::is_nothrow_constructible<std::pair<ImplicitT, ImplicitNothrowT>,
                                                     std::pair<int, int>&&>::value, "");
        static_assert( std::is_nothrow_constructible<std::pair<ImplicitNothrowT, ImplicitNothrowT>,
                                                     std::pair<int, int>&&>::value, "");
    }
}
