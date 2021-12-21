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

// template <class T> struct hash<optional<T>>;

#include <optional>
#include <string>
#include <memory>
#include <cassert>

#include "poisoned_hash_helper.hpp"

struct A {};
struct B {};

namespace std {

template <>
struct hash<B> {
  size_t operator()(B const&) TEST_NOEXCEPT_FALSE { return 0; }
};

}

int main()
{
    using std::optional;
    const std::size_t nullopt_hash =
        std::hash<optional<double>>{}(optional<double>{});


    {
        optional<B> opt;
        ASSERT_NOT_NOEXCEPT(std::hash<optional<B>>()(opt));
        ASSERT_NOT_NOEXCEPT(std::hash<optional<const B>>()(opt));
    }

    {
        typedef int T;
        optional<T> opt;
        assert(std::hash<optional<T>>{}(opt) == nullopt_hash);
        opt = 2;
        assert(std::hash<optional<T>>{}(opt) == std::hash<T>{}(*opt));
    }
    {
        typedef std::string T;
        optional<T> opt;
        assert(std::hash<optional<T>>{}(opt) == nullopt_hash);
        opt = std::string("123");
        assert(std::hash<optional<T>>{}(opt) == std::hash<T>{}(*opt));
    }
    {
        typedef std::unique_ptr<int> T;
        optional<T> opt;
        assert(std::hash<optional<T>>{}(opt) == nullopt_hash);
        opt = std::unique_ptr<int>(new int(3));
        assert(std::hash<optional<T>>{}(opt) == std::hash<T>{}(*opt));
    }
    {
      test_hash_enabled_for_type<std::optional<int> >();
      test_hash_enabled_for_type<std::optional<int*> >();
      test_hash_enabled_for_type<std::optional<const int> >();
      test_hash_enabled_for_type<std::optional<int* const> >();

      test_hash_disabled_for_type<std::optional<A>>();
      test_hash_disabled_for_type<std::optional<const A>>();

      test_hash_enabled_for_type<std::optional<B>>();
      test_hash_enabled_for_type<std::optional<const B>>();
    }
}
