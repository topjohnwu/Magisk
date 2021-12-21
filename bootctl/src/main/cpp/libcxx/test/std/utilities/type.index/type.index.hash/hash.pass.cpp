//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <typeindex>

// class type_index

// template <>
// struct hash<type_index>
//     : public unary_function<type_index, size_t>
// {
//     size_t operator()(type_index index) const;
// };

#include <typeindex>
#include <type_traits>
#include <cassert>

int main()
{
    typedef std::hash<std::type_index> H;
    static_assert((std::is_same<typename H::argument_type, std::type_index>::value), "" );
    static_assert((std::is_same<typename H::result_type, std::size_t>::value), "" );

    std::type_index t1 = typeid(int);
    assert(std::hash<std::type_index>()(t1) == t1.hash_code());
}
