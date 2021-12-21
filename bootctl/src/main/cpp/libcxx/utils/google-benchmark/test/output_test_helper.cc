#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <random>
#include <sstream>
#include <streambuf>

#include "../src/benchmark_api_internal.h"
#include "../src/check.h"  // NOTE: check.h is for internal use only!
#include "../src/re.h"     // NOTE: re.h is for internal use only
#include "output_test.h"

// ========================================================================= //
// ------------------------------ Internals -------------------------------- //
// ========================================================================= //
namespace internal {
namespace {

using TestCaseList = std::vector<TestCase>;

// Use a vector because the order elements are added matters during iteration.
// std::map/unordered_map don't guarantee that.
// For example:
//  SetSubstitutions({{"%HelloWorld", "Hello"}, {"%Hello", "Hi"}});
//     Substitute("%HelloWorld") // Always expands to Hello.
using SubMap = std::vector<std::pair<std::string, std::string>>;

TestCaseList& GetTestCaseList(TestCaseID ID) {
  // Uses function-local statics to ensure initialization occurs
  // before first use.
  static TestCaseList lists[TC_NumID];
  return lists[ID];
}

SubMap& GetSubstitutions() {
  // Don't use 'dec_re' from header because it may not yet be initialized.
  // clang-format off
  static std::string safe_dec_re = "[0-9]*[.]?[0-9]+([eE][-+][0-9]+)?";
  static std::string time_re = "([0-9]+[.])?[0-9]+";
  static SubMap map = {
      {"%float", "[0-9]*[.]?[0-9]+([eE][-+][0-9]+)?"},
      // human-readable float
      {"%hrfloat", "[0-9]*[.]?[0-9]+([eE][-+][0-9]+)?[kMGTPEZYmunpfazy]?"},
      {"%int", "[ ]*[0-9]+"},
      {" %s ", "[ ]+"},
      {"%time", "[ ]*" + time_re + "[ ]+ns"},
      {"%console_report", "[ ]*" + time_re + "[ ]+ns [ ]*" + time_re + "[ ]+ns [ ]*[0-9]+"},
      {"%console_time_only_report", "[ ]*" + time_re + "[ ]+ns [ ]*" + time_re + "[ ]+ns"},
      {"%console_us_report", "[ ]*" + time_re + "[ ]+us [ ]*" + time_re + "[ ]+us [ ]*[0-9]+"},
      {"%console_us_time_only_report", "[ ]*" + time_re + "[ ]+us [ ]*" + time_re + "[ ]+us"},
      {"%csv_header",
       "name,iterations,real_time,cpu_time,time_unit,bytes_per_second,"
       "items_per_second,label,error_occurred,error_message"},
      {"%csv_report", "[0-9]+," + safe_dec_re + "," + safe_dec_re + ",ns,,,,,"},
      {"%csv_us_report", "[0-9]+," + safe_dec_re + "," + safe_dec_re + ",us,,,,,"},
      {"%csv_bytes_report",
       "[0-9]+," + safe_dec_re + "," + safe_dec_re + ",ns," + safe_dec_re + ",,,,"},
      {"%csv_items_report",
       "[0-9]+," + safe_dec_re + "," + safe_dec_re + ",ns,," + safe_dec_re + ",,,"},
      {"%csv_bytes_items_report",
       "[0-9]+," + safe_dec_re + "," + safe_dec_re + ",ns," + safe_dec_re +
       "," + safe_dec_re + ",,,"},
      {"%csv_label_report_begin", "[0-9]+," + safe_dec_re + "," + safe_dec_re + ",ns,,,"},
      {"%csv_label_report_end", ",,"}};
  // clang-format on
  return map;
}

std::string PerformSubstitutions(std::string source) {
  SubMap const& subs = GetSubstitutions();
  using SizeT = std::string::size_type;
  for (auto const& KV : subs) {
    SizeT pos;
    SizeT next_start = 0;
    while ((pos = source.find(KV.first, next_start)) != std::string::npos) {
      next_start = pos + KV.second.size();
      source.replace(pos, KV.first.size(), KV.second);
    }
  }
  return source;
}

void CheckCase(std::stringstream& remaining_output, TestCase const& TC,
               TestCaseList const& not_checks) {
  std::string first_line;
  bool on_first = true;
  std::string line;
  while (remaining_output.eof() == false) {
    CHECK(remaining_output.good());
    std::getline(remaining_output, line);
    if (on_first) {
      first_line = line;
      on_first = false;
    }
    for (const auto& NC : not_checks) {
      CHECK(!NC.regex->Match(line))
          << "Unexpected match for line \"" << line << "\" for MR_Not regex \""
          << NC.regex_str << "\""
          << "\n    actual regex string \"" << TC.substituted_regex << "\""
          << "\n    started matching near: " << first_line;
    }
    if (TC.regex->Match(line)) return;
    CHECK(TC.match_rule != MR_Next)
        << "Expected line \"" << line << "\" to match regex \"" << TC.regex_str
        << "\""
        << "\n    actual regex string \"" << TC.substituted_regex << "\""
        << "\n    started matching near: " << first_line;
  }
  CHECK(remaining_output.eof() == false)
      << "End of output reached before match for regex \"" << TC.regex_str
      << "\" was found"
      << "\n    actual regex string \"" << TC.substituted_regex << "\""
      << "\n    started matching near: " << first_line;
}

void CheckCases(TestCaseList const& checks, std::stringstream& output) {
  std::vector<TestCase> not_checks;
  for (size_t i = 0; i < checks.size(); ++i) {
    const auto& TC = checks[i];
    if (TC.match_rule == MR_Not) {
      not_checks.push_back(TC);
      continue;
    }
    CheckCase(output, TC, not_checks);
    not_checks.clear();
  }
}

class TestReporter : public benchmark::BenchmarkReporter {
 public:
  TestReporter(std::vector<benchmark::BenchmarkReporter*> reps)
      : reporters_(reps) {}

  virtual bool ReportContext(const Context& context) {
    bool last_ret = false;
    bool first = true;
    for (auto rep : reporters_) {
      bool new_ret = rep->ReportContext(context);
      CHECK(first || new_ret == last_ret)
          << "Reports return different values for ReportContext";
      first = false;
      last_ret = new_ret;
    }
    (void)first;
    return last_ret;
  }

  void ReportRuns(const std::vector<Run>& report) {
    for (auto rep : reporters_) rep->ReportRuns(report);
  }
  void Finalize() {
    for (auto rep : reporters_) rep->Finalize();
  }

 private:
  std::vector<benchmark::BenchmarkReporter*> reporters_;
};
}  // namespace

}  // end namespace internal

// ========================================================================= //
// -------------------------- Results checking ----------------------------- //
// ========================================================================= //

namespace internal {

// Utility class to manage subscribers for checking benchmark results.
// It works by parsing the CSV output to read the results.
class ResultsChecker {
 public:
  struct PatternAndFn : public TestCase {  // reusing TestCase for its regexes
    PatternAndFn(const std::string& rx, ResultsCheckFn fn_)
        : TestCase(rx), fn(fn_) {}
    ResultsCheckFn fn;
  };

  std::vector<PatternAndFn> check_patterns;
  std::vector<Results> results;
  std::vector<std::string> field_names;

  void Add(const std::string& entry_pattern, ResultsCheckFn fn);

  void CheckResults(std::stringstream& output);

 private:
  void SetHeader_(const std::string& csv_header);
  void SetValues_(const std::string& entry_csv_line);

  std::vector<std::string> SplitCsv_(const std::string& line);
};

// store the static ResultsChecker in a function to prevent initialization
// order problems
ResultsChecker& GetResultsChecker() {
  static ResultsChecker rc;
  return rc;
}

// add a results checker for a benchmark
void ResultsChecker::Add(const std::string& entry_pattern, ResultsCheckFn fn) {
  check_patterns.emplace_back(entry_pattern, fn);
}

// check the results of all subscribed benchmarks
void ResultsChecker::CheckResults(std::stringstream& output) {
  // first reset the stream to the start
  {
    auto start = std::stringstream::pos_type(0);
    // clear before calling tellg()
    output.clear();
    // seek to zero only when needed
    if (output.tellg() > start) output.seekg(start);
    // and just in case
    output.clear();
  }
  // now go over every line and publish it to the ResultsChecker
  std::string line;
  bool on_first = true;
  while (output.eof() == false) {
    CHECK(output.good());
    std::getline(output, line);
    if (on_first) {
      SetHeader_(line);  // this is important
      on_first = false;
      continue;
    }
    SetValues_(line);
  }
  // finally we can call the subscribed check functions
  for (const auto& p : check_patterns) {
    VLOG(2) << "--------------------------------\n";
    VLOG(2) << "checking for benchmarks matching " << p.regex_str << "...\n";
    for (const auto& r : results) {
      if (!p.regex->Match(r.name)) {
        VLOG(2) << p.regex_str << " is not matched by " << r.name << "\n";
        continue;
      } else {
        VLOG(2) << p.regex_str << " is matched by " << r.name << "\n";
      }
      VLOG(1) << "Checking results of " << r.name << ": ... \n";
      p.fn(r);
      VLOG(1) << "Checking results of " << r.name << ": OK.\n";
    }
  }
}

// prepare for the names in this header
void ResultsChecker::SetHeader_(const std::string& csv_header) {
  field_names = SplitCsv_(csv_header);
}

// set the values for a benchmark
void ResultsChecker::SetValues_(const std::string& entry_csv_line) {
  if (entry_csv_line.empty()) return;  // some lines are empty
  CHECK(!field_names.empty());
  auto vals = SplitCsv_(entry_csv_line);
  CHECK_EQ(vals.size(), field_names.size());
  results.emplace_back(vals[0]);  // vals[0] is the benchmark name
  auto& entry = results.back();
  for (size_t i = 1, e = vals.size(); i < e; ++i) {
    entry.values[field_names[i]] = vals[i];
  }
}

// a quick'n'dirty csv splitter (eliminating quotes)
std::vector<std::string> ResultsChecker::SplitCsv_(const std::string& line) {
  std::vector<std::string> out;
  if (line.empty()) return out;
  if (!field_names.empty()) out.reserve(field_names.size());
  size_t prev = 0, pos = line.find_first_of(','), curr = pos;
  while (pos != line.npos) {
    CHECK(curr > 0);
    if (line[prev] == '"') ++prev;
    if (line[curr - 1] == '"') --curr;
    out.push_back(line.substr(prev, curr - prev));
    prev = pos + 1;
    pos = line.find_first_of(',', pos + 1);
    curr = pos;
  }
  curr = line.size();
  if (line[prev] == '"') ++prev;
  if (line[curr - 1] == '"') --curr;
  out.push_back(line.substr(prev, curr - prev));
  return out;
}

}  // end namespace internal

size_t AddChecker(const char* bm_name, ResultsCheckFn fn) {
  auto& rc = internal::GetResultsChecker();
  rc.Add(bm_name, fn);
  return rc.results.size();
}

int Results::NumThreads() const {
  auto pos = name.find("/threads:");
  if (pos == name.npos) return 1;
  auto end = name.find('/', pos + 9);
  std::stringstream ss;
  ss << name.substr(pos + 9, end);
  int num = 1;
  ss >> num;
  CHECK(!ss.fail());
  return num;
}

double Results::NumIterations() const {
  return GetAs<double>("iterations");
}

double Results::GetTime(BenchmarkTime which) const {
  CHECK(which == kCpuTime || which == kRealTime);
  const char* which_str = which == kCpuTime ? "cpu_time" : "real_time";
  double val = GetAs<double>(which_str);
  auto unit = Get("time_unit");
  CHECK(unit);
  if (*unit == "ns") {
    return val * 1.e-9;
  } else if (*unit == "us") {
    return val * 1.e-6;
  } else if (*unit == "ms") {
    return val * 1.e-3;
  } else if (*unit == "s") {
    return val;
  } else {
    CHECK(1 == 0) << "unknown time unit: " << *unit;
    return 0;
  }
}

// ========================================================================= //
// -------------------------- Public API Definitions------------------------ //
// ========================================================================= //

TestCase::TestCase(std::string re, int rule)
    : regex_str(std::move(re)),
      match_rule(rule),
      substituted_regex(internal::PerformSubstitutions(regex_str)),
      regex(std::make_shared<benchmark::Regex>()) {
  std::string err_str;
  regex->Init(substituted_regex, &err_str);
  CHECK(err_str.empty()) << "Could not construct regex \"" << substituted_regex
                         << "\""
                         << "\n    originally \"" << regex_str << "\""
                         << "\n    got error: " << err_str;
}

int AddCases(TestCaseID ID, std::initializer_list<TestCase> il) {
  auto& L = internal::GetTestCaseList(ID);
  L.insert(L.end(), il);
  return 0;
}

int SetSubstitutions(
    std::initializer_list<std::pair<std::string, std::string>> il) {
  auto& subs = internal::GetSubstitutions();
  for (auto KV : il) {
    bool exists = false;
    KV.second = internal::PerformSubstitutions(KV.second);
    for (auto& EKV : subs) {
      if (EKV.first == KV.first) {
        EKV.second = std::move(KV.second);
        exists = true;
        break;
      }
    }
    if (!exists) subs.push_back(std::move(KV));
  }
  return 0;
}

void RunOutputTests(int argc, char* argv[]) {
  using internal::GetTestCaseList;
  benchmark::Initialize(&argc, argv);
  auto options = benchmark::internal::GetOutputOptions(/*force_no_color*/ true);
  benchmark::ConsoleReporter CR(options);
  benchmark::JSONReporter JR;
  benchmark::CSVReporter CSVR;
  struct ReporterTest {
    const char* name;
    std::vector<TestCase>& output_cases;
    std::vector<TestCase>& error_cases;
    benchmark::BenchmarkReporter& reporter;
    std::stringstream out_stream;
    std::stringstream err_stream;

    ReporterTest(const char* n, std::vector<TestCase>& out_tc,
                 std::vector<TestCase>& err_tc,
                 benchmark::BenchmarkReporter& br)
        : name(n), output_cases(out_tc), error_cases(err_tc), reporter(br) {
      reporter.SetOutputStream(&out_stream);
      reporter.SetErrorStream(&err_stream);
    }
  } TestCases[] = {
      {"ConsoleReporter", GetTestCaseList(TC_ConsoleOut),
       GetTestCaseList(TC_ConsoleErr), CR},
      {"JSONReporter", GetTestCaseList(TC_JSONOut), GetTestCaseList(TC_JSONErr),
       JR},
      {"CSVReporter", GetTestCaseList(TC_CSVOut), GetTestCaseList(TC_CSVErr),
       CSVR},
  };

  // Create the test reporter and run the benchmarks.
  std::cout << "Running benchmarks...\n";
  internal::TestReporter test_rep({&CR, &JR, &CSVR});
  benchmark::RunSpecifiedBenchmarks(&test_rep);

  for (auto& rep_test : TestCases) {
    std::string msg = std::string("\nTesting ") + rep_test.name + " Output\n";
    std::string banner(msg.size() - 1, '-');
    std::cout << banner << msg << banner << "\n";

    std::cerr << rep_test.err_stream.str();
    std::cout << rep_test.out_stream.str();

    internal::CheckCases(rep_test.error_cases, rep_test.err_stream);
    internal::CheckCases(rep_test.output_cases, rep_test.out_stream);

    std::cout << "\n";
  }

  // now that we know the output is as expected, we can dispatch
  // the checks to subscribees.
  auto& csv = TestCases[2];
  // would use == but gcc spits a warning
  CHECK(std::strcmp(csv.name, "CSVReporter") == 0);
  internal::GetResultsChecker().CheckResults(csv.out_stream);
}

int SubstrCnt(const std::string& haystack, const std::string& pat) {
  if (pat.length() == 0) return 0;
  int count = 0;
  for (size_t offset = haystack.find(pat); offset != std::string::npos;
       offset = haystack.find(pat, offset + pat.length()))
    ++count;
  return count;
}

static char ToHex(int ch) {
  return ch < 10 ? static_cast<char>('0' + ch)
                 : static_cast<char>('a' + (ch - 10));
}

static char RandomHexChar() {
  static std::mt19937 rd{std::random_device{}()};
  static std::uniform_int_distribution<int> mrand{0, 15};
  return ToHex(mrand(rd));
}

static std::string GetRandomFileName() {
  std::string model = "test.%%%%%%";
  for (auto & ch :  model) {
    if (ch == '%')
      ch = RandomHexChar();
  }
  return model;
}

static bool FileExists(std::string const& name) {
  std::ifstream in(name.c_str());
  return in.good();
}

static std::string GetTempFileName() {
  // This function attempts to avoid race conditions where two tests
  // create the same file at the same time. However, it still introduces races
  // similar to tmpnam.
  int retries = 3;
  while (--retries) {
    std::string name = GetRandomFileName();
    if (!FileExists(name))
      return name;
  }
  std::cerr << "Failed to create unique temporary file name" << std::endl;
  std::abort();
}

std::string GetFileReporterOutput(int argc, char* argv[]) {
  std::vector<char*> new_argv(argv, argv + argc);
  assert(static_cast<decltype(new_argv)::size_type>(argc) == new_argv.size());

  std::string tmp_file_name = GetTempFileName();
  std::cout << "Will be using this as the tmp file: " << tmp_file_name << '\n';

  std::string tmp = "--benchmark_out=";
  tmp += tmp_file_name;
  new_argv.emplace_back(const_cast<char*>(tmp.c_str()));

  argc = int(new_argv.size());

  benchmark::Initialize(&argc, new_argv.data());
  benchmark::RunSpecifiedBenchmarks();

  // Read the output back from the file, and delete the file.
  std::ifstream tmp_stream(tmp_file_name);
  std::string output = std::string((std::istreambuf_iterator<char>(tmp_stream)),
                                   std::istreambuf_iterator<char>());
  std::remove(tmp_file_name.c_str());

  return output;
}
