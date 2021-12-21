#ifndef POINTER_COMPARISON_TEST_HELPER_HPP
#define POINTER_COMPARISON_TEST_HELPER_HPP

#include <vector>
#include <memory>
#include <cstdint>
#include <cassert>

#include "test_macros.h"

template <class T, template<class> class CompareTemplate>
void do_pointer_comparison_test() {
    typedef CompareTemplate<T*> Compare;
    typedef CompareTemplate<std::uintptr_t> UIntCompare;
#if TEST_STD_VER > 11
    typedef CompareTemplate<void> VoidCompare;
#else
    typedef Compare VoidCompare;
#endif
    std::vector<std::shared_ptr<T> > pointers;
    const std::size_t test_size = 100;
    for (size_t i=0; i < test_size; ++i)
        pointers.push_back(std::shared_ptr<T>(new T()));
    Compare comp;
    UIntCompare ucomp;
    VoidCompare vcomp;
    for (size_t i=0; i < test_size; ++i) {
        for (size_t j=0; j < test_size; ++j) {
            T* lhs = pointers[i].get();
            T* rhs = pointers[j].get();
            std::uintptr_t lhs_uint = reinterpret_cast<std::uintptr_t>(lhs);
            std::uintptr_t rhs_uint = reinterpret_cast<std::uintptr_t>(rhs);
            assert(comp(lhs, rhs) == ucomp(lhs_uint, rhs_uint));
            assert(vcomp(lhs, rhs) == ucomp(lhs_uint, rhs_uint));
        }
    }
}

#endif // POINTER_COMPARISON_TEST_HELPER_HPP
