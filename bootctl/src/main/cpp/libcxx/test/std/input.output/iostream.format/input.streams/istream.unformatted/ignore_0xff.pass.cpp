//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <istream>

// basic_istream<charT,traits>&
//    ignore(streamsize n = 1, int_type delim = traits::eof());

// https://bugs.llvm.org/show_bug.cgi?id=16427

#include <sstream>
#include <cassert>

int main()
{
    int bad=-1;
    std::ostringstream os;
    os << "aaaabbbb" << static_cast<char>(bad)
       << "ccccdddd" << std::endl;
    std::string s=os.str();

    std::istringstream is(s);
    const unsigned int ignoreLen=10;
    std::istringstream::pos_type a=is.tellg();
    is.ignore(ignoreLen);
    std::istringstream::pos_type b=is.tellg();
    assert((b-a)==ignoreLen);
}
