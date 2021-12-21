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

// ~optional();

#include <optional>
#include <type_traits>
#include <cassert>

using std::optional;

struct PODType {
  int value;
  int value2;
};

class X
{
public:
    static bool dtor_called;
    X() = default;
    ~X() {dtor_called = true;}
};

bool X::dtor_called = false;

int main()
{
    {
        typedef int T;
        static_assert(std::is_trivially_destructible<T>::value, "");
        static_assert(std::is_trivially_destructible<optional<T>>::value, "");
        static_assert(std::is_literal_type<optional<T>>::value, "");
    }
    {
        typedef double T;
        static_assert(std::is_trivially_destructible<T>::value, "");
        static_assert(std::is_trivially_destructible<optional<T>>::value, "");
        static_assert(std::is_literal_type<optional<T>>::value, "");
    }
    {
        typedef PODType T;
        static_assert(std::is_trivially_destructible<T>::value, "");
        static_assert(std::is_trivially_destructible<optional<T>>::value, "");
        static_assert(std::is_literal_type<optional<T>>::value, "");
    }
    {
        typedef X T;
        static_assert(!std::is_trivially_destructible<T>::value, "");
        static_assert(!std::is_trivially_destructible<optional<T>>::value, "");
        static_assert(!std::is_literal_type<optional<T>>::value, "");
        {
            X x;
            optional<X> opt{x};
            assert(X::dtor_called == false);
        }
        assert(X::dtor_called == true);
    }
}
