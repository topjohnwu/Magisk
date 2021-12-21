//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string>
//   ... manipulating sequences of any non-array trivial standard-layout types.

#include <string>
#include "test_traits.h"

struct NotTrivial {
    NotTrivial() : value(3) {}
    int value;
};

struct NotStandardLayout {
public:
    NotStandardLayout() : one(1), two(2) {}
    int sum() const { return one + two; } // silences "unused field 'two' warning"
    int one;
private:
    int two;
};

int main()
{
    {
//  array
    typedef char C[3];
    static_assert(std::is_array<C>::value, "");
    std::basic_string<C, test_traits<C> > s;
//  expected-error-re@string:* {{static_assert failed{{.*}} "Character type of basic_string must not be an array"}}
    }

    {
//  not trivial
    static_assert(!std::is_trivial<NotTrivial>::value, "");
    std::basic_string<NotTrivial, test_traits<NotTrivial> > s;
//  expected-error-re@string:* {{static_assert failed{{.*}} "Character type of basic_string must be trivial"}}
    }

    {
//  not standard layout
    static_assert(!std::is_standard_layout<NotStandardLayout>::value, "");
    std::basic_string<NotStandardLayout, test_traits<NotStandardLayout> > s;
//  expected-error-re@string:* {{static_assert failed{{.*}} "Character type of basic_string must be standard-layout"}}
    }
}
