//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <utility>

// template <class T1, class T2> struct pair

// pair& operator=(pair&& p);

#include <utility>
#include <memory>
#include <cassert>


struct NonAssignable {
  NonAssignable& operator=(NonAssignable const&) = delete;
  NonAssignable& operator=(NonAssignable&&) = delete;
};
struct CopyAssignable {
  CopyAssignable() = default;
  CopyAssignable& operator=(CopyAssignable const&) = default;
  CopyAssignable& operator=(CopyAssignable&&) = delete;
};
struct MoveAssignable {
  MoveAssignable() = default;
  MoveAssignable& operator=(MoveAssignable const&) = delete;
  MoveAssignable& operator=(MoveAssignable&&) = default;
};

struct CountAssign {
  static int copied;
  static int moved;
  static void reset() { copied = moved = 0; }
  CountAssign() = default;
  CountAssign& operator=(CountAssign const&) { ++copied; return *this; }
  CountAssign& operator=(CountAssign&&) { ++moved; return *this; }
};
int CountAssign::copied = 0;
int CountAssign::moved = 0;

int main()
{
    {
        typedef std::pair<std::unique_ptr<int>, int> P;
        P p1(std::unique_ptr<int>(new int(3)), 4);
        P p2;
        p2 = std::move(p1);
        assert(*p2.first == 3);
        assert(p2.second == 4);
    }
    {
        using P = std::pair<int&, int&&>;
        int x = 42;
        int y = 101;
        int x2 = -1;
        int y2 = 300;
        P p1(x, std::move(y));
        P p2(x2, std::move(y2));
        p1 = std::move(p2);
        assert(p1.first == x2);
        assert(p1.second == y2);
    }
    {
        using P = std::pair<int, NonAssignable>;
        static_assert(!std::is_move_assignable<P>::value, "");
    }
    {
        // The move decays to the copy constructor
        CountAssign::reset();
        using P = std::pair<CountAssign, CopyAssignable>;
        static_assert(std::is_move_assignable<P>::value, "");
        P p;
        P p2;
        p = std::move(p2);
        assert(CountAssign::moved == 0);
        assert(CountAssign::copied == 1);
    }
    {
        CountAssign::reset();
        using P = std::pair<CountAssign, MoveAssignable>;
        static_assert(std::is_move_assignable<P>::value, "");
        P p;
        P p2;
        p = std::move(p2);
        assert(CountAssign::moved == 1);
        assert(CountAssign::copied == 0);
    }
}
