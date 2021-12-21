//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <filesystem>

// class directory_iterator

// directory_iterator::directory_iterator() noexcept


#include "filesystem_include.hpp"
#include <type_traits>
#include <cassert>

#include "test_macros.h"


int main() {
    {
        static_assert(std::is_nothrow_default_constructible<fs::directory_iterator>::value, "");
    }
    {
        fs::directory_iterator d1;
        const fs::directory_iterator d2;
        assert(d1 == d2);
    }
}
