//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11
#include <functional>
#include <string>

template <class T>
struct is_transparent
{
private:
    struct two {char lx; char lxx;};
    template <class U> static two test(...);
    template <class U> static char test(typename U::is_transparent* = 0);
public:
    static const bool value = sizeof(test<T>(0)) == 1;
};


int main ()
{
    static_assert ( !is_transparent<std::less<int>>::value, "" );
    static_assert ( !is_transparent<std::less<std::string>>::value, "" );
    static_assert (  is_transparent<std::less<void>>::value, "" );
    static_assert (  is_transparent<std::less<>>::value, "" );

    static_assert ( !is_transparent<std::less_equal<int>>::value, "" );
    static_assert ( !is_transparent<std::less_equal<std::string>>::value, "" );
    static_assert (  is_transparent<std::less_equal<void>>::value, "" );
    static_assert (  is_transparent<std::less_equal<>>::value, "" );

    static_assert ( !is_transparent<std::equal_to<int>>::value, "" );
    static_assert ( !is_transparent<std::equal_to<std::string>>::value, "" );
    static_assert (  is_transparent<std::equal_to<void>>::value, "" );
    static_assert (  is_transparent<std::equal_to<>>::value, "" );

    static_assert ( !is_transparent<std::not_equal_to<int>>::value, "" );
    static_assert ( !is_transparent<std::not_equal_to<std::string>>::value, "" );
    static_assert (  is_transparent<std::not_equal_to<void>>::value, "" );
    static_assert (  is_transparent<std::not_equal_to<>>::value, "" );

    static_assert ( !is_transparent<std::greater<int>>::value, "" );
    static_assert ( !is_transparent<std::greater<std::string>>::value, "" );
    static_assert (  is_transparent<std::greater<void>>::value, "" );
    static_assert (  is_transparent<std::greater<>>::value, "" );

    static_assert ( !is_transparent<std::greater_equal<int>>::value, "" );
    static_assert ( !is_transparent<std::greater_equal<std::string>>::value, "" );
    static_assert (  is_transparent<std::greater_equal<void>>::value, "" );
    static_assert (  is_transparent<std::greater_equal<>>::value, "" );

    return 0;
}
