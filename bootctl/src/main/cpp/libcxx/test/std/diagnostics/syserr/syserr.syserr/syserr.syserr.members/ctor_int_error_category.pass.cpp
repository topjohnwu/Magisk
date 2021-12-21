//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <system_error>

// class system_error

// system_error(int ev, const error_category& ecat);

// Test is slightly non-portable

#include <system_error>
#include <string>
#include <cassert>

int main()
{
    std::system_error se(static_cast<int>(std::errc::not_a_directory),
                         std::generic_category());
    assert(se.code() == std::make_error_code(std::errc::not_a_directory));
    std::string what_message(se.what());
    assert(what_message.find("Not a directory") != std::string::npos);
}
