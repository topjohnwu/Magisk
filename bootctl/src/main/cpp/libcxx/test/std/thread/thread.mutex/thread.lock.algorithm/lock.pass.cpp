//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// UNSUPPORTED: libcpp-has-no-threads

// This test hangs forever when built against libstdc++ and MSVC. In order to allow
// validation of the test suite against other STLs we have to mark it
// unsupported.
// UNSUPPORTED: libstdc++, msvc

// <mutex>

// template <class L1, class L2, class... L3>
//   void lock(L1&, L2&, L3&...);

#include <mutex>
#include <cassert>

#include "test_macros.h"

class L0
{
    bool locked_;

public:
    L0() : locked_(false) {}

    void lock()
    {
        locked_ = true;
    }

    bool try_lock()
    {
        locked_ = true;
        return locked_;
    }

    void unlock() {locked_ = false;}

    bool locked() const {return locked_;}
};

class L1
{
    bool locked_;

public:
    L1() : locked_(false) {}

    void lock()
    {
        locked_ = true;
    }

    bool try_lock()
    {
        locked_ = false;
        return locked_;
    }

    void unlock() {locked_ = false;}

    bool locked() const {return locked_;}
};

class L2
{
    bool locked_;

public:
    L2() : locked_(false) {}

    void lock()
    {
        TEST_THROW(1);
    }

    bool try_lock()
    {
        TEST_THROW(1);
        return locked_;
    }

    void unlock() {locked_ = false;}

    bool locked() const {return locked_;}
};

int main()
{
    {
        L0 l0;
        L0 l1;
        std::lock(l0, l1);
        assert(l0.locked());
        assert(l1.locked());
    }
    {
        L0 l0;
        L1 l1;
        std::lock(l0, l1);
        assert(l0.locked());
        assert(l1.locked());
    }
    {
        L1 l0;
        L0 l1;
        std::lock(l0, l1);
        assert(l0.locked());
        assert(l1.locked());
    }
#ifndef TEST_HAS_NO_EXCEPTIONS
    {
        L0 l0;
        L2 l1;
        try
        {
            std::lock(l0, l1);
            assert(false);
        }
        catch (int)
        {
            assert(!l0.locked());
            assert(!l1.locked());
        }
    }
    {
        L2 l0;
        L0 l1;
        try
        {
            std::lock(l0, l1);
            assert(false);
        }
        catch (int)
        {
            assert(!l0.locked());
            assert(!l1.locked());
        }
    }
    {
        L1 l0;
        L2 l1;
        try
        {
            std::lock(l0, l1);
            assert(false);
        }
        catch (int)
        {
            assert(!l0.locked());
            assert(!l1.locked());
        }
    }
    {
        L2 l0;
        L1 l1;
        try
        {
            std::lock(l0, l1);
            assert(false);
        }
        catch (int)
        {
            assert(!l0.locked());
            assert(!l1.locked());
        }
    }
    {
        L2 l0;
        L2 l1;
        try
        {
            std::lock(l0, l1);
            assert(false);
        }
        catch (int)
        {
            assert(!l0.locked());
            assert(!l1.locked());
        }
    }
#endif
#if TEST_STD_VER >= 11
    {
        L0 l0;
        L0 l1;
        L0 l2;
        std::lock(l0, l1, l2);
        assert(l0.locked());
        assert(l1.locked());
        assert(l2.locked());
    }
#ifndef TEST_HAS_NO_EXCEPTIONS
    {
        L2 l0;
        L2 l1;
        L2 l2;
        try
        {
            std::lock(l0, l1, l2);
            assert(false);
        }
        catch (int)
        {
            assert(!l0.locked());
            assert(!l1.locked());
            assert(!l2.locked());
        }
    }
#endif
    {
        L0 l0;
        L0 l1;
        L1 l2;
        std::lock(l0, l1, l2);
        assert(l0.locked());
        assert(l1.locked());
        assert(l2.locked());
    }
    {
        L0 l0;
        L1 l1;
        L0 l2;
        std::lock(l0, l1, l2);
        assert(l0.locked());
        assert(l1.locked());
        assert(l2.locked());
    }
    {
        L1 l0;
        L0 l1;
        L0 l2;
        std::lock(l0, l1, l2);
        assert(l0.locked());
        assert(l1.locked());
        assert(l2.locked());
    }
#ifndef TEST_HAS_NO_EXCEPTIONS
    {
        L0 l0;
        L0 l1;
        L2 l2;
        try
        {
            std::lock(l0, l1, l2);
            assert(false);
        }
        catch (int)
        {
            assert(!l0.locked());
            assert(!l1.locked());
            assert(!l2.locked());
        }
    }
    {
        L0 l0;
        L2 l1;
        L0 l2;
        try
        {
            std::lock(l0, l1, l2);
            assert(false);
        }
        catch (int)
        {
            assert(!l0.locked());
            assert(!l1.locked());
            assert(!l2.locked());
        }
    }
    {
        L2 l0;
        L0 l1;
        L0 l2;
        try
        {
            std::lock(l0, l1, l2);
            assert(false);
        }
        catch (int)
        {
            assert(!l0.locked());
            assert(!l1.locked());
            assert(!l2.locked());
        }
    }
    {
        L2 l0;
        L2 l1;
        L0 l2;
        try
        {
            std::lock(l0, l1, l2);
            assert(false);
        }
        catch (int)
        {
            assert(!l0.locked());
            assert(!l1.locked());
            assert(!l2.locked());
        }
    }
    {
        L2 l0;
        L0 l1;
        L2 l2;
        try
        {
            std::lock(l0, l1, l2);
            assert(false);
        }
        catch (int)
        {
            assert(!l0.locked());
            assert(!l1.locked());
            assert(!l2.locked());
        }
    }
    {
        L0 l0;
        L2 l1;
        L2 l2;
        try
        {
            std::lock(l0, l1, l2);
            assert(false);
        }
        catch (int)
        {
            assert(!l0.locked());
            assert(!l1.locked());
            assert(!l2.locked());
        }
    }
    {
        L2 l0;
        L2 l1;
        L1 l2;
        try
        {
            std::lock(l0, l1, l2);
            assert(false);
        }
        catch (int)
        {
            assert(!l0.locked());
            assert(!l1.locked());
            assert(!l2.locked());
        }
    }
    {
        L2 l0;
        L1 l1;
        L2 l2;
        try
        {
            std::lock(l0, l1, l2);
            assert(false);
        }
        catch (int)
        {
            assert(!l0.locked());
            assert(!l1.locked());
            assert(!l2.locked());
        }
    }
    {
        L1 l0;
        L2 l1;
        L2 l2;
        try
        {
            std::lock(l0, l1, l2);
            assert(false);
        }
        catch (int)
        {
            assert(!l0.locked());
            assert(!l1.locked());
            assert(!l2.locked());
        }
    }
#endif  // TEST_HAS_NO_EXCEPTIONS
    {
        L0 l0;
        L0 l1;
        L0 l2;
        L0 l3;
        std::lock(l0, l1, l2, l3);
        assert(l0.locked());
        assert(l1.locked());
        assert(l2.locked());
        assert(l3.locked());
    }
    {
        L0 l0;
        L0 l1;
        L0 l2;
        L1 l3;
        std::lock(l0, l1, l2, l3);
        assert(l0.locked());
        assert(l1.locked());
        assert(l2.locked());
        assert(l3.locked());
    }
    {
        L0 l0;
        L0 l1;
        L1 l2;
        L0 l3;
        std::lock(l0, l1, l2, l3);
        assert(l0.locked());
        assert(l1.locked());
        assert(l2.locked());
        assert(l3.locked());
    }
    {
        L0 l0;
        L1 l1;
        L0 l2;
        L0 l3;
        std::lock(l0, l1, l2, l3);
        assert(l0.locked());
        assert(l1.locked());
        assert(l2.locked());
        assert(l3.locked());
    }
    {
        L1 l0;
        L0 l1;
        L0 l2;
        L0 l3;
        std::lock(l0, l1, l2, l3);
        assert(l0.locked());
        assert(l1.locked());
        assert(l2.locked());
        assert(l3.locked());
    }
#ifndef TEST_HAS_NO_EXCEPTIONS
    {
        L0 l0;
        L0 l1;
        L0 l2;
        L2 l3;
        try
        {
            std::lock(l0, l1, l2, l3);
            assert(false);
        }
        catch (int)
        {
            assert(!l0.locked());
            assert(!l1.locked());
            assert(!l2.locked());
            assert(!l3.locked());
        }
    }
    {
        L0 l0;
        L0 l1;
        L2 l2;
        L0 l3;
        try
        {
            std::lock(l0, l1, l2, l3);
            assert(false);
        }
        catch (int)
        {
            assert(!l0.locked());
            assert(!l1.locked());
            assert(!l2.locked());
            assert(!l3.locked());
        }
    }
    {
        L0 l0;
        L2 l1;
        L0 l2;
        L0 l3;
        try
        {
            std::lock(l0, l1, l2, l3);
            assert(false);
        }
        catch (int)
        {
            assert(!l0.locked());
            assert(!l1.locked());
            assert(!l2.locked());
            assert(!l3.locked());
        }
    }
    {
        L2 l0;
        L0 l1;
        L0 l2;
        L0 l3;
        try
        {
            std::lock(l0, l1, l2, l3);
            assert(false);
        }
        catch (int)
        {
            assert(!l0.locked());
            assert(!l1.locked());
            assert(!l2.locked());
            assert(!l3.locked());
        }
    }
#endif  // TEST_HAS_NO_EXCEPTIONS
#endif // TEST_STD_VER >= 11
}
