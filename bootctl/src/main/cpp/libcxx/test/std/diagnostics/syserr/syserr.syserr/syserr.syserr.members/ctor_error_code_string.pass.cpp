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

// system_error(error_code ec, const string& what_arg);

// Test is slightly non-portable

#include <system_error>
#include <string>
#include <cassert>

int main()
{
    std::string what_arg("test message");
    std::system_error se(make_error_code(std::errc::not_a_directory), what_arg);
    assert(se.code() == std::make_error_code(std::errc::not_a_directory));
    std::string what_message(se.what());
    assert(what_message.find(what_arg) != std::string::npos);
    assert(what_message.find("Not a directory") != std::string::npos);
}
