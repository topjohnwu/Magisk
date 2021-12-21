//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <locale>

// template <class charT> class ctype_byname;

// bool is(mask m, charT c) const;

#include <locale>
#include <type_traits>
#include <cassert>

int main()
{
    {
        std::locale l("C");
        {
            typedef std::ctype<wchar_t> WF;
            const WF& wf = std::use_facet<WF>(l);
            typedef std::ctype<char> CF;
            const CF& cf = std::use_facet<CF>(l);

            // The ctype masks in Newlib don't form a proper bitmask because
            // the mask is only 8 bits wide, and there are more than 8 mask
            // kinds. This means that the mask for alpha is (_U | _L), which
            // is tricky to match in the do_is implementation because in
            // [22.4.1.1.2 2] the standard specifies that the match code behaves
            // like (m & M) != 0, but following this exactly would give false
            // positives for characters that are both 'upper' and 'alpha', but
            // not 'lower', for example.
            assert( wf.is(WF::upper, L'A'));
            assert( cf.is(CF::upper,  'A'));
            assert(!wf.is(WF::lower, L'A'));
            assert(!cf.is(CF::lower,  'A'));
            assert( wf.is(WF::alpha, L'A'));
            assert( cf.is(CF::alpha,  'A'));

            assert(!wf.is(WF::upper, L'a'));
            assert(!cf.is(CF::upper,  'a'));
            assert( wf.is(WF::lower, L'a'));
            assert( cf.is(CF::lower,  'a'));
            assert( wf.is(WF::alpha, L'a'));
            assert( cf.is(CF::alpha,  'a'));
        }
    }
}
