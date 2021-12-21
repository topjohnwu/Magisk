//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// UNSUPPORTED: libcpp-has-no-threads

// <memory>

// shared_ptr

// template<class T>
// bool
// atomic_is_lock_free(const shared_ptr<T>* p);

// UNSUPPORTED: c++98, c++03

#include <memory>
#include <cassert>

#include "test_macros.h"

int main()
{
    {
        const std::shared_ptr<int> p(new int(3));
        assert(std::atomic_is_lock_free(&p) == false);
    }
}
