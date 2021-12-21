//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef SUPPORT_TYPE_ID_H
#define SUPPORT_TYPE_ID_H

#include <functional>
#include <typeinfo>
#include <string>
#include <cstdio>
#include <cassert>

#include "test_macros.h"
#include "demangle.h"

#if TEST_STD_VER < 11
#error This header requires C++11 or greater
#endif

// TypeID - Represent a unique identifier for a type. TypeID allows equality
// comparisons between different types.
struct TypeID {
  friend bool operator==(TypeID const& LHS, TypeID const& RHS)
  {return LHS.m_id == RHS.m_id; }
  friend bool operator!=(TypeID const& LHS, TypeID const& RHS)
  {return LHS.m_id != RHS.m_id; }

  std::string name() const {
    return demangle(m_id);
  }

  void dump() const {
    std::string s = name();
    std::printf("TypeID: %s\n", s.c_str());
  }

private:
  explicit constexpr TypeID(const char* xid) : m_id(xid) {}

  TypeID(const TypeID&) = delete;
  TypeID& operator=(TypeID const&) = delete;

  const char* const m_id;
  template <class T> friend TypeID const& makeTypeIDImp();
};

// makeTypeID - Return the TypeID for the specified type 'T'.
template <class T>
inline TypeID const& makeTypeIDImp() {
  static const TypeID id(typeid(T).name());
  return id;
}

template <class T>
struct TypeWrapper {};

template <class T>
inline  TypeID const& makeTypeID() {
  return makeTypeIDImp<TypeWrapper<T>>();
}

template <class ...Args>
struct ArgumentListID {};

// makeArgumentID - Create and return a unique identifier for a given set
// of arguments.
template <class ...Args>
inline  TypeID const& makeArgumentID() {
  return makeTypeIDImp<ArgumentListID<Args...>>();
}


// COMPARE_TYPEID(...) is a utility macro for generating diagnostics when
// two typeid's are expected to be equal
#define COMPARE_TYPEID(LHS, RHS) CompareTypeIDVerbose(#LHS, LHS, #RHS, RHS)

inline bool CompareTypeIDVerbose(const char* LHSString, TypeID const* LHS,
                                 const char* RHSString, TypeID const* RHS) {
  if (*LHS == *RHS)
    return true;
  std::printf("TypeID's not equal:\n");
  std::printf("%s: %s\n----------\n%s: %s\n",
              LHSString, LHS->name().c_str(),
              RHSString, RHS->name().c_str());
  return false;
}

#endif // SUPPORT_TYPE_ID_H
