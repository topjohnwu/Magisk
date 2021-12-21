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

// strstreambuf* rdbuf() const;

#include <strstream>
#include <cassert>

int main()
{
    {
        char buf[] = "123 4.5 dog";
        const std::ostrstream out(buf, 0);
        std::strstreambuf* sb = out.rdbuf();
        assert(sb->sputc('a') == 'a');
        assert(buf == std::string("a23 4.5 dog"));
    }
}
