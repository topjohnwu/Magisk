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
// ValueType any_cast(any &&);

// Try and use the rvalue any_cast to cast to an lvalue reference

#include <any>

struct TestType {};
using std::any;
using std::any_cast;

// On platforms that do not support any_cast, an additional availability error
// is triggered by these tests.
// expected-error@any_cast_request_invalid_value_category.fail.cpp:* 0+ {{call to unavailable function 'any_cast': introduced in macOS 10.14}}

void test_const_lvalue_cast_request_non_const_lvalue()
{
    const any a;
    // expected-error-re@any:* {{static_assert failed{{.*}} "ValueType is required to be a const lvalue reference or a CopyConstructible type"}}
    // expected-error@any:* {{binding value of type 'const TestType' to reference to type 'TestType' drops 'const' qualifier}}
    any_cast<TestType &>(a); // expected-note {{requested here}}

    const any a2(42);
    // expected-error-re@any:* {{static_assert failed{{.*}} "ValueType is required to be a const lvalue reference or a CopyConstructible type"}}
    // expected-error@any:* {{binding value of type 'const int' to reference to type 'int' drops 'const' qualifier}}
    any_cast<int&>(a2); // expected-note {{requested here}}
}

void test_lvalue_any_cast_request_rvalue()
{
    any a;
    // expected-error-re@any:* {{static_assert failed{{.*}} "ValueType is required to be an lvalue reference or a CopyConstructible type"}}
    any_cast<TestType &&>(a); // expected-note {{requested here}}

    any a2(42);
    // expected-error-re@any:* {{static_assert failed{{.*}} "ValueType is required to be an lvalue reference or a CopyConstructible type"}}
    any_cast<int&&>(a2); // expected-note {{requested here}}
}

void test_rvalue_any_cast_request_lvalue()
{
    any a;
    // expected-error-re@any:* {{static_assert failed{{.*}} "ValueType is required to be an rvalue reference or a CopyConstructible type"}}
    // expected-error@any:* {{non-const lvalue reference to type 'TestType' cannot bind to a temporary}}
    any_cast<TestType &>(std::move(a)); // expected-note {{requested here}}

    // expected-error-re@any:* {{static_assert failed{{.*}} "ValueType is required to be an rvalue reference or a CopyConstructible type"}}
    // expected-error@any:* {{non-const lvalue reference to type 'int' cannot bind to a temporary}}
    any_cast<int&>(42);
}

int main()
{
    test_const_lvalue_cast_request_non_const_lvalue();
    test_lvalue_any_cast_request_rvalue();
    test_rvalue_any_cast_request_lvalue();
}
