//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <memory>

// class bad_weak_ptr
//     : public std::exception
// {
// public:
//     bad_weak_ptr();
// };

#include <memory>
#include <type_traits>
#include <cassert>
#include <cstring>

int main()
{
    static_assert((std::is_base_of<std::exception, std::bad_weak_ptr>::value), "");
    std::bad_weak_ptr e;
    std::bad_weak_ptr e2 = e;
    e2 = e;
    assert(std::strcmp(e.what(), "bad_weak_ptr") == 0);
}
