
#include <cstdint>
#include <new>
#include <vector>

#include "CartesianBenchmarks.hpp"
#include "GenerateInput.hpp"
#include "benchmark/benchmark.h"
#include "test_macros.h"

constexpr std::size_t MAX_STRING_LEN = 8 << 14;

// Benchmark when there is no match.
static void BM_StringFindNoMatch(benchmark::State &state) {
  std::string s1(state.range(0), '-');
  std::string s2(8, '*');
  for (auto _ : state)
    benchmark::DoNotOptimize(s1.find(s2));
}
BENCHMARK(BM_StringFindNoMatch)->Range(10, MAX_STRING_LEN);

// Benchmark when the string matches first time.
static void BM_StringFindAllMatch(benchmark::State &state) {
  std::string s1(MAX_STRING_LEN, '-');
  std::string s2(state.range(0), '-');
  for (auto _ : state)
    benchmark::DoNotOptimize(s1.find(s2));
}
BENCHMARK(BM_StringFindAllMatch)->Range(1, MAX_STRING_LEN);

// Benchmark when the string matches somewhere in the end.
static void BM_StringFindMatch1(benchmark::State &state) {
  std::string s1(MAX_STRING_LEN / 2, '*');
  s1 += std::string(state.range(0), '-');
  std::string s2(state.range(0), '-');
  for (auto _ : state)
    benchmark::DoNotOptimize(s1.find(s2));
}
BENCHMARK(BM_StringFindMatch1)->Range(1, MAX_STRING_LEN / 4);

// Benchmark when the string matches somewhere from middle to the end.
static void BM_StringFindMatch2(benchmark::State &state) {
  std::string s1(MAX_STRING_LEN / 2, '*');
  s1 += std::string(state.range(0), '-');
  s1 += std::string(state.range(0), '*');
  std::string s2(state.range(0), '-');
  for (auto _ : state)
    benchmark::DoNotOptimize(s1.find(s2));
}
BENCHMARK(BM_StringFindMatch2)->Range(1, MAX_STRING_LEN / 4);

static void BM_StringCtorDefault(benchmark::State &state) {
  for (auto _ : state) {
    std::string Default;
    benchmark::DoNotOptimize(Default);
  }
}
BENCHMARK(BM_StringCtorDefault);

enum class Length { Empty, Small, Large, Huge };
struct AllLengths : EnumValuesAsTuple<AllLengths, Length, 4> {
  static constexpr const char* Names[] = {"Empty", "Small", "Large", "Huge"};
};

enum class Opacity { Opaque, Transparent };
struct AllOpacity : EnumValuesAsTuple<AllOpacity, Opacity, 2> {
  static constexpr const char* Names[] = {"Opaque", "Transparent"};
};

enum class DiffType { Control, ChangeFirst, ChangeMiddle, ChangeLast };
struct AllDiffTypes : EnumValuesAsTuple<AllDiffTypes, DiffType, 4> {
  static constexpr const char* Names[] = {"Control", "ChangeFirst",
                                          "ChangeMiddle", "ChangeLast"};
};

TEST_ALWAYS_INLINE const char* getSmallString(DiffType D) {
  switch (D) {
    case DiffType::Control:
      return "0123456";
    case DiffType::ChangeFirst:
      return "-123456";
    case DiffType::ChangeMiddle:
      return "012-456";
    case DiffType::ChangeLast:
      return "012345-";
  }
}

TEST_ALWAYS_INLINE const char* getLargeString(DiffType D) {
#define LARGE_STRING_FIRST "123456789012345678901234567890"
#define LARGE_STRING_SECOND "234567890123456789012345678901"
  switch (D) {
    case DiffType::Control:
      return "0" LARGE_STRING_FIRST "1" LARGE_STRING_SECOND "2";
    case DiffType::ChangeFirst:
      return "-" LARGE_STRING_FIRST "1" LARGE_STRING_SECOND "2";
    case DiffType::ChangeMiddle:
      return "0" LARGE_STRING_FIRST "-" LARGE_STRING_SECOND "2";
    case DiffType::ChangeLast:
      return "0" LARGE_STRING_FIRST "1" LARGE_STRING_SECOND "-";
  }
}

TEST_ALWAYS_INLINE const char* getHugeString(DiffType D) {
#define HUGE_STRING0 "0123456789"
#define HUGE_STRING1 HUGE_STRING0 HUGE_STRING0 HUGE_STRING0 HUGE_STRING0
#define HUGE_STRING2 HUGE_STRING1 HUGE_STRING1 HUGE_STRING1 HUGE_STRING1
#define HUGE_STRING3 HUGE_STRING2 HUGE_STRING2 HUGE_STRING2 HUGE_STRING2
#define HUGE_STRING4 HUGE_STRING3 HUGE_STRING3 HUGE_STRING3 HUGE_STRING3
  switch (D) {
    case DiffType::Control:
      return "0123456789" HUGE_STRING4 "0123456789" HUGE_STRING4 "0123456789";
    case DiffType::ChangeFirst:
      return "-123456789" HUGE_STRING4 "0123456789" HUGE_STRING4 "0123456789";
    case DiffType::ChangeMiddle:
      return "0123456789" HUGE_STRING4 "01234-6789" HUGE_STRING4 "0123456789";
    case DiffType::ChangeLast:
      return "0123456789" HUGE_STRING4 "0123456789" HUGE_STRING4 "012345678-";
  }
}

TEST_ALWAYS_INLINE std::string makeString(Length L,
                                          DiffType D = DiffType::Control,
                                          Opacity O = Opacity::Transparent) {
  switch (L) {
  case Length::Empty:
    return maybeOpaque("", O == Opacity::Opaque);
  case Length::Small:
    return maybeOpaque(getSmallString(D), O == Opacity::Opaque);
  case Length::Large:
    return maybeOpaque(getLargeString(D), O == Opacity::Opaque);
  case Length::Huge:
    return maybeOpaque(getHugeString(D), O == Opacity::Opaque);
  }
}

template <class Length, class Opaque>
struct StringConstructDestroyCStr {
  static void run(benchmark::State& state) {
    for (auto _ : state) {
      benchmark::DoNotOptimize(
          makeString(Length(), DiffType::Control, Opaque()));
    }
  }

  static std::string name() {
    return "BM_StringConstructDestroyCStr" + Length::name() + Opaque::name();
  }
};

template <class Length, bool MeasureCopy, bool MeasureDestroy>
static void StringCopyAndDestroy(benchmark::State& state) {
  static constexpr size_t NumStrings = 1024;
  auto Orig = makeString(Length());
  std::aligned_storage<sizeof(std::string)>::type Storage[NumStrings];

  while (state.KeepRunningBatch(NumStrings)) {
    if (!MeasureCopy)
      state.PauseTiming();
    for (size_t I = 0; I < NumStrings; ++I) {
      ::new (static_cast<void*>(Storage + I)) std::string(Orig);
    }
    if (!MeasureCopy)
      state.ResumeTiming();
    if (!MeasureDestroy)
      state.PauseTiming();
    for (size_t I = 0; I < NumStrings; ++I) {
      using S = std::string;
      reinterpret_cast<S*>(Storage + I)->~S();
    }
    if (!MeasureDestroy)
      state.ResumeTiming();
  }
}

template <class Length>
struct StringCopy {
  static void run(benchmark::State& state) {
    StringCopyAndDestroy<Length, true, false>(state);
  }

  static std::string name() { return "BM_StringCopy" + Length::name(); }
};

template <class Length>
struct StringDestroy {
  static void run(benchmark::State& state) {
    StringCopyAndDestroy<Length, false, true>(state);
  }

  static std::string name() { return "BM_StringDestroy" + Length::name(); }
};

template <class Length>
struct StringMove {
  static void run(benchmark::State& state) {
    // Keep two object locations and move construct back and forth.
    std::aligned_storage<sizeof(std::string), alignof(std::string)>::type Storage[2];
    using S = std::string;
    size_t I = 0;
    S *newS = new (static_cast<void*>(Storage)) std::string(makeString(Length()));
    for (auto _ : state) {
      // Switch locations.
      I ^= 1;
      benchmark::DoNotOptimize(Storage);
      // Move construct into the new location,
      S *tmpS = new (static_cast<void*>(Storage + I)) S(std::move(*newS));
      // then destroy the old one.
      newS->~S();
      newS = tmpS;
    }
    newS->~S();
  }

  static std::string name() { return "BM_StringMove" + Length::name(); }
};

enum class Relation { Eq, Less, Compare };
struct AllRelations : EnumValuesAsTuple<AllRelations, Relation, 3> {
  static constexpr const char* Names[] = {"Eq", "Less", "Compare"};
};

template <class Rel, class LHLength, class RHLength, class DiffType>
struct StringRelational {
  static void run(benchmark::State& state) {
    auto Lhs = makeString(RHLength());
    auto Rhs = makeString(LHLength(), DiffType());
    for (auto _ : state) {
      benchmark::DoNotOptimize(Lhs);
      benchmark::DoNotOptimize(Rhs);
      switch (Rel()) {
      case Relation::Eq:
        benchmark::DoNotOptimize(Lhs == Rhs);
        break;
      case Relation::Less:
        benchmark::DoNotOptimize(Lhs < Rhs);
        break;
      case Relation::Compare:
        benchmark::DoNotOptimize(Lhs.compare(Rhs));
        break;
      }
    }
  }

  static bool skip() {
    // Eq is commutative, so skip half the matrix.
    if (Rel() == Relation::Eq && LHLength() > RHLength())
      return true;
    // We only care about control when the lengths differ.
    if (LHLength() != RHLength() && DiffType() != ::DiffType::Control)
      return true;
    // For empty, only control matters.
    if (LHLength() == Length::Empty && DiffType() != ::DiffType::Control)
      return true;
    return false;
  }

  static std::string name() {
    return "BM_StringRelational" + Rel::name() + LHLength::name() +
           RHLength::name() + DiffType::name();
  }
};

enum class Depth { Shallow, Deep };
struct AllDepths : EnumValuesAsTuple<AllDepths, Depth, 2> {
  static constexpr const char* Names[] = {"Shallow", "Deep"};
};

enum class Temperature { Hot, Cold };
struct AllTemperatures : EnumValuesAsTuple<AllTemperatures, Temperature, 2> {
  static constexpr const char* Names[] = {"Hot", "Cold"};
};

template <class Temperature, class Depth, class Length>
struct StringRead {
  void run(benchmark::State& state) const {
    static constexpr size_t NumStrings =
        Temperature() == ::Temperature::Hot
            ? 1 << 10
            : /* Enough strings to overflow the cache */ 1 << 20;
    static_assert((NumStrings & (NumStrings - 1)) == 0,
                  "NumStrings should be a power of two to reduce overhead.");

    std::vector<std::string> Values(NumStrings, makeString(Length()));
    size_t I = 0;
    for (auto _ : state) {
      // Jump long enough to defeat cache locality, and use a value that is
      // coprime with NumStrings to ensure we visit every element.
      I = (I + 17) % NumStrings;
      const auto& V = Values[I];

      // Read everything first. Escaping data() through DoNotOptimize might
      // cause the compiler to have to recalculate information about `V` due to
      // aliasing.
      const char* const Data = V.data();
      const size_t Size = V.size();
      benchmark::DoNotOptimize(Data);
      benchmark::DoNotOptimize(Size);
      if (Depth() == ::Depth::Deep) {
        // Read into the payload. This mainly shows the benefit of SSO when the
        // data is cold.
        benchmark::DoNotOptimize(*Data);
      }
    }
  }

  static bool skip() {
    // Huge does not give us anything that Large doesn't have. Skip it.
    if (Length() == ::Length::Huge) {
      return true;
    }
    return false;
  }

  std::string name() const {
    return "BM_StringRead" + Temperature::name() + Depth::name() +
           Length::name();
  }
};

void sanityCheckGeneratedStrings() {
  for (auto Lhs : {Length::Empty, Length::Small, Length::Large, Length::Huge}) {
    const auto LhsString = makeString(Lhs);
    for (auto Rhs :
         {Length::Empty, Length::Small, Length::Large, Length::Huge}) {
      if (Lhs > Rhs)
        continue;
      const auto RhsString = makeString(Rhs);

      // The smaller one must be a prefix of the larger one.
      if (RhsString.find(LhsString) != 0) {
        fprintf(stderr, "Invalid autogenerated strings for sizes (%d,%d).\n",
                static_cast<int>(Lhs), static_cast<int>(Rhs));
        std::abort();
      }
    }
  }
  // Verify the autogenerated diffs
  for (auto L : {Length::Small, Length::Large, Length::Huge}) {
    const auto Control = makeString(L);
    const auto Verify = [&](std::string Exp, size_t Pos) {
      // Only change on the Pos char.
      if (Control[Pos] != Exp[Pos]) {
        Exp[Pos] = Control[Pos];
        if (Control == Exp)
          return;
      }
      fprintf(stderr, "Invalid autogenerated diff with size %d\n",
              static_cast<int>(L));
      std::abort();
    };
    Verify(makeString(L, DiffType::ChangeFirst), 0);
    Verify(makeString(L, DiffType::ChangeMiddle), Control.size() / 2);
    Verify(makeString(L, DiffType::ChangeLast), Control.size() - 1);
  }
}

int main(int argc, char** argv) {
  benchmark::Initialize(&argc, argv);
  if (benchmark::ReportUnrecognizedArguments(argc, argv))
    return 1;

  sanityCheckGeneratedStrings();

  makeCartesianProductBenchmark<StringConstructDestroyCStr, AllLengths,
                                AllOpacity>();
  makeCartesianProductBenchmark<StringCopy, AllLengths>();
  makeCartesianProductBenchmark<StringMove, AllLengths>();
  makeCartesianProductBenchmark<StringDestroy, AllLengths>();
  makeCartesianProductBenchmark<StringRelational, AllRelations, AllLengths,
                                AllLengths, AllDiffTypes>();
  makeCartesianProductBenchmark<StringRead, AllTemperatures, AllDepths,
                                AllLengths>();
  benchmark::RunSpecifiedBenchmarks();
}
