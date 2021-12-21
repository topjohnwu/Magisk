//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11, c++14

// <any>

// template <class ValueType>
// ValueType const any_cast(any const&);
//
// template <class ValueType>
// ValueType any_cast(any &);
//
// template <class ValueType>
// ValueType any_cast(any &&);

// Test instantiating the any_cast with a non-copyable type.

#include <any>

using std::any;
using std::any_cast;

struct no_copy
{
    no_copy() {}
    no_copy(no_copy &&) {}
    no_copy(no_copy const &) = delete;
};

struct no_move {
  no_move() {}
  no_move(no_move&&) = delete;
  no_move(no_move const&) {}
};

// On platforms that do not support any_cast, an additional availability error
// is triggered by these tests.
// expected-error@not_copy_constructible.fail.cpp:* 0+ {{call to unavailable function 'any_cast': introduced in macOS 10.14}}

int main() {
    any a;
    // expected-error-re@any:* {{static_assert failed{{.*}} "ValueType is required to be an lvalue reference or a CopyConstructible type"}}
    // expected-error@any:* {{static_cast from 'no_copy' to 'no_copy' uses deleted function}}
    any_cast<no_copy>(static_cast<any&>(a)); // expected-note {{requested here}}

    // expected-error-re@any:* {{static_assert failed{{.*}} "ValueType is required to be a const lvalue reference or a CopyConstructible type"}}
    // expected-error@any:* {{static_cast from 'const no_copy' to 'no_copy' uses deleted function}}
    any_cast<no_copy>(static_cast<any const&>(a)); // expected-note {{requested here}}

    any_cast<no_copy>(static_cast<any &&>(a)); // OK

    // expected-error-re@any:* {{static_assert failed{{.*}} "ValueType is required to be an rvalue reference or a CopyConstructible type"}}
    // expected-error@any:* {{static_cast from 'typename remove_reference<no_move &>::type' (aka 'no_move') to 'no_move' uses deleted function}}
    any_cast<no_move>(static_cast<any &&>(a));
}
