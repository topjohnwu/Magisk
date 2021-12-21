//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <memory>

// allocator:
// size_type max_size() const throw();

#include <memory>
#include <limits>
#include <cstddef>
#include <cassert>

int new_called = 0;

int main()
{
    const std::allocator<int> a;
    std::size_t M = a.max_size();
    assert(M > 0xFFFF && M <= (std::numeric_limits<std::size_t>::max() / sizeof(int)));
}
