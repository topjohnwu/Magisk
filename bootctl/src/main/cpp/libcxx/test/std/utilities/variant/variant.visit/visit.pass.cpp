// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11, c++14

// XFAIL: availability=macosx10.13
// XFAIL: availability=macosx10.12
// XFAIL: availability=macosx10.11
// XFAIL: availability=macosx10.10
// XFAIL: availability=macosx10.9
// XFAIL: availability=macosx10.8
// XFAIL: availability=macosx10.7

// <variant>
// template <class Visitor, class... Variants>
// constexpr see below visit(Visitor&& vis, Variants&&... vars);

#include <cassert>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>

#include "test_macros.h"
#include "type_id.h"
#include "variant_test_helpers.hpp"

enum CallType : unsigned {
  CT_None,
  CT_NonConst = 1,
  CT_Const = 2,
  CT_LValue = 4,
  CT_RValue = 8
};

inline constexpr CallType operator|(CallType LHS, CallType RHS) {
  return static_cast<CallType>(static_cast<unsigned>(LHS) |
                               static_cast<unsigned>(RHS));
}

struct ForwardingCallObject {

  template <class... Args> bool operator()(Args &&...) & {
    set_call<Args &&...>(CT_NonConst | CT_LValue);
    return true;
  }

  template <class... Args> bool operator()(Args &&...) const & {
    set_call<Args &&...>(CT_Const | CT_LValue);
    return true;
  }

  // Don't allow the call operator to be invoked as an rvalue.
  template <class... Args> bool operator()(Args &&...) && {
    set_call<Args &&...>(CT_NonConst | CT_RValue);
    return true;
  }

  template <class... Args> bool operator()(Args &&...) const && {
    set_call<Args &&...>(CT_Const | CT_RValue);
    return true;
  }

  template <class... Args> static void set_call(CallType type) {
    assert(last_call_type == CT_None);
    assert(last_call_args == nullptr);
    last_call_type = type;
    last_call_args = std::addressof(makeArgumentID<Args...>());
  }

  template <class... Args> static bool check_call(CallType type) {
    bool result = last_call_type == type && last_call_args &&
                  *last_call_args == makeArgumentID<Args...>();
    last_call_type = CT_None;
    last_call_args = nullptr;
    return result;
  }

  static CallType last_call_type;
  static const TypeID *last_call_args;
};

CallType ForwardingCallObject::last_call_type = CT_None;
const TypeID *ForwardingCallObject::last_call_args = nullptr;

void test_call_operator_forwarding() {
  using Fn = ForwardingCallObject;
  Fn obj{};
  const Fn &cobj = obj;
  { // test call operator forwarding - no variant
    std::visit(obj);
    assert(Fn::check_call<>(CT_NonConst | CT_LValue));
    std::visit(cobj);
    assert(Fn::check_call<>(CT_Const | CT_LValue));
    std::visit(std::move(obj));
    assert(Fn::check_call<>(CT_NonConst | CT_RValue));
    std::visit(std::move(cobj));
    assert(Fn::check_call<>(CT_Const | CT_RValue));
  }
  { // test call operator forwarding - single variant, single arg
    using V = std::variant<int>;
    V v(42);
    std::visit(obj, v);
    assert(Fn::check_call<int &>(CT_NonConst | CT_LValue));
    std::visit(cobj, v);
    assert(Fn::check_call<int &>(CT_Const | CT_LValue));
    std::visit(std::move(obj), v);
    assert(Fn::check_call<int &>(CT_NonConst | CT_RValue));
    std::visit(std::move(cobj), v);
    assert(Fn::check_call<int &>(CT_Const | CT_RValue));
  }
  { // test call operator forwarding - single variant, multi arg
    using V = std::variant<int, long, double>;
    V v(42l);
    std::visit(obj, v);
    assert(Fn::check_call<long &>(CT_NonConst | CT_LValue));
    std::visit(cobj, v);
    assert(Fn::check_call<long &>(CT_Const | CT_LValue));
    std::visit(std::move(obj), v);
    assert(Fn::check_call<long &>(CT_NonConst | CT_RValue));
    std::visit(std::move(cobj), v);
    assert(Fn::check_call<long &>(CT_Const | CT_RValue));
  }
  { // test call operator forwarding - multi variant, multi arg
    using V = std::variant<int, long, double>;
    using V2 = std::variant<int *, std::string>;
    V v(42l);
    V2 v2("hello");
    std::visit(obj, v, v2);
    assert((Fn::check_call<long &, std::string &>(CT_NonConst | CT_LValue)));
    std::visit(cobj, v, v2);
    assert((Fn::check_call<long &, std::string &>(CT_Const | CT_LValue)));
    std::visit(std::move(obj), v, v2);
    assert((Fn::check_call<long &, std::string &>(CT_NonConst | CT_RValue)));
    std::visit(std::move(cobj), v, v2);
    assert((Fn::check_call<long &, std::string &>(CT_Const | CT_RValue)));
  }
}

void test_argument_forwarding() {
  using Fn = ForwardingCallObject;
  Fn obj{};
  const auto Val = CT_LValue | CT_NonConst;
  { // single argument - value type
    using V = std::variant<int>;
    V v(42);
    const V &cv = v;
    std::visit(obj, v);
    assert(Fn::check_call<int &>(Val));
    std::visit(obj, cv);
    assert(Fn::check_call<const int &>(Val));
    std::visit(obj, std::move(v));
    assert(Fn::check_call<int &&>(Val));
    std::visit(obj, std::move(cv));
    assert(Fn::check_call<const int &&>(Val));
  }
#if !defined(TEST_VARIANT_HAS_NO_REFERENCES)
  { // single argument - lvalue reference
    using V = std::variant<int &>;
    int x = 42;
    V v(x);
    const V &cv = v;
    std::visit(obj, v);
    assert(Fn::check_call<int &>(Val));
    std::visit(obj, cv);
    assert(Fn::check_call<int &>(Val));
    std::visit(obj, std::move(v));
    assert(Fn::check_call<int &>(Val));
    std::visit(obj, std::move(cv));
    assert(Fn::check_call<int &>(Val));
  }
  { // single argument - rvalue reference
    using V = std::variant<int &&>;
    int x = 42;
    V v(std::move(x));
    const V &cv = v;
    std::visit(obj, v);
    assert(Fn::check_call<int &>(Val));
    std::visit(obj, cv);
    assert(Fn::check_call<int &>(Val));
    std::visit(obj, std::move(v));
    assert(Fn::check_call<int &&>(Val));
    std::visit(obj, std::move(cv));
    assert(Fn::check_call<int &&>(Val));
  }
  { // multi argument - multi variant
    using S = const std::string &;
    using V = std::variant<int, S, long &&>;
    const std::string str = "hello";
    long l = 43;
    V v1(42);
    const V &cv1 = v1;
    V v2(str);
    const V &cv2 = v2;
    V v3(std::move(l));
    const V &cv3 = v3;
    std::visit(obj, v1, v2, v3);
    assert((Fn::check_call<int &, S, long &>(Val)));
    std::visit(obj, cv1, cv2, std::move(v3));
    assert((Fn::check_call<const int &, S, long &&>(Val)));
  }
#endif
}

struct ReturnFirst {
  template <class... Args> constexpr int operator()(int f, Args &&...) const {
    return f;
  }
};

struct ReturnArity {
  template <class... Args> constexpr int operator()(Args &&...) const {
    return sizeof...(Args);
  }
};

void test_constexpr() {
  constexpr ReturnFirst obj{};
  constexpr ReturnArity aobj{};
  {
    using V = std::variant<int>;
    constexpr V v(42);
    static_assert(std::visit(obj, v) == 42, "");
  }
  {
    using V = std::variant<short, long, char>;
    constexpr V v(42l);
    static_assert(std::visit(obj, v) == 42, "");
  }
  {
    using V1 = std::variant<int>;
    using V2 = std::variant<int, char *, long long>;
    using V3 = std::variant<bool, int, int>;
    constexpr V1 v1;
    constexpr V2 v2(nullptr);
    constexpr V3 v3;
    static_assert(std::visit(aobj, v1, v2, v3) == 3, "");
  }
  {
    using V1 = std::variant<int>;
    using V2 = std::variant<int, char *, long long>;
    using V3 = std::variant<void *, int, int>;
    constexpr V1 v1;
    constexpr V2 v2(nullptr);
    constexpr V3 v3;
    static_assert(std::visit(aobj, v1, v2, v3) == 3, "");
  }
}

void test_exceptions() {
#ifndef TEST_HAS_NO_EXCEPTIONS
  ReturnArity obj{};
  auto test = [&](auto &&... args) {
    try {
      std::visit(obj, args...);
    } catch (const std::bad_variant_access &) {
      return true;
    } catch (...) {
    }
    return false;
  };
  {
    using V = std::variant<int, MakeEmptyT>;
    V v;
    makeEmpty(v);
    assert(test(v));
  }
  {
    using V = std::variant<int, MakeEmptyT>;
    using V2 = std::variant<long, std::string, void *>;
    V v;
    makeEmpty(v);
    V2 v2("hello");
    assert(test(v, v2));
  }
  {
    using V = std::variant<int, MakeEmptyT>;
    using V2 = std::variant<long, std::string, void *>;
    V v;
    makeEmpty(v);
    V2 v2("hello");
    assert(test(v2, v));
  }
  {
    using V = std::variant<int, MakeEmptyT>;
    using V2 = std::variant<long, std::string, void *, MakeEmptyT>;
    V v;
    makeEmpty(v);
    V2 v2;
    makeEmpty(v2);
    assert(test(v, v2));
  }
#endif
}

// See https://bugs.llvm.org/show_bug.cgi?id=31916
void test_caller_accepts_nonconst() {
  struct A {};
  struct Visitor {
    void operator()(A&) {}
  };
  std::variant<A> v;
  std::visit(Visitor{}, v);
}

int main() {
  test_call_operator_forwarding();
  test_argument_forwarding();
  test_constexpr();
  test_exceptions();
  test_caller_accepts_nonconst();
}
