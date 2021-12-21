//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <memory>

// shared_ptr

// template<class T, class... Args> shared_ptr<T> make_shared(Args&&... args);

#include <memory>
#include <cassert>

#include "test_macros.h"

struct S {
protected:
   S () {};  // ctor is protected
};

int main()
{
    std::shared_ptr<S> p = std::make_shared<S>();  // expected-error-re@memory:* {{static_assert failed{{.*}} "Can't construct object in make_shared"}}
}
