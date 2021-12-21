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

//    The only way to create an unique_ptr<T[]> is to default construct them.

class foo {
public:
    foo () : val_(3) {}
    int get () const { return val_; }
private:
    int val_;
    };

int main()
{
    {
    auto p1 = std::make_unique<int[]>(5);
    for ( int i = 0; i < 5; ++i )
        assert ( p1[i] == 0 );
    }

    {
    auto p2 = std::make_unique<std::string[]>(5);
    for ( int i = 0; i < 5; ++i )
        assert ( p2[i].size () == 0 );
    }

    {
    auto p3 = std::make_unique<foo[]>(7);
    for ( int i = 0; i < 7; ++i )
        assert ( p3[i].get () == 3 );
    }
}
