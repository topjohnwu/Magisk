//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <codecvt>

// enum codecvt_mode
// {
//     consume_header = 4,
//     generate_header = 2,
//     little_endian = 1
// };

#include <codecvt>
#include <cassert>

int main()
{
    assert(std::consume_header == 4);
    assert(std::generate_header == 2);
    assert(std::little_endian == 1);
    std::codecvt_mode e = std::consume_header;
    assert(e == 4);
}
