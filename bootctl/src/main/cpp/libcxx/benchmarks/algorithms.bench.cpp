
#include <algorithm>
#include <cstdint>
#include <map>
#include <random>
#include <string>
#include <utility>
#include <vector>

#include "CartesianBenchmarks.hpp"
#include "GenerateInput.hpp"
#include "benchmark/benchmark.h"
#include "test_macros.h"

namespace {

enum class ValueType { Uint32, String };
struct AllValueTypes : EnumValuesAsTuple<AllValueTypes, ValueType, 2> {
  static constexpr const char* Names[] = {"uint32", "string"};
};

template <class V>
using Value =
    std::conditional_t<V() == ValueType::Uint32, uint32_t, std::string>;

enum class Order {
  Random,
  Ascending,
  Descending,
  SingleElement,
  PipeOrgan,
  Heap
};
struct AllOrders : EnumValuesAsTuple<AllOrders, Order, 6> {
  static constexpr const char* Names[] = {"Random",     "Ascending",
                                          "Descending", "SingleElement",
                                          "PipeOrgan",  "Heap"};
};

void fillValues(std::vector<uint32_t>& V, size_t N, Order O) {
  if (O == Order::SingleElement) {
    V.resize(N, 0);
  } else {
    while (V.size() < N)
      V.push_back(V.size());
  }
}

void fillValues(std::vector<std::string>& V, size_t N, Order O) {

  if (O == Order::SingleElement) {
    V.resize(N, getRandomString(1024));
  } else {
    while (V.size() < N)
      V.push_back(getRandomString(1024));
  }
}

template <class T>
void sortValues(T& V, Order O) {
  assert(std::is_sorted(V.begin(), V.end()));
  switch (O) {
  case Order::Random: {
    std::random_device R;
    std::mt19937 M(R());
    std::shuffle(V.begin(), V.end(), M);
    break;
  }
  case Order::Ascending:
    std::sort(V.begin(), V.end());
    break;
  case Order::Descending:
    std::sort(V.begin(), V.end(), std::greater<>());
    break;
  case Order::SingleElement:
    // Nothing to do
    break;
  case Order::PipeOrgan:
    std::sort(V.begin(), V.end());
    std::reverse(V.begin() + V.size() / 2, V.end());
    break;
  case Order::Heap:
    std::make_heap(V.begin(), V.end());
    break;
  }
}

template <class ValueType>
std::vector<std::vector<Value<ValueType> > > makeOrderedValues(size_t N,
                                                               Order O) {
  // Let's make sure that all random sequences of the same size are the same.
  // That way we can compare the different algorithms with the same input.
  static std::map<std::pair<size_t, Order>, std::vector<Value<ValueType> > >
      Cached;

  auto& Values = Cached[{N, O}];
  if (Values.empty()) {
    fillValues(Values, N, O);
    sortValues(Values, O);
  };
  const size_t NumCopies = std::max(size_t{1}, 1000 / N);
  return { NumCopies, Values };
}

template <class T, class U>
TEST_ALWAYS_INLINE void resetCopies(benchmark::State& state, T& Copies,
                                    U& Orig) {
  state.PauseTiming();
  for (auto& Copy : Copies)
    Copy = Orig;
  state.ResumeTiming();
}

template <class ValueType, class F>
void runOpOnCopies(benchmark::State& state, size_t Quantity, Order O,
                   bool CountElements, F f) {
  auto Copies = makeOrderedValues<ValueType>(Quantity, O);
  const auto Orig = Copies[0];

  const size_t Batch = CountElements ? Copies.size() * Quantity : Copies.size();
  while (state.KeepRunningBatch(Batch)) {
    for (auto& Copy : Copies) {
      f(Copy);
      benchmark::DoNotOptimize(Copy);
    }
    resetCopies(state, Copies, Orig);
  }
}

template <class ValueType, class Order>
struct Sort {
  size_t Quantity;

  void run(benchmark::State& state) const {
    runOpOnCopies<ValueType>(state, Quantity, Order(), false, [](auto& Copy) {
      std::sort(Copy.begin(), Copy.end());
    });
  }

  bool skip() const { return Order() == ::Order::Heap; }

  std::string name() const {
    return "BM_Sort" + ValueType::name() + Order::name() + "_" +
           std::to_string(Quantity);
  };
};

template <class ValueType, class Order>
struct StableSort {
  size_t Quantity;

  void run(benchmark::State& state) const {
    runOpOnCopies<ValueType>(state, Quantity, Order(), false, [](auto& Copy) {
      std::stable_sort(Copy.begin(), Copy.end());
    });
  }

  bool skip() const { return Order() == ::Order::Heap; }

  std::string name() const {
    return "BM_StableSort" + ValueType::name() + Order::name() + "_" +
           std::to_string(Quantity);
  };
};

template <class ValueType, class Order>
struct MakeHeap {
  size_t Quantity;

  void run(benchmark::State& state) const {
    runOpOnCopies<ValueType>(state, Quantity, Order(), false, [](auto& Copy) {
      std::make_heap(Copy.begin(), Copy.end());
    });
  }

  std::string name() const {
    return "BM_MakeHeap" + ValueType::name() + Order::name() + "_" +
           std::to_string(Quantity);
  };
};

template <class ValueType>
struct SortHeap {
  size_t Quantity;

  void run(benchmark::State& state) const {
    runOpOnCopies<ValueType>(
        state, Quantity, Order::Heap, false,
        [](auto& Copy) { std::sort_heap(Copy.begin(), Copy.end()); });
  }

  std::string name() const {
    return "BM_SortHeap" + ValueType::name() + "_" + std::to_string(Quantity);
  };
};

template <class ValueType, class Order>
struct MakeThenSortHeap {
  size_t Quantity;

  void run(benchmark::State& state) const {
    runOpOnCopies<ValueType>(state, Quantity, Order(), false, [](auto& Copy) {
      std::make_heap(Copy.begin(), Copy.end());
      std::sort_heap(Copy.begin(), Copy.end());
    });
  }

  std::string name() const {
    return "BM_MakeThenSortHeap" + ValueType::name() + Order::name() + "_" +
           std::to_string(Quantity);
  };
};

template <class ValueType, class Order>
struct PushHeap {
  size_t Quantity;

  void run(benchmark::State& state) const {
    runOpOnCopies<ValueType>(state, Quantity, Order(), true, [](auto& Copy) {
      for (auto I = Copy.begin(), E = Copy.end(); I != E; ++I) {
        std::push_heap(Copy.begin(), I + 1);
      }
    });
  }

  bool skip() const { return Order() == ::Order::Heap; }

  std::string name() const {
    return "BM_PushHeap" + ValueType::name() + Order::name() + "_" +
           std::to_string(Quantity);
  };
};

template <class ValueType>
struct PopHeap {
  size_t Quantity;

  void run(benchmark::State& state) const {
    runOpOnCopies<ValueType>(state, Quantity, Order(), true, [](auto& Copy) {
      for (auto B = Copy.begin(), I = Copy.end(); I != B; --I) {
        std::pop_heap(B, I);
      }
    });
  }

  std::string name() const {
    return "BM_PopHeap" + ValueType::name() + "_" + std::to_string(Quantity);
  };
};

} // namespace

int main(int argc, char** argv) {
  benchmark::Initialize(&argc, argv);
  if (benchmark::ReportUnrecognizedArguments(argc, argv))
    return 1;

  const std::vector<size_t> Quantities = {1 << 0, 1 << 2,  1 << 4,  1 << 6,
                                          1 << 8, 1 << 10, 1 << 14, 1 << 18};
  makeCartesianProductBenchmark<Sort, AllValueTypes, AllOrders>(Quantities);
  makeCartesianProductBenchmark<StableSort, AllValueTypes, AllOrders>(
      Quantities);
  makeCartesianProductBenchmark<MakeHeap, AllValueTypes, AllOrders>(Quantities);
  makeCartesianProductBenchmark<SortHeap, AllValueTypes>(Quantities);
  makeCartesianProductBenchmark<MakeThenSortHeap, AllValueTypes, AllOrders>(
      Quantities);
  makeCartesianProductBenchmark<PushHeap, AllValueTypes, AllOrders>(Quantities);
  makeCartesianProductBenchmark<PopHeap, AllValueTypes>(Quantities);
  benchmark::RunSpecifiedBenchmarks();
}
