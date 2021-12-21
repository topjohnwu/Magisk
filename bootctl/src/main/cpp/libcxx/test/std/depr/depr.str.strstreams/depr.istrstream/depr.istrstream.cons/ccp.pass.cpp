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

// explicit istrstream(const char* s);

#include <strstream>
#include <cassert>
#include <string>

int main()
{
    {
        const char buf[] = "123 4.5 dog";
        std::istrstream in(buf);
        int i;
        in >> i;
        assert(i == 123);
        double d;
        in >> d;
        assert(d == 4.5);
        std::string s;
        in >> s;
        assert(s == "dog");
        assert(in.eof());
        assert(!in.fail());
        in.clear();
        in.putback('g');
        assert(!in.fail());
        in.putback('g');
        assert(in.fail());
        assert(buf[9] == 'o');
        assert(buf[10] == 'g');
    }
}
