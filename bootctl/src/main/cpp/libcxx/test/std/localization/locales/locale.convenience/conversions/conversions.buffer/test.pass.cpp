//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <locale>

// wbuffer_convert<Codecvt, Elem, Tr>

#include <fstream>
#include <locale>
#include <codecvt>
#include <cassert>

int main()
{
    {
        std::ofstream bytestream("myfile.txt");
        std::wbuffer_convert<std::codecvt_utf8<wchar_t> > mybuf(bytestream.rdbuf());
        std::wostream mystr(&mybuf);
        mystr << L"Hello" << std::endl;
    }
    {
        std::ifstream bytestream("myfile.txt");
        std::wbuffer_convert<std::codecvt_utf8<wchar_t> > mybuf(bytestream.rdbuf());
        std::wistream mystr(&mybuf);
        std::wstring ws;
        mystr >> ws;
        assert(ws == L"Hello");
    }
    std::remove("myfile.txt");
}
