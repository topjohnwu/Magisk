//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <strstream>

// class strstreambuf

// int_type overflow(int_type c = EOF);

#include <strstream>
#include <cassert>

int main()
{
    {
        char buf[12] = "abc";
        std::strstreambuf sb(buf, sizeof(buf), buf);
        assert(sb.sputc('1') == '1');
        assert(sb.str() == std::string("1bc"));
        assert(sb.sputc('2') == '2');
        assert(sb.str() == std::string("12c"));
        assert(sb.sputc('3') == '3');
        assert(sb.str() == std::string("123"));
        assert(sb.sputc('4') == '4');
        assert(sb.str() == std::string("1234"));
        assert(sb.sputc('5') == '5');
        assert(sb.str() == std::string("12345"));
        assert(sb.sputc('6') == '6');
        assert(sb.str() == std::string("123456"));
        assert(sb.sputc('7') == '7');
        assert(sb.str() == std::string("1234567"));
        assert(sb.sputc('8') == '8');
        assert(sb.str() == std::string("12345678"));
        assert(sb.sputc('9') == '9');
        assert(sb.str() == std::string("123456789"));
        assert(sb.sputc('0') == '0');
        assert(sb.str() == std::string("1234567890"));
        assert(sb.sputc('1') == '1');
        assert(sb.str() == std::string("12345678901"));
    }
}
