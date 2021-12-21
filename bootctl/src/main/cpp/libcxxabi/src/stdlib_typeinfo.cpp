//===----------------------------- typeinfo.cpp ---------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include <typeinfo>

namespace std
{

// type_info

type_info::~type_info()
{
}

// bad_cast

bad_cast::bad_cast() _NOEXCEPT
{
}

bad_cast::~bad_cast() _NOEXCEPT
{
}

const char*
bad_cast::what() const _NOEXCEPT
{
  return "std::bad_cast";
}

// bad_typeid

bad_typeid::bad_typeid() _NOEXCEPT
{
}

bad_typeid::~bad_typeid() _NOEXCEPT
{
}

const char*
bad_typeid::what() const _NOEXCEPT
{
  return "std::bad_typeid";
}

}  // std
