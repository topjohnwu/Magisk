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
// ValueType any_cast(any const &);

// Try and cast away const.

#include <any>

struct TestType {};
struct TestType2 {};

// On platforms that do not support any_cast, an additional availability error
// is triggered by these tests.
// expected-error@const_correctness.fail.cpp:* 0+ {{call to unavailable function 'any_cast': introduced in macOS 10.14}}

int main()
{
    using std::any;
    using std::any_cast;

    any a;

    // expected-error@any:* {{binding value of type 'const TestType' to reference to type 'TestType' drops 'const' qualifier}}
    // expected-error-re@any:* {{static_assert failed{{.*}} "ValueType is required to be a const lvalue reference or a CopyConstructible type"}}
    any_cast<TestType &>(static_cast<any const&>(a)); // expected-note {{requested here}}

    // expected-error@any:* {{cannot cast from lvalue of type 'const TestType' to rvalue reference type 'TestType &&'; types are not compatible}}
    // expected-error-re@any:* {{static_assert failed{{.*}} "ValueType is required to be a const lvalue reference or a CopyConstructible type"}}
    any_cast<TestType &&>(static_cast<any const&>(a)); // expected-note {{requested here}}

    // expected-error@any:* {{binding value of type 'const TestType2' to reference to type 'TestType2' drops 'const' qualifier}}
    // expected-error-re@any:* {{static_assert failed{{.*}} "ValueType is required to be a const lvalue reference or a CopyConstructible type"}}
    any_cast<TestType2 &>(static_cast<any const&&>(a)); // expected-note {{requested here}}

    // expected-error@any:* {{cannot cast from lvalue of type 'const TestType2' to rvalue reference type 'TestType2 &&'; types are not compatible}}
    // expected-error-re@any:* {{static_assert failed{{.*}} "ValueType is required to be a const lvalue reference or a CopyConstructible type"}}
    any_cast<TestType2 &&>(static_cast<any const&&>(a)); // expected-note {{requested here}}
}
