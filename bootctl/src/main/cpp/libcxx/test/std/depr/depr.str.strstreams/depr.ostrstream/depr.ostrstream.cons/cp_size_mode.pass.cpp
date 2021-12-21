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

// ostrstream(char* s, int n, ios_base::openmode mode = ios_base::out);

#include <strstream>
#include <cassert>
#include <string>

int main()
{
    {
        char buf[] = "123 4.5 dog";
        std::ostrstream out(buf, 0);
        assert(out.str() == std::string("123 4.5 dog"));
        int i = 321;
        double d = 5.5;
        std::string s("cat");
        out << i << ' ' << d << ' ' << s << std::ends;
        assert(out.str() == std::string("321 5.5 cat"));
    }
    {
        char buf[23] = "123 4.5 dog";
        std::ostrstream out(buf, 11, std::ios::app);
        assert(out.str() == std::string("123 4.5 dog"));
        int i = 321;
        double d = 5.5;
        std::string s("cat");
        out << i << ' ' << d << ' ' << s << std::ends;
        assert(out.str() == std::string("123 4.5 dog321 5.5 cat"));
    }
}
