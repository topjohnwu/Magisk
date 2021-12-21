//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <array>

// void swap(array& a);

#include <array>
#include <cassert>

// std::array is explicitly allowed to be initialized with A a = { init-list };.
// Disable the missing braces warning for this reason.
#include "disable_missing_braces_warning.h"

int main() {
  {
    typedef double T;
    typedef std::array<const T, 0> C;
    C c = {};
    C c2 = {};
    // expected-error-re@array:* {{static_assert failed {{.*}}"cannot swap zero-sized array of type 'const T'"}}
    c.swap(c2); // expected-note {{requested here}}
  }
}
