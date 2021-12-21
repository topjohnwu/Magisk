
#undef NDEBUG

#include "benchmark/benchmark.h"
#include "output_test.h"

// @todo: <jpmag> this checks the full output at once; the rule for
// CounterSet1 was failing because it was not matching "^[-]+$".
// @todo: <jpmag> check that the counters are vertically aligned.
ADD_CASES(
    TC_ConsoleOut,
    {
        // keeping these lines long improves readability, so:
        // clang-format off
    {"^[-]+$", MR_Next},
    {"^Benchmark %s Time %s CPU %s Iterations %s Bar %s Bat %s Baz %s Foo %s Frob %s Lob$", MR_Next},
    {"^[-]+$", MR_Next},
    {"^BM_Counters_Tabular/threads:%int %console_report [ ]*%hrfloat [ ]*%hrfloat [ ]*%hrfloat [ ]*%hrfloat [ ]*%hrfloat [ ]*%hrfloat$", MR_Next},
    {"^BM_Counters_Tabular/threads:%int %console_report [ ]*%hrfloat [ ]*%hrfloat [ ]*%hrfloat [ ]*%hrfloat [ ]*%hrfloat [ ]*%hrfloat$", MR_Next},
    {"^BM_Counters_Tabular/threads:%int %console_report [ ]*%hrfloat [ ]*%hrfloat [ ]*%hrfloat [ ]*%hrfloat [ ]*%hrfloat [ ]*%hrfloat$", MR_Next},
    {"^BM_Counters_Tabular/threads:%int %console_report [ ]*%hrfloat [ ]*%hrfloat [ ]*%hrfloat [ ]*%hrfloat [ ]*%hrfloat [ ]*%hrfloat$", MR_Next},
    {"^BM_Counters_Tabular/threads:%int %console_report [ ]*%hrfloat [ ]*%hrfloat [ ]*%hrfloat [ ]*%hrfloat [ ]*%hrfloat [ ]*%hrfloat$", MR_Next},
    {"^BM_CounterRates_Tabular/threads:%int %console_report [ ]*%hrfloat/s [ ]*%hrfloat/s [ ]*%hrfloat/s [ ]*%hrfloat/s [ ]*%hrfloat/s [ ]*%hrfloat/s$", MR_Next},
    {"^BM_CounterRates_Tabular/threads:%int %console_report [ ]*%hrfloat/s [ ]*%hrfloat/s [ ]*%hrfloat/s [ ]*%hrfloat/s [ ]*%hrfloat/s [ ]*%hrfloat/s$", MR_Next},
    {"^BM_CounterRates_Tabular/threads:%int %console_report [ ]*%hrfloat/s [ ]*%hrfloat/s [ ]*%hrfloat/s [ ]*%hrfloat/s [ ]*%hrfloat/s [ ]*%hrfloat/s$", MR_Next},
    {"^BM_CounterRates_Tabular/threads:%int %console_report [ ]*%hrfloat/s [ ]*%hrfloat/s [ ]*%hrfloat/s [ ]*%hrfloat/s [ ]*%hrfloat/s [ ]*%hrfloat/s$", MR_Next},
    {"^BM_CounterRates_Tabular/threads:%int %console_report [ ]*%hrfloat/s [ ]*%hrfloat/s [ ]*%hrfloat/s [ ]*%hrfloat/s [ ]*%hrfloat/s [ ]*%hrfloat/s$", MR_Next},
    {"^[-]+$", MR_Next},
    {"^Benchmark %s Time %s CPU %s Iterations %s Bar %s Baz %s Foo$", MR_Next},
    {"^[-]+$", MR_Next},
    {"^BM_CounterSet0_Tabular/threads:%int %console_report [ ]*%hrfloat [ ]*%hrfloat [ ]*%hrfloat$", MR_Next},
    {"^BM_CounterSet0_Tabular/threads:%int %console_report [ ]*%hrfloat [ ]*%hrfloat [ ]*%hrfloat$", MR_Next},
    {"^BM_CounterSet0_Tabular/threads:%int %console_report [ ]*%hrfloat [ ]*%hrfloat [ ]*%hrfloat$", MR_Next},
    {"^BM_CounterSet0_Tabular/threads:%int %console_report [ ]*%hrfloat [ ]*%hrfloat [ ]*%hrfloat$", MR_Next},
    {"^BM_CounterSet0_Tabular/threads:%int %console_report [ ]*%hrfloat [ ]*%hrfloat [ ]*%hrfloat$", MR_Next},
    {"^BM_CounterSet1_Tabular/threads:%int %console_report [ ]*%hrfloat [ ]*%hrfloat [ ]*%hrfloat$", MR_Next},
    {"^BM_CounterSet1_Tabular/threads:%int %console_report [ ]*%hrfloat [ ]*%hrfloat [ ]*%hrfloat$", MR_Next},
    {"^BM_CounterSet1_Tabular/threads:%int %console_report [ ]*%hrfloat [ ]*%hrfloat [ ]*%hrfloat$", MR_Next},
    {"^BM_CounterSet1_Tabular/threads:%int %console_report [ ]*%hrfloat [ ]*%hrfloat [ ]*%hrfloat$", MR_Next},
    {"^BM_CounterSet1_Tabular/threads:%int %console_report [ ]*%hrfloat [ ]*%hrfloat [ ]*%hrfloat$", MR_Next},
    {"^[-]+$", MR_Next},
    {"^Benchmark %s Time %s CPU %s Iterations %s Bat %s Baz %s Foo$", MR_Next},
    {"^[-]+$", MR_Next},
    {"^BM_CounterSet2_Tabular/threads:%int %console_report [ ]*%hrfloat [ ]*%hrfloat [ ]*%hrfloat$", MR_Next},
    {"^BM_CounterSet2_Tabular/threads:%int %console_report [ ]*%hrfloat [ ]*%hrfloat [ ]*%hrfloat$", MR_Next},
    {"^BM_CounterSet2_Tabular/threads:%int %console_report [ ]*%hrfloat [ ]*%hrfloat [ ]*%hrfloat$", MR_Next},
    {"^BM_CounterSet2_Tabular/threads:%int %console_report [ ]*%hrfloat [ ]*%hrfloat [ ]*%hrfloat$", MR_Next},
    {"^BM_CounterSet2_Tabular/threads:%int %console_report [ ]*%hrfloat [ ]*%hrfloat [ ]*%hrfloat$"},
        // clang-format on
    });
ADD_CASES(TC_CSVOut, {{"%csv_header,"
                       "\"Bar\",\"Bat\",\"Baz\",\"Foo\",\"Frob\",\"Lob\""}});

// ========================================================================= //
// ------------------------- Tabular Counters Output ----------------------- //
// ========================================================================= //

void BM_Counters_Tabular(benchmark::State& state) {
  for (auto _ : state) {
  }
  namespace bm = benchmark;
  state.counters.insert({
      {"Foo", {1, bm::Counter::kAvgThreads}},
      {"Bar", {2, bm::Counter::kAvgThreads}},
      {"Baz", {4, bm::Counter::kAvgThreads}},
      {"Bat", {8, bm::Counter::kAvgThreads}},
      {"Frob", {16, bm::Counter::kAvgThreads}},
      {"Lob", {32, bm::Counter::kAvgThreads}},
  });
}
BENCHMARK(BM_Counters_Tabular)->ThreadRange(1, 16);
ADD_CASES(TC_JSONOut,
          {{"\"name\": \"BM_Counters_Tabular/threads:%int\",$"},
           {"\"run_name\": \"BM_Counters_Tabular/threads:%int\",$", MR_Next},
           {"\"run_type\": \"iteration\",$", MR_Next},
           {"\"iterations\": %int,$", MR_Next},
           {"\"real_time\": %float,$", MR_Next},
           {"\"cpu_time\": %float,$", MR_Next},
           {"\"time_unit\": \"ns\",$", MR_Next},
           {"\"Bar\": %float,$", MR_Next},
           {"\"Bat\": %float,$", MR_Next},
           {"\"Baz\": %float,$", MR_Next},
           {"\"Foo\": %float,$", MR_Next},
           {"\"Frob\": %float,$", MR_Next},
           {"\"Lob\": %float$", MR_Next},
           {"}", MR_Next}});
ADD_CASES(TC_CSVOut, {{"^\"BM_Counters_Tabular/threads:%int\",%csv_report,"
                       "%float,%float,%float,%float,%float,%float$"}});
// VS2013 does not allow this function to be passed as a lambda argument
// to CHECK_BENCHMARK_RESULTS()
void CheckTabular(Results const& e) {
  CHECK_COUNTER_VALUE(e, int, "Foo", EQ, 1);
  CHECK_COUNTER_VALUE(e, int, "Bar", EQ, 2);
  CHECK_COUNTER_VALUE(e, int, "Baz", EQ, 4);
  CHECK_COUNTER_VALUE(e, int, "Bat", EQ, 8);
  CHECK_COUNTER_VALUE(e, int, "Frob", EQ, 16);
  CHECK_COUNTER_VALUE(e, int, "Lob", EQ, 32);
}
CHECK_BENCHMARK_RESULTS("BM_Counters_Tabular/threads:%int", &CheckTabular);

// ========================================================================= //
// -------------------- Tabular+Rate Counters Output ----------------------- //
// ========================================================================= //

void BM_CounterRates_Tabular(benchmark::State& state) {
  for (auto _ : state) {
  }
  namespace bm = benchmark;
  state.counters.insert({
      {"Foo", {1, bm::Counter::kAvgThreadsRate}},
      {"Bar", {2, bm::Counter::kAvgThreadsRate}},
      {"Baz", {4, bm::Counter::kAvgThreadsRate}},
      {"Bat", {8, bm::Counter::kAvgThreadsRate}},
      {"Frob", {16, bm::Counter::kAvgThreadsRate}},
      {"Lob", {32, bm::Counter::kAvgThreadsRate}},
  });
}
BENCHMARK(BM_CounterRates_Tabular)->ThreadRange(1, 16);
ADD_CASES(TC_JSONOut,
          {{"\"name\": \"BM_CounterRates_Tabular/threads:%int\",$"},
           {"\"run_name\": \"BM_CounterRates_Tabular/threads:%int\",$",
            MR_Next},
           {"\"run_type\": \"iteration\",$", MR_Next},
           {"\"iterations\": %int,$", MR_Next},
           {"\"real_time\": %float,$", MR_Next},
           {"\"cpu_time\": %float,$", MR_Next},
           {"\"time_unit\": \"ns\",$", MR_Next},
           {"\"Bar\": %float,$", MR_Next},
           {"\"Bat\": %float,$", MR_Next},
           {"\"Baz\": %float,$", MR_Next},
           {"\"Foo\": %float,$", MR_Next},
           {"\"Frob\": %float,$", MR_Next},
           {"\"Lob\": %float$", MR_Next},
           {"}", MR_Next}});
ADD_CASES(TC_CSVOut, {{"^\"BM_CounterRates_Tabular/threads:%int\",%csv_report,"
                       "%float,%float,%float,%float,%float,%float$"}});
// VS2013 does not allow this function to be passed as a lambda argument
// to CHECK_BENCHMARK_RESULTS()
void CheckTabularRate(Results const& e) {
  double t = e.DurationCPUTime();
  CHECK_FLOAT_COUNTER_VALUE(e, "Foo", EQ, 1. / t, 0.001);
  CHECK_FLOAT_COUNTER_VALUE(e, "Bar", EQ, 2. / t, 0.001);
  CHECK_FLOAT_COUNTER_VALUE(e, "Baz", EQ, 4. / t, 0.001);
  CHECK_FLOAT_COUNTER_VALUE(e, "Bat", EQ, 8. / t, 0.001);
  CHECK_FLOAT_COUNTER_VALUE(e, "Frob", EQ, 16. / t, 0.001);
  CHECK_FLOAT_COUNTER_VALUE(e, "Lob", EQ, 32. / t, 0.001);
}
CHECK_BENCHMARK_RESULTS("BM_CounterRates_Tabular/threads:%int",
                        &CheckTabularRate);

// ========================================================================= //
// ------------------------- Tabular Counters Output ----------------------- //
// ========================================================================= //

// set only some of the counters
void BM_CounterSet0_Tabular(benchmark::State& state) {
  for (auto _ : state) {
  }
  namespace bm = benchmark;
  state.counters.insert({
      {"Foo", {10, bm::Counter::kAvgThreads}},
      {"Bar", {20, bm::Counter::kAvgThreads}},
      {"Baz", {40, bm::Counter::kAvgThreads}},
  });
}
BENCHMARK(BM_CounterSet0_Tabular)->ThreadRange(1, 16);
ADD_CASES(TC_JSONOut,
          {{"\"name\": \"BM_CounterSet0_Tabular/threads:%int\",$"},
           {"\"run_name\": \"BM_CounterSet0_Tabular/threads:%int\",$", MR_Next},
           {"\"run_type\": \"iteration\",$", MR_Next},
           {"\"iterations\": %int,$", MR_Next},
           {"\"real_time\": %float,$", MR_Next},
           {"\"cpu_time\": %float,$", MR_Next},
           {"\"time_unit\": \"ns\",$", MR_Next},
           {"\"Bar\": %float,$", MR_Next},
           {"\"Baz\": %float,$", MR_Next},
           {"\"Foo\": %float$", MR_Next},
           {"}", MR_Next}});
ADD_CASES(TC_CSVOut, {{"^\"BM_CounterSet0_Tabular/threads:%int\",%csv_report,"
                       "%float,,%float,%float,,"}});
// VS2013 does not allow this function to be passed as a lambda argument
// to CHECK_BENCHMARK_RESULTS()
void CheckSet0(Results const& e) {
  CHECK_COUNTER_VALUE(e, int, "Foo", EQ, 10);
  CHECK_COUNTER_VALUE(e, int, "Bar", EQ, 20);
  CHECK_COUNTER_VALUE(e, int, "Baz", EQ, 40);
}
CHECK_BENCHMARK_RESULTS("BM_CounterSet0_Tabular", &CheckSet0);

// again.
void BM_CounterSet1_Tabular(benchmark::State& state) {
  for (auto _ : state) {
  }
  namespace bm = benchmark;
  state.counters.insert({
      {"Foo", {15, bm::Counter::kAvgThreads}},
      {"Bar", {25, bm::Counter::kAvgThreads}},
      {"Baz", {45, bm::Counter::kAvgThreads}},
  });
}
BENCHMARK(BM_CounterSet1_Tabular)->ThreadRange(1, 16);
ADD_CASES(TC_JSONOut,
          {{"\"name\": \"BM_CounterSet1_Tabular/threads:%int\",$"},
           {"\"run_name\": \"BM_CounterSet1_Tabular/threads:%int\",$", MR_Next},
           {"\"run_type\": \"iteration\",$", MR_Next},
           {"\"iterations\": %int,$", MR_Next},
           {"\"real_time\": %float,$", MR_Next},
           {"\"cpu_time\": %float,$", MR_Next},
           {"\"time_unit\": \"ns\",$", MR_Next},
           {"\"Bar\": %float,$", MR_Next},
           {"\"Baz\": %float,$", MR_Next},
           {"\"Foo\": %float$", MR_Next},
           {"}", MR_Next}});
ADD_CASES(TC_CSVOut, {{"^\"BM_CounterSet1_Tabular/threads:%int\",%csv_report,"
                       "%float,,%float,%float,,"}});
// VS2013 does not allow this function to be passed as a lambda argument
// to CHECK_BENCHMARK_RESULTS()
void CheckSet1(Results const& e) {
  CHECK_COUNTER_VALUE(e, int, "Foo", EQ, 15);
  CHECK_COUNTER_VALUE(e, int, "Bar", EQ, 25);
  CHECK_COUNTER_VALUE(e, int, "Baz", EQ, 45);
}
CHECK_BENCHMARK_RESULTS("BM_CounterSet1_Tabular/threads:%int", &CheckSet1);

// ========================================================================= //
// ------------------------- Tabular Counters Output ----------------------- //
// ========================================================================= //

// set only some of the counters, different set now.
void BM_CounterSet2_Tabular(benchmark::State& state) {
  for (auto _ : state) {
  }
  namespace bm = benchmark;
  state.counters.insert({
      {"Foo", {10, bm::Counter::kAvgThreads}},
      {"Bat", {30, bm::Counter::kAvgThreads}},
      {"Baz", {40, bm::Counter::kAvgThreads}},
  });
}
BENCHMARK(BM_CounterSet2_Tabular)->ThreadRange(1, 16);
ADD_CASES(TC_JSONOut,
          {{"\"name\": \"BM_CounterSet2_Tabular/threads:%int\",$"},
           {"\"run_name\": \"BM_CounterSet2_Tabular/threads:%int\",$", MR_Next},
           {"\"run_type\": \"iteration\",$", MR_Next},
           {"\"iterations\": %int,$", MR_Next},
           {"\"real_time\": %float,$", MR_Next},
           {"\"cpu_time\": %float,$", MR_Next},
           {"\"time_unit\": \"ns\",$", MR_Next},
           {"\"Bat\": %float,$", MR_Next},
           {"\"Baz\": %float,$", MR_Next},
           {"\"Foo\": %float$", MR_Next},
           {"}", MR_Next}});
ADD_CASES(TC_CSVOut, {{"^\"BM_CounterSet2_Tabular/threads:%int\",%csv_report,"
                       ",%float,%float,%float,,"}});
// VS2013 does not allow this function to be passed as a lambda argument
// to CHECK_BENCHMARK_RESULTS()
void CheckSet2(Results const& e) {
  CHECK_COUNTER_VALUE(e, int, "Foo", EQ, 10);
  CHECK_COUNTER_VALUE(e, int, "Bat", EQ, 30);
  CHECK_COUNTER_VALUE(e, int, "Baz", EQ, 40);
}
CHECK_BENCHMARK_RESULTS("BM_CounterSet2_Tabular", &CheckSet2);

// ========================================================================= //
// --------------------------- TEST CASES END ------------------------------ //
// ========================================================================= //

int main(int argc, char* argv[]) { RunOutputTests(argc, argv); }
