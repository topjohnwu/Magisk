#include "benchmark/benchmark.h"

#include <cstdlib>
#include <map>

namespace {

std::map<int, int> ConstructRandomMap(int size) {
  std::map<int, int> m;
  for (int i = 0; i < size; ++i) {
    m.insert(std::make_pair(std::rand() % size, std::rand() % size));
  }
  return m;
}

}  // namespace

// Basic version.
static void BM_MapLookup(benchmark::State& state) {
  const int size = static_cast<int>(state.range(0));
  std::map<int, int> m;
  for (auto _ : state) {
    state.PauseTiming();
    m = ConstructRandomMap(size);
    state.ResumeTiming();
    for (int i = 0; i < size; ++i) {
      benchmark::DoNotOptimize(m.find(std::rand() % size));
    }
  }
  state.SetItemsProcessed(state.iterations() * size);
}
BENCHMARK(BM_MapLookup)->Range(1 << 3, 1 << 12);

// Using fixtures.
class MapFixture : public ::benchmark::Fixture {
 public:
  void SetUp(const ::benchmark::State& st) {
    m = ConstructRandomMap(static_cast<int>(st.range(0)));
  }

  void TearDown(const ::benchmark::State&) { m.clear(); }

  std::map<int, int> m;
};

BENCHMARK_DEFINE_F(MapFixture, Lookup)(benchmark::State& state) {
  const int size = static_cast<int>(state.range(0));
  for (auto _ : state) {
    for (int i = 0; i < size; ++i) {
      benchmark::DoNotOptimize(m.find(std::rand() % size));
    }
  }
  state.SetItemsProcessed(state.iterations() * size);
}
BENCHMARK_REGISTER_F(MapFixture, Lookup)->Range(1 << 3, 1 << 12);

BENCHMARK_MAIN();
