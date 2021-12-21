//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11
#include <memory>
#include <string>
#include <cassert>

int main()
{
    {
    std::unique_ptr<int> p1 = std::make_unique<int>(1);
    assert ( *p1 == 1 );
    p1 = std::make_unique<int> ();
    assert ( *p1 == 0 );
    }

    {
    std::unique_ptr<std::string> p2 = std::make_unique<std::string> ( "Meow!" );
    assert ( *p2 == "Meow!" );
    p2 = std::make_unique<std::string> ();
    assert ( *p2 == "" );
    p2 = std::make_unique<std::string> ( 6, 'z' );
    assert ( *p2 == "zzzzzz" );
    }
}
