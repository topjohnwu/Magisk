//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef EXPERIMENTAL_ANY_HELPERS_H
#define EXPERIMENTAL_ANY_HELPERS_H

#include <experimental/any>
#include <typeinfo>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

#if !defined(TEST_HAS_NO_RTTI)
#define RTTI_ASSERT(X) assert(X)
#else
#define RTTI_ASSERT(X)
#endif

template <class T>
  struct IsSmallObject
    : public std::integral_constant<bool
        , sizeof(T) <= (sizeof(void*)*3)
          && std::alignment_of<void*>::value
             % std::alignment_of<T>::value == 0
          && std::is_nothrow_move_constructible<T>::value
        >
  {};


// Return 'true' if 'Type' will be considered a small type by 'any'
template <class Type>
bool isSmallType() {
#if defined(_LIBCPP_VERSION)
    return std::experimental::__any_imp::_IsSmallObject<Type>::value;
#else
    return IsSmallObject<Type>::value;
#endif

}

// Assert that an object is empty. If the object used to contain an object
// of type 'LastType' check that it can no longer be accessed.
template <class LastType = int>
void assertEmpty(std::experimental::any const& a) {
    assert(a.empty());
    RTTI_ASSERT(a.type() == typeid(void));
    assert(std::experimental::any_cast<LastType const>(&a) == nullptr);
}

// Assert that an 'any' object stores the specified 'Type' and 'value'.
template <class Type>
_LIBCPP_AVAILABILITY_THROW_BAD_ANY_CAST
void assertContains(std::experimental::any const& a, int value = 1) {
    assert(!a.empty());
    RTTI_ASSERT(a.type() == typeid(Type));
    assert(std::experimental::any_cast<Type const &>(a).value == value);
}

// Modify the value of a "test type" stored within an any to the specified
// 'value'.
template <class Type>
_LIBCPP_AVAILABILITY_THROW_BAD_ANY_CAST
void modifyValue(std::experimental::any& a, int value) {
    assert(!a.empty());
    RTTI_ASSERT(a.type() == typeid(Type));
    std::experimental::any_cast<Type&>(a).value = value;
}

// A test type that will trigger the small object optimization within 'any'.
template <int Dummy = 0>
struct small_type
{
    static int count;
    static int copied;
    static int moved;
    static int const_copied;
    static int non_const_copied;

    static void reset() {
        small_type::copied = 0;
        small_type::moved = 0;
        small_type::const_copied = 0;
        small_type::non_const_copied = 0;
    }

    int value;

    explicit small_type(int val) : value(val) {
        ++count;
    }

    small_type(small_type const & other) throw() {
        value = other.value;
        ++count;
        ++copied;
        ++const_copied;
    }

    small_type(small_type& other) throw() {
        value = other.value;
        ++count;
        ++copied;
        ++non_const_copied;
    }

    small_type(small_type && other) throw() {
        value = other.value;
        other.value = 0;
        ++count;
        ++moved;
    }

    ~small_type() {
        value = -1;
        --count;
    }

private:
    small_type& operator=(small_type const&) = delete;
    small_type& operator=(small_type&&) = delete;
};

template <int Dummy>
int small_type<Dummy>::count = 0;

template <int Dummy>
int small_type<Dummy>::copied = 0;

template <int Dummy>
int small_type<Dummy>::moved = 0;

template <int Dummy>
int small_type<Dummy>::const_copied = 0;

template <int Dummy>
int small_type<Dummy>::non_const_copied = 0;

typedef small_type<> small;
typedef small_type<1> small1;
typedef small_type<2> small2;


// A test type that will NOT trigger the small object optimization in any.
template <int Dummy = 0>
struct large_type
{
    static int count;
    static int copied;
    static int moved;
    static int const_copied;
    static int non_const_copied;

    static void reset() {
        large_type::copied = 0;
        large_type::moved  = 0;
        large_type::const_copied = 0;
        large_type::non_const_copied = 0;
    }

    int value;

    large_type(int val) : value(val) {
        ++count;
        data[0] = 0;
    }

    large_type(large_type const & other) {
        value = other.value;
        ++count;
        ++copied;
        ++const_copied;
    }

    large_type(large_type & other) {
        value = other.value;
        ++count;
        ++copied;
        ++non_const_copied;
    }

    large_type(large_type && other) {
        value = other.value;
        other.value = 0;
        ++count;
        ++moved;
    }

    ~large_type()  {
        value = 0;
        --count;
    }

private:
    large_type& operator=(large_type const&) = delete;
    large_type& operator=(large_type &&) = delete;
    int data[10];
};

template <int Dummy>
int large_type<Dummy>::count = 0;

template <int Dummy>
int large_type<Dummy>::copied = 0;

template <int Dummy>
int large_type<Dummy>::moved = 0;

template <int Dummy>
int large_type<Dummy>::const_copied = 0;

template <int Dummy>
int large_type<Dummy>::non_const_copied = 0;

typedef large_type<> large;
typedef large_type<1> large1;
typedef large_type<2> large2;

// The exception type thrown by 'small_throws_on_copy', 'large_throws_on_copy'
// and 'throws_on_move'.
struct my_any_exception {};

void throwMyAnyExpression() {
#if !defined(TEST_HAS_NO_EXCEPTIONS)
        throw my_any_exception();
#else
        assert(false && "Exceptions are disabled");
#endif
}

// A test type that will trigger the small object optimization within 'any'.
// this type throws if it is copied.
struct small_throws_on_copy
{
    static int count;
    int value;

    explicit small_throws_on_copy(int val = 0) : value(val) {
        ++count;
    }

    small_throws_on_copy(small_throws_on_copy const &) {
        throwMyAnyExpression();
    }

    small_throws_on_copy(small_throws_on_copy && other) throw() {
        value = other.value;
        ++count;
    }

    ~small_throws_on_copy() {
        --count;
    }
private:
    small_throws_on_copy& operator=(small_throws_on_copy const&) = delete;
    small_throws_on_copy& operator=(small_throws_on_copy &&) = delete;
};

int small_throws_on_copy::count = 0;

// A test type that will NOT trigger the small object optimization within 'any'.
// this type throws if it is copied.
struct large_throws_on_copy
{
    static int count;
    int value = 0;

    explicit large_throws_on_copy(int val = 0) : value(val) {
        data[0] = 0;
        ++count;
    }

    large_throws_on_copy(large_throws_on_copy const &) {
         throwMyAnyExpression();
    }

    large_throws_on_copy(large_throws_on_copy && other) throw() {
        value = other.value;
        ++count;
    }

    ~large_throws_on_copy() {
        --count;
    }

private:
    large_throws_on_copy& operator=(large_throws_on_copy const&) = delete;
    large_throws_on_copy& operator=(large_throws_on_copy &&) = delete;
    int data[10];
};

int large_throws_on_copy::count = 0;

// A test type that throws when it is moved. This object will NOT trigger
// the small object optimization in 'any'.
struct throws_on_move
{
    static int count;
    int value;

    explicit throws_on_move(int val = 0) : value(val) { ++count; }

    throws_on_move(throws_on_move const & other) {
        value = other.value;
        ++count;
    }

    throws_on_move(throws_on_move &&) {
        throwMyAnyExpression();
    }

    ~throws_on_move() {
        --count;
    }
private:
    throws_on_move& operator=(throws_on_move const&) = delete;
    throws_on_move& operator=(throws_on_move &&) = delete;
};

int throws_on_move::count = 0;


#endif
