
#include "benchmark/benchmark.h"

#include <cassert>
#include <memory>

class MyFixture : public ::benchmark::Fixture {
 public:
  void SetUp(const ::benchmark::State& state) {
    if (state.thread_index == 0) {
      assert(data.get() == nullptr);
      data.reset(new int(42));
    }
  }

  void TearDown(const ::benchmark::State& state) {
    if (state.thread_index == 0) {
      assert(data.get() != nullptr);
      data.reset();
    }
  }

  ~MyFixture() { assert(data == nullptr); }

  std::unique_ptr<int> data;
};

BENCHMARK_F(MyFixture, Foo)(benchmark::State &st) {
  assert(data.get() != nullptr);
  assert(*data == 42);
  for (auto _ : st) {
  }
}

BENCHMARK_DEFINE_F(MyFixture, Bar)(benchmark::State& st) {
  if (st.thread_index == 0) {
    assert(data.get() != nullptr);
    assert(*data == 42);
  }
  for (auto _ : st) {
    assert(data.get() != nullptr);
    assert(*data == 42);
  }
  st.SetItemsProcessed(st.range(0));
}
BENCHMARK_REGISTER_F(MyFixture, Bar)->Arg(42);
BENCHMARK_REGISTER_F(MyFixture, Bar)->Arg(42)->ThreadPerCpu();

BENCHMARK_MAIN();
