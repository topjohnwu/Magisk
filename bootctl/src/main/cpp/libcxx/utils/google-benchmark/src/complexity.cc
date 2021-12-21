// Copyright 2016 Ismael Jimenez Martinez. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Source project : https://github.com/ismaelJimenez/cpp.leastsq
// Adapted to be used with google benchmark

#include "benchmark/benchmark.h"

#include <algorithm>
#include <cmath>
#include "check.h"
#include "complexity.h"

namespace benchmark {

// Internal function to calculate the different scalability forms
BigOFunc* FittingCurve(BigO complexity) {
  static const double kLog2E = 1.44269504088896340736;
  switch (complexity) {
    case oN:
      return [](int64_t n) -> double { return static_cast<double>(n); };
    case oNSquared:
      return [](int64_t n) -> double { return std::pow(n, 2); };
    case oNCubed:
      return [](int64_t n) -> double { return std::pow(n, 3); };
    case oLogN:
      /* Note: can't use log2 because Android's GNU STL lacks it */
      return [](int64_t n) { return kLog2E * log(static_cast<double>(n)); };
    case oNLogN:
      /* Note: can't use log2 because Android's GNU STL lacks it */
      return [](int64_t n) { return kLog2E * n * log(static_cast<double>(n)); };
    case o1:
    default:
      return [](int64_t) { return 1.0; };
  }
}

// Function to return an string for the calculated complexity
std::string GetBigOString(BigO complexity) {
  switch (complexity) {
    case oN:
      return "N";
    case oNSquared:
      return "N^2";
    case oNCubed:
      return "N^3";
    case oLogN:
      return "lgN";
    case oNLogN:
      return "NlgN";
    case o1:
      return "(1)";
    default:
      return "f(N)";
  }
}

// Find the coefficient for the high-order term in the running time, by
// minimizing the sum of squares of relative error, for the fitting curve
// given by the lambda expression.
//   - n             : Vector containing the size of the benchmark tests.
//   - time          : Vector containing the times for the benchmark tests.
//   - fitting_curve : lambda expression (e.g. [](int64_t n) {return n; };).

// For a deeper explanation on the algorithm logic, please refer to
// https://en.wikipedia.org/wiki/Least_squares#Least_squares,_regression_analysis_and_statistics

LeastSq MinimalLeastSq(const std::vector<int64_t>& n,
                       const std::vector<double>& time,
                       BigOFunc* fitting_curve) {
  double sigma_gn = 0.0;
  double sigma_gn_squared = 0.0;
  double sigma_time = 0.0;
  double sigma_time_gn = 0.0;

  // Calculate least square fitting parameter
  for (size_t i = 0; i < n.size(); ++i) {
    double gn_i = fitting_curve(n[i]);
    sigma_gn += gn_i;
    sigma_gn_squared += gn_i * gn_i;
    sigma_time += time[i];
    sigma_time_gn += time[i] * gn_i;
  }

  LeastSq result;
  result.complexity = oLambda;

  // Calculate complexity.
  result.coef = sigma_time_gn / sigma_gn_squared;

  // Calculate RMS
  double rms = 0.0;
  for (size_t i = 0; i < n.size(); ++i) {
    double fit = result.coef * fitting_curve(n[i]);
    rms += pow((time[i] - fit), 2);
  }

  // Normalized RMS by the mean of the observed values
  double mean = sigma_time / n.size();
  result.rms = sqrt(rms / n.size()) / mean;

  return result;
}

// Find the coefficient for the high-order term in the running time, by
// minimizing the sum of squares of relative error.
//   - n          : Vector containing the size of the benchmark tests.
//   - time       : Vector containing the times for the benchmark tests.
//   - complexity : If different than oAuto, the fitting curve will stick to
//                  this one. If it is oAuto, it will be calculated the best
//                  fitting curve.
LeastSq MinimalLeastSq(const std::vector<int64_t>& n,
                       const std::vector<double>& time, const BigO complexity) {
  CHECK_EQ(n.size(), time.size());
  CHECK_GE(n.size(), 2);  // Do not compute fitting curve is less than two
                          // benchmark runs are given
  CHECK_NE(complexity, oNone);

  LeastSq best_fit;

  if (complexity == oAuto) {
    std::vector<BigO> fit_curves = {oLogN, oN, oNLogN, oNSquared, oNCubed};

    // Take o1 as default best fitting curve
    best_fit = MinimalLeastSq(n, time, FittingCurve(o1));
    best_fit.complexity = o1;

    // Compute all possible fitting curves and stick to the best one
    for (const auto& fit : fit_curves) {
      LeastSq current_fit = MinimalLeastSq(n, time, FittingCurve(fit));
      if (current_fit.rms < best_fit.rms) {
        best_fit = current_fit;
        best_fit.complexity = fit;
      }
    }
  } else {
    best_fit = MinimalLeastSq(n, time, FittingCurve(complexity));
    best_fit.complexity = complexity;
  }

  return best_fit;
}

std::vector<BenchmarkReporter::Run> ComputeBigO(
    const std::vector<BenchmarkReporter::Run>& reports) {
  typedef BenchmarkReporter::Run Run;
  std::vector<Run> results;

  if (reports.size() < 2) return results;

  // Accumulators.
  std::vector<int64_t> n;
  std::vector<double> real_time;
  std::vector<double> cpu_time;

  // Populate the accumulators.
  for (const Run& run : reports) {
    CHECK_GT(run.complexity_n, 0) << "Did you forget to call SetComplexityN?";
    n.push_back(run.complexity_n);
    real_time.push_back(run.real_accumulated_time / run.iterations);
    cpu_time.push_back(run.cpu_accumulated_time / run.iterations);
  }

  LeastSq result_cpu;
  LeastSq result_real;

  if (reports[0].complexity == oLambda) {
    result_cpu = MinimalLeastSq(n, cpu_time, reports[0].complexity_lambda);
    result_real = MinimalLeastSq(n, real_time, reports[0].complexity_lambda);
  } else {
    result_cpu = MinimalLeastSq(n, cpu_time, reports[0].complexity);
    result_real = MinimalLeastSq(n, real_time, result_cpu.complexity);
  }

  std::string run_name = reports[0].benchmark_name().substr(
      0, reports[0].benchmark_name().find('/'));

  // Get the data from the accumulator to BenchmarkReporter::Run's.
  Run big_o;
  big_o.run_name = run_name;
  big_o.run_type = BenchmarkReporter::Run::RT_Aggregate;
  big_o.aggregate_name = "BigO";
  big_o.iterations = 0;
  big_o.real_accumulated_time = result_real.coef;
  big_o.cpu_accumulated_time = result_cpu.coef;
  big_o.report_big_o = true;
  big_o.complexity = result_cpu.complexity;

  // All the time results are reported after being multiplied by the
  // time unit multiplier. But since RMS is a relative quantity it
  // should not be multiplied at all. So, here, we _divide_ it by the
  // multiplier so that when it is multiplied later the result is the
  // correct one.
  double multiplier = GetTimeUnitMultiplier(reports[0].time_unit);

  // Only add label to mean/stddev if it is same for all runs
  Run rms;
  rms.run_name = run_name;
  big_o.report_label = reports[0].report_label;
  rms.run_type = BenchmarkReporter::Run::RT_Aggregate;
  rms.aggregate_name = "RMS";
  rms.report_label = big_o.report_label;
  rms.iterations = 0;
  rms.real_accumulated_time = result_real.rms / multiplier;
  rms.cpu_accumulated_time = result_cpu.rms / multiplier;
  rms.report_rms = true;
  rms.complexity = result_cpu.complexity;
  // don't forget to keep the time unit, or we won't be able to
  // recover the correct value.
  rms.time_unit = reports[0].time_unit;

  results.push_back(big_o);
  results.push_back(rms);
  return results;
}

}  // end namespace benchmark
