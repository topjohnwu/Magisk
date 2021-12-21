//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <sstream>

// template <class charT, class traits = char_traits<charT>, class Allocator = allocator<charT> >
// class basic_stringstream

// explicit basic_stringstream(const basic_string<charT,traits,Allocator>& str,
//                             ios_base::openmode which = ios_base::out|ios_base::in);

#include <sstream>
#include <cassert>

template<typename T>
struct NoDefaultAllocator : std::allocator<T>
{
  template<typename U> struct rebind { using other = NoDefaultAllocator<U>; };
  NoDefaultAllocator(int id_) : id(id_) { }
  template<typename U> NoDefaultAllocator(const NoDefaultAllocator<U>& a) : id(a.id) { }
  int id;
};


int main()
{
    {
        std::stringstream ss(" 123 456 ");
        assert(ss.rdbuf() != 0);
        assert(ss.good());
        assert(ss.str() == " 123 456 ");
        int i = 0;
        ss >> i;
        assert(i == 123);
        ss >> i;
        assert(i == 456);
        ss << i << ' ' << 123;
        assert(ss.str() == "456 1236 ");
    }
    {
        std::wstringstream ss(L" 123 456 ");
        assert(ss.rdbuf() != 0);
        assert(ss.good());
        assert(ss.str() == L" 123 456 ");
        int i = 0;
        ss >> i;
        assert(i == 123);
        ss >> i;
        assert(i == 456);
        ss << i << ' ' << 123;
        assert(ss.str() == L"456 1236 ");
    }
    { // This is https://bugs.llvm.org/show_bug.cgi?id=33727
        typedef std::basic_string   <char, std::char_traits<char>, NoDefaultAllocator<char> > S;
        typedef std::basic_stringbuf<char, std::char_traits<char>, NoDefaultAllocator<char> > SB;

        S s(NoDefaultAllocator<char>(1));
        SB sb(s);
    //  This test is not required by the standard, but *where else* could it get the allocator?
        assert(sb.str().get_allocator() == s.get_allocator());
    }
}
