//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <tuple>

// template <class... Types> class tuple;

// template <class... UTypes>
//   explicit tuple(UTypes&&... u);

// UNSUPPORTED: c++98, c++03

/*
    This is testing an extension whereby only Types having an explicit conversion
    from UTypes are bound by the explicit tuple constructor.
*/

#include <tuple>
#include <cassert>

class MoveOnly
{
    MoveOnly(const MoveOnly&);
    MoveOnly& operator=(const MoveOnly&);

    int data_;
public:
    explicit MoveOnly(int data = 1) : data_(data) {}
    MoveOnly(MoveOnly&& x)
        : data_(x.data_) {x.data_ = 0;}
    MoveOnly& operator=(MoveOnly&& x)
        {data_ = x.data_; x.data_ = 0; return *this;}

    int get() const {return data_;}

    bool operator==(const MoveOnly& x) const {return data_ == x.data_;}
    bool operator< (const MoveOnly& x) const {return data_ <  x.data_;}
};

int main()
{
    {
        std::tuple<MoveOnly> t = 1;
    }
}
