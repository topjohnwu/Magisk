//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11
// <system_error>

// class error_category

// constexpr error_category() noexcept;

#include <system_error>
#include <type_traits>
#include <string>
#include <cassert>

class test1
    : public std::error_category
{
public:
    constexpr test1() = default;  // won't compile if error_category() is not constexpr
    virtual const char* name() const noexcept {return nullptr;}
    virtual std::string message(int) const {return std::string();}
};

int main()
{
    static_assert(std::is_nothrow_default_constructible<test1>::value,
                                 "error_category() must exist and be noexcept");
}
