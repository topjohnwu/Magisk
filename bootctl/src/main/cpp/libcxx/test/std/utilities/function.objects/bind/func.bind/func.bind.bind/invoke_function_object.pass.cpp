//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <functional>

// template<CopyConstructible Fn, CopyConstructible... Types>
//   unspecified bind(Fn, Types...);
// template<Returnable R, CopyConstructible Fn, CopyConstructible... Types>
//   unspecified bind(Fn, Types...);

// https://bugs.llvm.org/show_bug.cgi?id=22003

#include <functional>

struct DummyUnaryFunction
{
    template <typename S>
    int operator()(S const &) const { return 0; }
};

struct BadUnaryFunction
{
    template <typename S>
    constexpr int operator()(S const &) const
    {
        // Trigger a compile error if this function is instantiated.
        // The constexpr is needed so that it is instantiated while checking
        // __invoke_of<BadUnaryFunction &, ...>.
        static_assert(!std::is_same<S, S>::value, "Shit");
        return 0;
    }
};

int main()
{
    // Check that BadUnaryFunction::operator()(S const &) is not
    // instantiated when checking if BadUnaryFunction is a nested bind
    // expression during b(0). See PR22003.
    auto b = std::bind(DummyUnaryFunction(), BadUnaryFunction());
    b(0);
    auto b2 = std::bind<long>(DummyUnaryFunction(), BadUnaryFunction());
    b2(0);
}
