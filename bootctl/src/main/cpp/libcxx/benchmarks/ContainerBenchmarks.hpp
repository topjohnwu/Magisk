#ifndef BENCHMARK_CONTAINER_BENCHMARKS_HPP
#define BENCHMARK_CONTAINER_BENCHMARKS_HPP

#include <cassert>

#include "benchmark/benchmark.h"

namespace ContainerBenchmarks {


template <class Container, class GenInputs>
void BM_ConstructIterIter(benchmark::State& st, Container, GenInputs gen) {
    auto in = gen(st.range(0));
    const auto begin = in.begin();
    const auto end = in.end();
    benchmark::DoNotOptimize(&in);
    while (st.KeepRunning()) {
        Container c(begin, end);
        benchmark::DoNotOptimize(c.data());
    }
}

template <class Container, class GenInputs>
void BM_InsertValue(benchmark::State& st, Container c, GenInputs gen) {
    auto in = gen(st.range(0));
    const auto end = in.end();
    while (st.KeepRunning()) {
        c.clear();
        for (auto it = in.begin(); it != end; ++it) {
            benchmark::DoNotOptimize(&(*c.insert(*it).first));
        }
        benchmark::ClobberMemory();
    }
}

template <class Container, class GenInputs>
void BM_InsertValueRehash(benchmark::State& st, Container c, GenInputs gen) {
    auto in = gen(st.range(0));
    const auto end = in.end();
    while (st.KeepRunning()) {
        c.clear();
        c.rehash(16);
        for (auto it = in.begin(); it != end; ++it) {
            benchmark::DoNotOptimize(&(*c.insert(*it).first));
        }
        benchmark::ClobberMemory();
    }
}


template <class Container, class GenInputs>
void BM_InsertDuplicate(benchmark::State& st, Container c, GenInputs gen) {
    auto in = gen(st.range(0));
    const auto end = in.end();
    c.insert(in.begin(), in.end());
    benchmark::DoNotOptimize(&c);
    benchmark::DoNotOptimize(&in);
    while (st.KeepRunning()) {
        for (auto it = in.begin(); it != end; ++it) {
            benchmark::DoNotOptimize(&(*c.insert(*it).first));
        }
        benchmark::ClobberMemory();
    }
}


template <class Container, class GenInputs>
void BM_EmplaceDuplicate(benchmark::State& st, Container c, GenInputs gen) {
    auto in = gen(st.range(0));
    const auto end = in.end();
    c.insert(in.begin(), in.end());
    benchmark::DoNotOptimize(&c);
    benchmark::DoNotOptimize(&in);
    while (st.KeepRunning()) {
        for (auto it = in.begin(); it != end; ++it) {
            benchmark::DoNotOptimize(&(*c.emplace(*it).first));
        }
        benchmark::ClobberMemory();
    }
}

template <class Container, class GenInputs>
static void BM_Find(benchmark::State& st, Container c, GenInputs gen) {
    auto in = gen(st.range(0));
    c.insert(in.begin(), in.end());
    benchmark::DoNotOptimize(&(*c.begin()));
    const auto end = in.data() + in.size();
    while (st.KeepRunning()) {
        for (auto it = in.data(); it != end; ++it) {
            benchmark::DoNotOptimize(&(*c.find(*it)));
        }
        benchmark::ClobberMemory();
    }
}

template <class Container, class GenInputs>
static void BM_FindRehash(benchmark::State& st, Container c, GenInputs gen) {
    c.rehash(8);
    auto in = gen(st.range(0));
    c.insert(in.begin(), in.end());
    benchmark::DoNotOptimize(&(*c.begin()));
    const auto end = in.data() + in.size();
    while (st.KeepRunning()) {
        for (auto it = in.data(); it != end; ++it) {
            benchmark::DoNotOptimize(&(*c.find(*it)));
        }
        benchmark::ClobberMemory();
    }
}

} // end namespace ContainerBenchmarks

#endif // BENCHMARK_CONTAINER_BENCHMARKS_HPP
