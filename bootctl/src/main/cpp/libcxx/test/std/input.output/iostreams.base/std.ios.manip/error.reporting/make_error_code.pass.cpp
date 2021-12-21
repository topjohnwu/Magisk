//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <ios>

// error_code make_error_code(io_errc e);

#include <ios>
#include <cassert>

int main()
{
    {
        std::error_code ec = make_error_code(std::io_errc::stream);
        assert(ec.value() == static_cast<int>(std::io_errc::stream));
        assert(ec.category() == std::iostream_category());
    }
}
