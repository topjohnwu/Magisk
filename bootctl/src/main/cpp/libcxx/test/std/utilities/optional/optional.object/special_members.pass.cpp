//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11, c++14

// <optional>

// Make sure we properly generate special member functions for optional<T>
// based on the properties of T itself.

#include <optional>
#include <type_traits>

#include "archetypes.hpp"


template <class T>
struct SpecialMemberTest {
    using O = std::optional<T>;

    static_assert(std::is_default_constructible_v<O>,
        "optional is always default constructible.");

    static_assert(std::is_copy_constructible_v<O> == std::is_copy_constructible_v<T>,
        "optional<T> is copy constructible if and only if T is copy constructible.");

    static_assert(std::is_move_constructible_v<O> ==
        (std::is_copy_constructible_v<T> || std::is_move_constructible_v<T>),
        "optional<T> is move constructible if and only if T is copy or move constructible.");

    static_assert(std::is_copy_assignable_v<O> ==
        (std::is_copy_constructible_v<T> && std::is_copy_assignable_v<T>),
        "optional<T> is copy assignable if and only if T is both copy "
        "constructible and copy assignable.");

    static_assert(std::is_move_assignable_v<O> ==
        ((std::is_move_constructible_v<T> && std::is_move_assignable_v<T>) ||
         (std::is_copy_constructible_v<T> && std::is_copy_assignable_v<T>)),
        "optional<T> is move assignable if and only if T is both move constructible and "
        "move assignable, or both copy constructible and copy assignable.");
};

template <class ...Args> static void sink(Args&&...) {}

template <class ...TestTypes>
struct DoTestsMetafunction {
    DoTestsMetafunction() { sink(SpecialMemberTest<TestTypes>{}...); }
};

int main() {
    sink(
        ImplicitTypes::ApplyTypes<DoTestsMetafunction>{},
        ExplicitTypes::ApplyTypes<DoTestsMetafunction>{},
        NonLiteralTypes::ApplyTypes<DoTestsMetafunction>{},
        NonTrivialTypes::ApplyTypes<DoTestsMetafunction>{}
    );
}
