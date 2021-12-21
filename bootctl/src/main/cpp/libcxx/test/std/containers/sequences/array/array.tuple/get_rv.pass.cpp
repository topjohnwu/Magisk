//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <array>

// template <size_t I, class T, size_t N> T&& get(array<T, N>&& a);

// UNSUPPORTED: c++98, c++03

#include <array>
#include <memory>
#include <utility>
#include <cassert>

// std::array is explicitly allowed to be initialized with A a = { init-list };.
// Disable the missing braces warning for this reason.
#include "disable_missing_braces_warning.h"

int main()
{

    {
        typedef std::unique_ptr<double> T;
        typedef std::array<T, 1> C;
        C c = {std::unique_ptr<double>(new double(3.5))};
        T t = std::get<0>(std::move(c));
        assert(*t == 3.5);
    }
}
