//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <strstream>

// class istrstream
//     : public basic_istream<char>
// {
//     ...

#include <strstream>
#include <type_traits>

int main()
{
    static_assert((std::is_base_of<std::istream, std::istrstream>::value), "");
}
