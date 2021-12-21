//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <iterator>

// front_insert_iterator

// Test nested types and data member:

// template <class Container>
// class front_insert_iterator {
// protected:
//   Container* container;
// public:
//   typedef Container                   container_type;
//   typedef void                        value_type;
//   typedef void                        difference_type;
//   typedef void                        reference;
//   typedef void                        pointer;
//   typedef output_iterator_tag         iterator_category;
// };

#include <iterator>
#include <type_traits>
#include <vector>

template <class C>
struct find_container
    : private std::front_insert_iterator<C>
{
    explicit find_container(C& c) : std::front_insert_iterator<C>(c) {}
    void test() {this->container = 0;}
};

template <class C>
void
test()
{
    typedef std::front_insert_iterator<C> R;
    C c;
    find_container<C> q(c);
    q.test();
    static_assert((std::is_same<typename R::container_type, C>::value), "");
    static_assert((std::is_same<typename R::value_type, void>::value), "");
    static_assert((std::is_same<typename R::difference_type, void>::value), "");
    static_assert((std::is_same<typename R::reference, void>::value), "");
    static_assert((std::is_same<typename R::pointer, void>::value), "");
    static_assert((std::is_same<typename R::iterator_category, std::output_iterator_tag>::value), "");
}

int main()
{
    test<std::vector<int> >();
}
