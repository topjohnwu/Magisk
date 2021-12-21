//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// UNSUPPORTED: libcpp-has-no-threads
// UNSUPPORTED: c++98, c++03

// NOTE: atomic<> of a TriviallyCopyable class is wrongly rejected by older
// clang versions. It was fixed right before the llvm 3.5 release. See PR18097.
// XFAIL: apple-clang-6.0, clang-3.4, clang-3.3

// <atomic>

// constexpr atomic<T>::atomic(T value)

#include <atomic>
#include <type_traits>
#include <cassert>

#include "atomic_helpers.h"

struct UserType {
    int i;

    UserType() noexcept {}
    constexpr explicit UserType(int d) noexcept : i(d) {}

    friend bool operator==(const UserType& x, const UserType& y) {
        return x.i == y.i;
    }
};

template <class Tp>
struct TestFunc {
    void operator()() const {
        typedef std::atomic<Tp> Atomic;
        static_assert(std::is_literal_type<Atomic>::value, "");
        constexpr Tp t(42);
        {
            constexpr Atomic a(t);
            assert(a == t);
        }
        {
            constexpr Atomic a{t};
            assert(a == t);
        }
        {
            constexpr Atomic a = ATOMIC_VAR_INIT(t);
            assert(a == t);
        }
    }
};


int main()
{
    TestFunc<UserType>()();
    TestEachIntegralType<TestFunc>()();
}
