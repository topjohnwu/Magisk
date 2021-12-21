//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <locale>

// template <class charT> class collate;

// int compare(const charT* low1, const charT* high1,
//             const charT* low2, const charT* high2) const;

#include <locale>
#include <cassert>

int main()
{
    std::locale l = std::locale::classic();
    {
        const char ia[] = "1234";
        const unsigned sa = sizeof(ia)/sizeof(ia[0]);
        const char ib[] = "123";
        const std::collate<char>& f = std::use_facet<std::collate<char> >(l);
        assert(f.compare(ia, ia+sa, ib, ib+2) == 1);
        assert(f.compare(ib, ib+2, ia, ia+sa) == -1);
        assert(f.compare(ia, ia+sa, ib, ib+3) == 1);
        assert(f.compare(ib, ib+3, ia, ia+sa) == -1);
        assert(f.compare(ia, ia+sa, ib+1, ib+3) == -1);
        assert(f.compare(ib+1, ib+3, ia, ia+sa) == 1);
        assert(f.compare(ia, ia+3, ib, ib+3) == 0);
    }
    {
        const wchar_t ia[] = L"1234";
        const unsigned sa = sizeof(ia)/sizeof(ia[0]);
        const wchar_t ib[] = L"123";
        const std::collate<wchar_t>& f = std::use_facet<std::collate<wchar_t> >(l);
        assert(f.compare(ia, ia+sa, ib, ib+2) == 1);
        assert(f.compare(ib, ib+2, ia, ia+sa) == -1);
        assert(f.compare(ia, ia+sa, ib, ib+3) == 1);
        assert(f.compare(ib, ib+3, ia, ia+sa) == -1);
        assert(f.compare(ia, ia+sa, ib+1, ib+3) == -1);
        assert(f.compare(ib+1, ib+3, ia, ia+sa) == 1);
        assert(f.compare(ia, ia+3, ib, ib+3) == 0);
    }
}
