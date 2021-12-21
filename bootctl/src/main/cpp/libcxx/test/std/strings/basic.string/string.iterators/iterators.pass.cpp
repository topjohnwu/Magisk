//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11

// <string>

// iterator       begin();
// iterator       end();
// const_iterator begin()  const;
// const_iterator end()    const;
// const_iterator cbegin() const;
// const_iterator cend()   const;

#include <string>
#include <cassert>

int main()
{
    { // N3644 testing
        typedef std::string C;
        C::iterator ii1{}, ii2{};
        C::iterator ii4 = ii1;
        C::const_iterator cii{};
        assert ( ii1 == ii2 );
        assert ( ii1 == ii4 );
        assert ( ii1 == cii );
        assert ( !(ii1 != ii2 ));
        assert ( !(ii1 != cii ));
    }

    { // N3644 testing
        typedef std::wstring C;
        C::iterator ii1{}, ii2{};
        C::iterator ii4 = ii1;
        C::const_iterator cii{};
        assert ( ii1 == ii2 );
        assert ( ii1 == ii4 );
        assert ( ii1 == cii );
        assert ( !(ii1 != ii2 ));
        assert ( !(ii1 != cii ));
    }

#if defined(__cpp_lib_char8_t) && __cpp_lib_char8_t >= 201811L
    {
        typedef std::u8string C;
        C::iterator ii1{}, ii2{};
        C::iterator ii4 = ii1;
        C::const_iterator cii{};
        assert ( ii1 == ii2 );
        assert ( ii1 == ii4 );
        assert ( ii1 == cii );
        assert ( !(ii1 != ii2 ));
        assert ( !(ii1 != cii ));
    }
#endif

    { // N3644 testing
        typedef std::u16string C;
        C::iterator ii1{}, ii2{};
        C::iterator ii4 = ii1;
        C::const_iterator cii{};
        assert ( ii1 == ii2 );
        assert ( ii1 == ii4 );
        assert ( ii1 == cii );
        assert ( !(ii1 != ii2 ));
        assert ( !(ii1 != cii ));
    }

    { // N3644 testing
        typedef std::u32string C;
        C::iterator ii1{}, ii2{};
        C::iterator ii4 = ii1;
        C::const_iterator cii{};
        assert ( ii1 == ii2 );
        assert ( ii1 == ii4 );
        assert ( ii1 == cii );
        assert ( !(ii1 != ii2 ));
        assert ( !(ii1 != cii ));
    }
}
