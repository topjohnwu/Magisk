//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <strstream>

// class ostrstream
//     : public basic_ostream<char>
// {
//     ...

#include <strstream>
#include <type_traits>

int main()
{
    static_assert((std::is_base_of<std::ostream, std::ostrstream>::value), "");
}
