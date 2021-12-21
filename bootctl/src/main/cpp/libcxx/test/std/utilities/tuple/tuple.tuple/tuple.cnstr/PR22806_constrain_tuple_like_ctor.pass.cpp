//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// UNSUPPORTED: c++98, c++03

// <tuple>

// template <class... Types> class tuple;

// template <class TupleLike>
//   tuple(TupleLike&&);
// template <class Alloc, class TupleLike>
//   tuple(std::allocator_arg_t, Alloc const&, TupleLike&&);

// Check that the tuple-like ctors are properly disabled when the UTypes...
// constructor should be selected. See PR22806.

#include <tuple>
#include <memory>
#include <cassert>

template <class Tp>
using uncvref_t = typename std::remove_cv<typename std::remove_reference<Tp>::type>::type;

template <class Tuple, class = uncvref_t<Tuple>>
struct IsTuple : std::false_type {};

template <class Tuple, class ...Args>
struct IsTuple<Tuple, std::tuple<Args...>> : std::true_type {};

struct ConstructibleFromTupleAndInt {
  enum State { FromTuple, FromInt, Copied, Moved };
  State state;

  ConstructibleFromTupleAndInt(ConstructibleFromTupleAndInt const&) : state(Copied) {}
  ConstructibleFromTupleAndInt(ConstructibleFromTupleAndInt &&) : state(Moved) {}

  template <class Tuple, class = typename std::enable_if<IsTuple<Tuple>::value>::type>
  explicit ConstructibleFromTupleAndInt(Tuple&&) : state(FromTuple) {}

  explicit ConstructibleFromTupleAndInt(int) : state(FromInt) {}
};

struct ConvertibleFromTupleAndInt {
  enum State { FromTuple, FromInt, Copied, Moved };
  State state;

  ConvertibleFromTupleAndInt(ConvertibleFromTupleAndInt const&) : state(Copied) {}
  ConvertibleFromTupleAndInt(ConvertibleFromTupleAndInt &&) : state(Moved) {}

  template <class Tuple, class = typename std::enable_if<IsTuple<Tuple>::value>::type>
  ConvertibleFromTupleAndInt(Tuple&&) : state(FromTuple) {}

  ConvertibleFromTupleAndInt(int) : state(FromInt) {}
};

struct ConstructibleFromInt {
  enum State { FromInt, Copied, Moved };
  State state;

  ConstructibleFromInt(ConstructibleFromInt const&) : state(Copied) {}
  ConstructibleFromInt(ConstructibleFromInt &&) : state(Moved) {}

  explicit ConstructibleFromInt(int) : state(FromInt) {}
};

struct ConvertibleFromInt {
  enum State { FromInt, Copied, Moved };
  State state;

  ConvertibleFromInt(ConvertibleFromInt const&) : state(Copied) {}
  ConvertibleFromInt(ConvertibleFromInt &&) : state(Moved) {}
  ConvertibleFromInt(int) : state(FromInt) {}
};

int main()
{
    // Test for the creation of dangling references when a tuple is used to
    // store a reference to another tuple as its only element.
    // Ex std::tuple<std::tuple<int>&&>.
    // In this case the constructors 1) 'tuple(UTypes&&...)'
    // and 2) 'tuple(TupleLike&&)' need to be manually disambiguated because
    // when both #1 and #2 participate in partial ordering #2 will always
    // be chosen over #1.
    // See PR22806  and LWG issue #2549 for more information.
    // (https://bugs.llvm.org/show_bug.cgi?id=22806)
    using T = std::tuple<int>;
    std::allocator<int> A;
    { // rvalue reference
        T t1(42);
        std::tuple< T&& > t2(std::move(t1));
        assert(&std::get<0>(t2) == &t1);
    }
    { // const lvalue reference
        T t1(42);

        std::tuple< T const & > t2(t1);
        assert(&std::get<0>(t2) == &t1);

        std::tuple< T const & > t3(static_cast<T const&>(t1));
        assert(&std::get<0>(t3) == &t1);
    }
    { // lvalue reference
        T t1(42);

        std::tuple< T & > t2(t1);
        assert(&std::get<0>(t2) == &t1);
    }
    { // const rvalue reference
        T t1(42);

        std::tuple< T const && > t2(std::move(t1));
        assert(&std::get<0>(t2) == &t1);
    }
    { // rvalue reference via uses-allocator
        T t1(42);
        std::tuple< T&& > t2(std::allocator_arg, A, std::move(t1));
        assert(&std::get<0>(t2) == &t1);
    }
    { // const lvalue reference via uses-allocator
        T t1(42);

        std::tuple< T const & > t2(std::allocator_arg, A, t1);
        assert(&std::get<0>(t2) == &t1);

        std::tuple< T const & > t3(std::allocator_arg, A, static_cast<T const&>(t1));
        assert(&std::get<0>(t3) == &t1);
    }
    { // lvalue reference via uses-allocator
        T t1(42);

        std::tuple< T & > t2(std::allocator_arg, A, t1);
        assert(&std::get<0>(t2) == &t1);
    }
    { // const rvalue reference via uses-allocator
        T const t1(42);
        std::tuple< T const && > t2(std::allocator_arg, A, std::move(t1));
        assert(&std::get<0>(t2) == &t1);
    }
    // Test constructing a 1-tuple of the form tuple<UDT> from another 1-tuple
    // 'tuple<T>' where UDT *can* be constructed from 'tuple<T>'. In this case
    // the 'tuple(UTypes...)' ctor should be chosen and 'UDT' constructed from
    // 'tuple<T>'.
    {
        using VT = ConstructibleFromTupleAndInt;
        std::tuple<int> t1(42);
        std::tuple<VT> t2(t1);
        assert(std::get<0>(t2).state == VT::FromTuple);
    }
    {
        using VT = ConvertibleFromTupleAndInt;
        std::tuple<int> t1(42);
        std::tuple<VT> t2 = {t1};
        assert(std::get<0>(t2).state == VT::FromTuple);
    }
    // Test constructing a 1-tuple of the form tuple<UDT> from another 1-tuple
    // 'tuple<T>' where UDT cannot be constructed from 'tuple<T>' but can
    // be constructed from 'T'. In this case the tuple-like ctor should be
    // chosen and 'UDT' constructed from 'T'
    {
        using VT = ConstructibleFromInt;
        std::tuple<int> t1(42);
        std::tuple<VT> t2(t1);
        assert(std::get<0>(t2).state == VT::FromInt);
    }
    {
        using VT = ConvertibleFromInt;
        std::tuple<int> t1(42);
        std::tuple<VT> t2 = {t1};
        assert(std::get<0>(t2).state == VT::FromInt);
    }
}
