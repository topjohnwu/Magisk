//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <memory>

// template <class T>
// struct hash<shared_ptr<T>>
// {
//     typedef shared_ptr<T>    argument_type;
//     typedef size_t           result_type;
//     size_t operator()(const shared_ptr<T>& p) const;
// };

#include <memory>
#include <cassert>

#if TEST_STD_VER >= 11
#include "poisoned_hash_helper.hpp"

struct A {};
#endif

int main()
{
  {
    int* ptr = new int;
    std::shared_ptr<int> p(ptr);
    std::hash<std::shared_ptr<int> > f;
    std::size_t h = f(p);
    assert(h == std::hash<int*>()(ptr));
  }
#if TEST_STD_VER >= 11
  {
    test_hash_enabled_for_type<std::shared_ptr<int>>();
    test_hash_enabled_for_type<std::shared_ptr<A>>();
  }
#endif
}
