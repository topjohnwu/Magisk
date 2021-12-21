//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <typeindex>

// struct hash<type_index>
//     : public unary_function<type_index, size_t>
// {
//     size_t operator()(type_index index) const;
// };

#include <typeindex>
#include <type_traits>

#include "test_macros.h"
#if TEST_STD_VER >= 11
#include "poisoned_hash_helper.hpp"
#endif

int main()
{
  {
    typedef std::hash<std::type_index> H;
    static_assert((std::is_same<typename H::argument_type, std::type_index>::value), "" );
    static_assert((std::is_same<typename H::result_type, std::size_t>::value), "" );
  }
#if TEST_STD_VER >= 11
  {
    test_hash_enabled_for_type<std::type_index>(std::type_index(typeid(int)));
  }
#endif
}
