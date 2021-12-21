//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: clang-3.3, clang-3.4, clang-3.5

// <vector>

// reference operator[](size_type n);

#include <vector>
#include <cassert>
#include <cstdlib>

#include "asan_testing.h"
#include "min_allocator.h"
#include "test_iterators.h"
#include "test_macros.h"

#ifndef _LIBCPP_HAS_NO_ASAN
extern "C" void __sanitizer_set_death_callback(void (*callback)(void));

void do_exit() {
  exit(0);
}

int main()
{
#if TEST_STD_VER >= 11
    {
        typedef int T;
        typedef std::vector<T, min_allocator<T>> C;
        const T t[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        C c(std::begin(t), std::end(t));
        c.reserve(2*c.size());
        volatile T foo = c[c.size()];    // bad, but not caught by ASAN
        ((void)foo);
    }
#endif

    {
        typedef input_iterator<int*> MyInputIter;
        // Sould not trigger ASan.
        std::vector<int> v;
        v.reserve(1);
        int i[] = {42};
        v.insert(v.begin(), MyInputIter(i), MyInputIter(i + 1));
        assert(v[0] == 42);
        assert(is_contiguous_container_asan_correct(v));
    }

    __sanitizer_set_death_callback(do_exit);
    {
        typedef int T;
        typedef std::vector<T> C;
        const T t[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        C c(std::begin(t), std::end(t));
        c.reserve(2*c.size());
        assert(is_contiguous_container_asan_correct(c));
        assert(!__sanitizer_verify_contiguous_container( c.data(), c.data() + 1, c.data() + c.capacity()));
        volatile T foo = c[c.size()]; // should trigger ASAN. Use volatile to prevent being optimized away.
        assert(false);          // if we got here, ASAN didn't trigger
        ((void)foo);
    }
}
#else
int main () { return 0; }
#endif
