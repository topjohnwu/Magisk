//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11, c++14

// <any>

// Check that the size and alignment of any are what we expect.

#include <any>
#include "any_helpers.h"

constexpr std::size_t BufferSize = (sizeof(void*) * 3);
constexpr std::size_t BufferAlignment = alignof(void*);
// Clang doesn't like "alignof(BufferAlignment * 2)" due to PR13986.
// So we create "DoubleBufferAlignment" instead.
constexpr std::size_t DoubleBufferAlignment = BufferAlignment * 2;

class SmallThrowsDtor
{
public:
    SmallThrowsDtor() {}
    SmallThrowsDtor(SmallThrowsDtor const &) noexcept {}
    SmallThrowsDtor(SmallThrowsDtor &&) noexcept {}
    ~SmallThrowsDtor() noexcept(false) {}
};


struct alignas(1) MaxSizeType {
    char buff[BufferSize];
};

struct alignas(BufferAlignment) MaxAlignType {
};

struct alignas(BufferAlignment) MaxSizeAndAlignType {
    char buff[BufferSize];
};


struct alignas(1) OverSizeType {
    char buff[BufferSize + 1];
};

struct alignas(DoubleBufferAlignment) OverAlignedType {
};

struct alignas(DoubleBufferAlignment) OverSizeAndAlignedType {
    char buff[BufferSize + 1];
};

int main()
{
    using std::any;
    using std::__any_imp::_IsSmallObject;
    static_assert(_IsSmallObject<small>::value, "");
    static_assert(_IsSmallObject<void*>::value, "");
    static_assert(!_IsSmallObject<SmallThrowsDtor>::value, "");
    static_assert(!_IsSmallObject<large>::value, "");
    {
        // Check a type that meets the size requirement *exactly* and has
        // a lesser alignment requirement is considered small.
        typedef MaxSizeType T;
        static_assert(sizeof(T) == BufferSize, "");
        static_assert(alignof(T) < BufferAlignment,   "");
        static_assert(_IsSmallObject<T>::value, "");
    }
    {
        // Check a type that meets the alignment requirement *exactly* and has
        // a lesser size is considered small.
        typedef MaxAlignType T;
        static_assert(sizeof(T) < BufferSize, "");
        static_assert(alignof(T) == BufferAlignment,   "");
        static_assert(_IsSmallObject<T>::value, "");
    }
    {
        // Check a type that meets the size and alignment requirements *exactly*
        // is considered small.
        typedef MaxSizeAndAlignType T;
        static_assert(sizeof(T) == BufferSize, "");
        static_assert(alignof(T) == BufferAlignment,   "");
        static_assert(_IsSmallObject<T>::value, "");
    }
    {
        // Check a type that meets the alignment requirements but is over-sized
        // is not considered small.
        typedef OverSizeType T;
        static_assert(sizeof(T) > BufferSize, "");
        static_assert(alignof(T) < BufferAlignment, "");
        static_assert(!_IsSmallObject<T>::value, "");
    }
    {
        // Check a type that meets the size requirements but is over-aligned
        // is not considered small.
        typedef OverAlignedType T;
        static_assert(sizeof(T) < BufferSize, "");
        static_assert(alignof(T) > BufferAlignment, "");
        static_assert(!_IsSmallObject<T>::value, "");
    }
    {
        // Check a type that exceeds both the size an alignment requirements
        // is not considered small.
        typedef OverSizeAndAlignedType T;
        static_assert(sizeof(T) > BufferSize, "");
        static_assert(alignof(T) > BufferAlignment, "");
        static_assert(!_IsSmallObject<T>::value, "");
    }
}
