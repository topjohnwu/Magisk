//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11
// <utility>

// class make_integer_sequence

#include <tuple>
#include <utility>
#include <type_traits>
#include <cassert>

template <typename AtContainer, typename T, T... I>
auto extract ( const AtContainer &t, const std::integer_sequence<T, I...> )
-> decltype ( std::make_tuple ( std::get<I>(t)... ))
{     return  std::make_tuple ( std::get<I>(t)... ); }

int main()
{
//  Make a couple of sequences
    using int3    = std::make_integer_sequence<int, 3>;     // generates int:    0,1,2
    using size7   = std::make_integer_sequence<size_t, 7>;  // generates size_t: 0,1,2,3,4,5,6
    using size4   = std::make_index_sequence<4>;            // generates size_t: 0,1,2,3
    using size2   = std::index_sequence_for<int, size_t>;   // generates size_t: 0,1
    using intmix  = std::integer_sequence<int, 9, 8, 7, 2>; // generates int:    9,8,7,2
    using sizemix = std::index_sequence<1, 1, 2, 3, 5>;     // generates size_t: 1,1,2,3,5

//  Make sure they're what we expect
    static_assert ( std::is_same<int3::value_type, int>::value, "int3 type wrong" );
    static_assert ( int3::size () == 3, "int3 size wrong" );

    static_assert ( std::is_same<size7::value_type, size_t>::value, "size7 type wrong" );
    static_assert ( size7::size () == 7, "size7 size wrong" );

    static_assert ( std::is_same<size4::value_type, size_t>::value, "size4 type wrong" );
    static_assert ( size4::size () == 4, "size4 size wrong" );

    static_assert ( std::is_same<size2::value_type, size_t>::value, "size2 type wrong" );
    static_assert ( size2::size () == 2, "size2 size wrong" );

    static_assert ( std::is_same<intmix::value_type, int>::value, "intmix type wrong" );
    static_assert ( intmix::size () == 4, "intmix size wrong" );

    static_assert ( std::is_same<sizemix::value_type, size_t>::value, "sizemix type wrong" );
    static_assert ( sizemix::size () == 5, "sizemix size wrong" );

    auto tup = std::make_tuple ( 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20 );

//  Use them
    auto t3 = extract ( tup, int3() );
    static_assert ( std::tuple_size<decltype(t3)>::value == int3::size (), "t3 size wrong");
    assert ( t3 == std::make_tuple ( 10, 11, 12 ));

    auto t7 = extract ( tup, size7 ());
    static_assert ( std::tuple_size<decltype(t7)>::value == size7::size (), "t7 size wrong");
    assert ( t7 == std::make_tuple ( 10, 11, 12, 13, 14, 15, 16 ));

    auto t4 = extract ( tup, size4 ());
    static_assert ( std::tuple_size<decltype(t4)>::value == size4::size (), "t4 size wrong");
    assert ( t4 == std::make_tuple ( 10, 11, 12, 13 ));

    auto t2 = extract ( tup, size2 ());
    static_assert ( std::tuple_size<decltype(t2)>::value == size2::size (), "t2 size wrong");
    assert ( t2 == std::make_tuple ( 10, 11 ));

    auto tintmix = extract ( tup, intmix ());
    static_assert ( std::tuple_size<decltype(tintmix)>::value == intmix::size (), "tintmix size wrong");
    assert ( tintmix == std::make_tuple ( 19, 18, 17, 12 ));

    auto tsizemix = extract ( tup, sizemix ());
    static_assert ( std::tuple_size<decltype(tsizemix)>::value == sizemix::size (), "tsizemix size wrong");
    assert ( tsizemix == std::make_tuple ( 11, 11, 12, 13, 15 ));
}
