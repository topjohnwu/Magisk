//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//

// __is_referenceable<Tp>
//
// [defns.referenceable] defines "a referenceable type" as:
// An object type, a function type that does not have cv-qualifiers
//    or a ref-qualifier, or a reference type.
//

#include <type_traits>
#include <cassert>

#include "test_macros.h"

struct Foo {};

static_assert((!std::__is_referenceable<void>::value), "");
static_assert(( std::__is_referenceable<int>::value), "");
static_assert(( std::__is_referenceable<int[3]>::value), "");
static_assert(( std::__is_referenceable<int[]>::value), "");
static_assert(( std::__is_referenceable<int &>::value), "");
static_assert(( std::__is_referenceable<const int &>::value), "");
static_assert(( std::__is_referenceable<int *>::value), "");
static_assert(( std::__is_referenceable<const int *>::value), "");
static_assert(( std::__is_referenceable<Foo>::value), "");
static_assert(( std::__is_referenceable<const Foo>::value), "");
static_assert(( std::__is_referenceable<Foo &>::value), "");
static_assert(( std::__is_referenceable<const Foo &>::value), "");
#if TEST_STD_VER >= 11
static_assert(( std::__is_referenceable<Foo &&>::value), "");
static_assert(( std::__is_referenceable<const Foo &&>::value), "");
#endif

static_assert(( std::__is_referenceable<int   __attribute__((__vector_size__( 8)))>::value), "");
static_assert(( std::__is_referenceable<const int   __attribute__((__vector_size__( 8)))>::value), "");
static_assert(( std::__is_referenceable<float __attribute__((__vector_size__(16)))>::value), "");
static_assert(( std::__is_referenceable<const float __attribute__((__vector_size__(16)))>::value), "");

// Functions without cv-qualifiers are referenceable
static_assert(( std::__is_referenceable<void ()>::value), "");
#if TEST_STD_VER >= 11
static_assert((!std::__is_referenceable<void () const>::value), "");
static_assert((!std::__is_referenceable<void () &>::value), "");
static_assert((!std::__is_referenceable<void () const &>::value), "");
static_assert((!std::__is_referenceable<void () &&>::value), "");
static_assert((!std::__is_referenceable<void () const &&>::value), "");
#endif

static_assert(( std::__is_referenceable<void (int)>::value), "");
#if TEST_STD_VER >= 11
static_assert((!std::__is_referenceable<void (int) const>::value), "");
static_assert((!std::__is_referenceable<void (int) &>::value), "");
static_assert((!std::__is_referenceable<void (int) const &>::value), "");
static_assert((!std::__is_referenceable<void (int) &&>::value), "");
static_assert((!std::__is_referenceable<void (int) const &&>::value), "");
#endif

static_assert(( std::__is_referenceable<void (int, float)>::value), "");
#if TEST_STD_VER >= 11
static_assert((!std::__is_referenceable<void (int, float) const>::value), "");
static_assert((!std::__is_referenceable<void (int, float) &>::value), "");
static_assert((!std::__is_referenceable<void (int, float) const &>::value), "");
static_assert((!std::__is_referenceable<void (int, float) &&>::value), "");
static_assert((!std::__is_referenceable<void (int, float) const &&>::value), "");
#endif

static_assert(( std::__is_referenceable<void (int, float, Foo &)>::value), "");
#if TEST_STD_VER >= 11
static_assert((!std::__is_referenceable<void (int, float, Foo &) const>::value), "");
static_assert((!std::__is_referenceable<void (int, float, Foo &) &>::value), "");
static_assert((!std::__is_referenceable<void (int, float, Foo &) const &>::value), "");
static_assert((!std::__is_referenceable<void (int, float, Foo &) &&>::value), "");
static_assert((!std::__is_referenceable<void (int, float, Foo &) const &&>::value), "");
#endif

static_assert(( std::__is_referenceable<void (...)>::value), "");
#if TEST_STD_VER >= 11
static_assert((!std::__is_referenceable<void (...) const>::value), "");
static_assert((!std::__is_referenceable<void (...) &>::value), "");
static_assert((!std::__is_referenceable<void (...) const &>::value), "");
static_assert((!std::__is_referenceable<void (...) &&>::value), "");
static_assert((!std::__is_referenceable<void (...) const &&>::value), "");
#endif

static_assert(( std::__is_referenceable<void (int, ...)>::value), "");
#if TEST_STD_VER >= 11
static_assert((!std::__is_referenceable<void (int, ...) const>::value), "");
static_assert((!std::__is_referenceable<void (int, ...) &>::value), "");
static_assert((!std::__is_referenceable<void (int, ...) const &>::value), "");
static_assert((!std::__is_referenceable<void (int, ...) &&>::value), "");
static_assert((!std::__is_referenceable<void (int, ...) const &&>::value), "");
#endif

static_assert(( std::__is_referenceable<void (int, float, ...)>::value), "");
#if TEST_STD_VER >= 11
static_assert((!std::__is_referenceable<void (int, float, ...) const>::value), "");
static_assert((!std::__is_referenceable<void (int, float, ...) &>::value), "");
static_assert((!std::__is_referenceable<void (int, float, ...) const &>::value), "");
static_assert((!std::__is_referenceable<void (int, float, ...) &&>::value), "");
static_assert((!std::__is_referenceable<void (int, float, ...) const &&>::value), "");
#endif

static_assert(( std::__is_referenceable<void (int, float, Foo &, ...)>::value), "");
#if TEST_STD_VER >= 11
static_assert((!std::__is_referenceable<void (int, float, Foo &, ...) const>::value), "");
static_assert((!std::__is_referenceable<void (int, float, Foo &, ...) &>::value), "");
static_assert((!std::__is_referenceable<void (int, float, Foo &, ...) const &>::value), "");
static_assert((!std::__is_referenceable<void (int, float, Foo &, ...) &&>::value), "");
static_assert((!std::__is_referenceable<void (int, float, Foo &, ...) const &&>::value), "");
#endif

// member functions with or without cv-qualifiers are referenceable
static_assert(( std::__is_referenceable<void (Foo::*)()>::value), "");
static_assert(( std::__is_referenceable<void (Foo::*)() const>::value), "");
#if TEST_STD_VER >= 11
static_assert(( std::__is_referenceable<void (Foo::*)() &>::value), "");
static_assert(( std::__is_referenceable<void (Foo::*)() const &>::value), "");
static_assert(( std::__is_referenceable<void (Foo::*)() &&>::value), "");
static_assert(( std::__is_referenceable<void (Foo::*)() const &&>::value), "");
#endif

static_assert(( std::__is_referenceable<void (Foo::*)(int)>::value), "");
static_assert(( std::__is_referenceable<void (Foo::*)(int) const>::value), "");
#if TEST_STD_VER >= 11
static_assert(( std::__is_referenceable<void (Foo::*)(int) &>::value), "");
static_assert(( std::__is_referenceable<void (Foo::*)(int) const &>::value), "");
static_assert(( std::__is_referenceable<void (Foo::*)(int) &&>::value), "");
static_assert(( std::__is_referenceable<void (Foo::*)(int) const &&>::value), "");
#endif

static_assert(( std::__is_referenceable<void (Foo::*)(int, float)>::value), "");
static_assert(( std::__is_referenceable<void (Foo::*)(int, float) const>::value), "");
#if TEST_STD_VER >= 11
static_assert(( std::__is_referenceable<void (Foo::*)(int, float) &>::value), "");
static_assert(( std::__is_referenceable<void (Foo::*)(int, float) const &>::value), "");
static_assert(( std::__is_referenceable<void (Foo::*)(int, float) &&>::value), "");
static_assert(( std::__is_referenceable<void (Foo::*)(int, float) const &&>::value), "");
#endif

static_assert(( std::__is_referenceable<void (Foo::*)(int, float, Foo &)>::value), "");
static_assert(( std::__is_referenceable<void (Foo::*)(int, float, Foo &) const>::value), "");
#if TEST_STD_VER >= 11
static_assert(( std::__is_referenceable<void (Foo::*)(int, float, Foo &) &>::value), "");
static_assert(( std::__is_referenceable<void (Foo::*)(int, float, Foo &) const &>::value), "");
static_assert(( std::__is_referenceable<void (Foo::*)(int, float, Foo &) &&>::value), "");
static_assert(( std::__is_referenceable<void (Foo::*)(int, float, Foo &) const &&>::value), "");
#endif

static_assert(( std::__is_referenceable<void (Foo::*)(...)>::value), "");
static_assert(( std::__is_referenceable<void (Foo::*)(...) const>::value), "");
#if TEST_STD_VER >= 11
static_assert(( std::__is_referenceable<void (Foo::*)(...) &>::value), "");
static_assert(( std::__is_referenceable<void (Foo::*)(...) const &>::value), "");
static_assert(( std::__is_referenceable<void (Foo::*)(...) &&>::value), "");
static_assert(( std::__is_referenceable<void (Foo::*)(...) const &&>::value), "");
#endif

static_assert(( std::__is_referenceable<void (Foo::*)(int, ...)>::value), "");
static_assert(( std::__is_referenceable<void (Foo::*)(int, ...) const>::value), "");
#if TEST_STD_VER >= 11
static_assert(( std::__is_referenceable<void (Foo::*)(int, ...) &>::value), "");
static_assert(( std::__is_referenceable<void (Foo::*)(int, ...) const &>::value), "");
static_assert(( std::__is_referenceable<void (Foo::*)(int, ...) &&>::value), "");
static_assert(( std::__is_referenceable<void (Foo::*)(int, ...) const &&>::value), "");
#endif

static_assert(( std::__is_referenceable<void (Foo::*)(int, float, ...)>::value), "");
static_assert(( std::__is_referenceable<void (Foo::*)(int, float, ...) const>::value), "");
#if TEST_STD_VER >= 11
static_assert(( std::__is_referenceable<void (Foo::*)(int, float, ...) &>::value), "");
static_assert(( std::__is_referenceable<void (Foo::*)(int, float, ...) const &>::value), "");
static_assert(( std::__is_referenceable<void (Foo::*)(int, float, ...) &&>::value), "");
static_assert(( std::__is_referenceable<void (Foo::*)(int, float, ...) const &&>::value), "");
#endif

static_assert(( std::__is_referenceable<void (Foo::*)(int, float, Foo &, ...)>::value), "");
static_assert(( std::__is_referenceable<void (Foo::*)(int, float, Foo &, ...) const>::value), "");
#if TEST_STD_VER >= 11
static_assert(( std::__is_referenceable<void (Foo::*)(int, float, Foo &, ...) &>::value), "");
static_assert(( std::__is_referenceable<void (Foo::*)(int, float, Foo &, ...) const &>::value), "");
static_assert(( std::__is_referenceable<void (Foo::*)(int, float, Foo &, ...) &&>::value), "");
static_assert(( std::__is_referenceable<void (Foo::*)(int, float, Foo &, ...) const &&>::value), "");
#endif

int main () {}
