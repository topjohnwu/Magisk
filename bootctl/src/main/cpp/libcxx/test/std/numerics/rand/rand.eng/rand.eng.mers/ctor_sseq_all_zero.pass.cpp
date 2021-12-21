//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <random>

// template <class UIntType, size_t w, size_t n, size_t m, size_t r,
//           UIntType a, size_t u, UIntType d, size_t s,
//           UIntType b, size_t t,
//           UIntType c, size_t l, UIntType f>
// class mersenne_twister_engine;

// template <class Sseq> explicit mersenne_twister_engine(Sseq &q);
//
//     [ ... ] Finally, if the most significant $w-r$ bits of $X_{-n}$ are zero,
//     and if each of the other resulting $X_i$ is $0$, changes $X_{-n}$ to
//     $ 2^{w-1} $.

#include <random>

#include <algorithm>
#include <cassert>
#include <cstddef>
#if TEST_STD_VER >= 11
#include <initializer_list>
#endif

struct all_zero_seed_seq {
  typedef unsigned int result_type;

  all_zero_seed_seq() {}

  template <typename InputIterator>
  all_zero_seed_seq(InputIterator, InputIterator) {}
#if TEST_STD_VER >= 11
  all_zero_seed_seq(std::initializer_list<result_type>) {}
#endif

  template <typename RandomAccessIterator>
  void generate(RandomAccessIterator rb, RandomAccessIterator re) {
    std::fill(rb, re, 0u);
  }

  std::size_t size() const { return 0u; }
  template <typename OutputIterator> void param(OutputIterator) const {}
};

template <typename result_type, std::size_t word_size>
void test(void) {
  const std::size_t state_size = 1u;
  const std::size_t shift_size = 1u;
  const std::size_t tempering_l = word_size;

  all_zero_seed_seq q;
  std::mersenne_twister_engine<result_type, word_size, state_size,
                               shift_size,
                               0u,
                               0x0,
                               0u, 0x0, 0u, 0x0, 0u, 0x0,
                               tempering_l,
                               0u>
      e(q);

  const result_type Xneg1 = result_type(1) << (word_size - 1);
  const result_type Y = Xneg1;
  const result_type X0 = Xneg1 ^ (Y >> 1);
  assert(e() == X0);
}

int main() {
  // Test for k == 1: word_size <= 32.
  test<unsigned short, 3u>();

  // Test for k == 2: (32 < word_size <= 64).
  test<unsigned long long, 33u>();
}
