// Copyright 2015 Google Inc. All rights reserved.
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

#include "internal_macros.h"

#ifdef BENCHMARK_OS_WINDOWS
#include <shlwapi.h>
#undef StrCat  // Don't let StrCat in string_util.h be renamed to lstrcatA
#include <versionhelpers.h>
#include <windows.h>
#include <codecvt>
#else
#include <fcntl.h>
#ifndef BENCHMARK_OS_FUCHSIA
#include <sys/resource.h>
#endif
#include <sys/time.h>
#include <sys/types.h>  // this header must be included before 'sys/sysctl.h' to avoid compilation error on FreeBSD
#include <unistd.h>
#if defined BENCHMARK_OS_FREEBSD || defined BENCHMARK_OS_MACOSX || \
    defined BENCHMARK_OS_NETBSD || defined BENCHMARK_OS_OPENBSD
#define BENCHMARK_HAS_SYSCTL
#include <sys/sysctl.h>
#endif
#endif
#if defined(BENCHMARK_OS_SOLARIS)
#include <kstat.h>
#endif

#include <algorithm>
#include <array>
#include <bitset>
#include <cerrno>
#include <climits>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <memory>
#include <sstream>
#include <locale>

#include "check.h"
#include "cycleclock.h"
#include "internal_macros.h"
#include "log.h"
#include "sleep.h"
#include "string_util.h"

namespace benchmark {
namespace {

void PrintImp(std::ostream& out) { out << std::endl; }

template <class First, class... Rest>
void PrintImp(std::ostream& out, First&& f, Rest&&... rest) {
  out << std::forward<First>(f);
  PrintImp(out, std::forward<Rest>(rest)...);
}

template <class... Args>
BENCHMARK_NORETURN void PrintErrorAndDie(Args&&... args) {
  PrintImp(std::cerr, std::forward<Args>(args)...);
  std::exit(EXIT_FAILURE);
}

#ifdef BENCHMARK_HAS_SYSCTL

/// ValueUnion - A type used to correctly alias the byte-for-byte output of
/// `sysctl` with the result type it's to be interpreted as.
struct ValueUnion {
  union DataT {
    uint32_t uint32_value;
    uint64_t uint64_value;
    // For correct aliasing of union members from bytes.
    char bytes[8];
  };
  using DataPtr = std::unique_ptr<DataT, decltype(&std::free)>;

  // The size of the data union member + its trailing array size.
  size_t Size;
  DataPtr Buff;

 public:
  ValueUnion() : Size(0), Buff(nullptr, &std::free) {}

  explicit ValueUnion(size_t BuffSize)
      : Size(sizeof(DataT) + BuffSize),
        Buff(::new (std::malloc(Size)) DataT(), &std::free) {}

  ValueUnion(ValueUnion&& other) = default;

  explicit operator bool() const { return bool(Buff); }

  char* data() const { return Buff->bytes; }

  std::string GetAsString() const { return std::string(data()); }

  int64_t GetAsInteger() const {
    if (Size == sizeof(Buff->uint32_value))
      return static_cast<int32_t>(Buff->uint32_value);
    else if (Size == sizeof(Buff->uint64_value))
      return static_cast<int64_t>(Buff->uint64_value);
    BENCHMARK_UNREACHABLE();
  }

  uint64_t GetAsUnsigned() const {
    if (Size == sizeof(Buff->uint32_value))
      return Buff->uint32_value;
    else if (Size == sizeof(Buff->uint64_value))
      return Buff->uint64_value;
    BENCHMARK_UNREACHABLE();
  }

  template <class T, int N>
  std::array<T, N> GetAsArray() {
    const int ArrSize = sizeof(T) * N;
    CHECK_LE(ArrSize, Size);
    std::array<T, N> Arr;
    std::memcpy(Arr.data(), data(), ArrSize);
    return Arr;
  }
};

ValueUnion GetSysctlImp(std::string const& Name) {
#if defined BENCHMARK_OS_OPENBSD
  int mib[2];

  mib[0] = CTL_HW;
  if ((Name == "hw.ncpu") || (Name == "hw.cpuspeed")){
    ValueUnion buff(sizeof(int));

    if (Name == "hw.ncpu") {
      mib[1] = HW_NCPU;
    } else {
      mib[1] = HW_CPUSPEED;
    }

    if (sysctl(mib, 2, buff.data(), &buff.Size, nullptr, 0) == -1) {
      return ValueUnion();
    }
    return buff;
  }
  return ValueUnion();
#else
  size_t CurBuffSize = 0;
  if (sysctlbyname(Name.c_str(), nullptr, &CurBuffSize, nullptr, 0) == -1)
    return ValueUnion();

  ValueUnion buff(CurBuffSize);
  if (sysctlbyname(Name.c_str(), buff.data(), &buff.Size, nullptr, 0) == 0)
    return buff;
  return ValueUnion();
#endif
}

BENCHMARK_MAYBE_UNUSED
bool GetSysctl(std::string const& Name, std::string* Out) {
  Out->clear();
  auto Buff = GetSysctlImp(Name);
  if (!Buff) return false;
  Out->assign(Buff.data());
  return true;
}

template <class Tp,
          class = typename std::enable_if<std::is_integral<Tp>::value>::type>
bool GetSysctl(std::string const& Name, Tp* Out) {
  *Out = 0;
  auto Buff = GetSysctlImp(Name);
  if (!Buff) return false;
  *Out = static_cast<Tp>(Buff.GetAsUnsigned());
  return true;
}

template <class Tp, size_t N>
bool GetSysctl(std::string const& Name, std::array<Tp, N>* Out) {
  auto Buff = GetSysctlImp(Name);
  if (!Buff) return false;
  *Out = Buff.GetAsArray<Tp, N>();
  return true;
}
#endif

template <class ArgT>
bool ReadFromFile(std::string const& fname, ArgT* arg) {
  *arg = ArgT();
  std::ifstream f(fname.c_str());
  if (!f.is_open()) return false;
  f >> *arg;
  return f.good();
}

bool CpuScalingEnabled(int num_cpus) {
  // We don't have a valid CPU count, so don't even bother.
  if (num_cpus <= 0) return false;
#ifndef BENCHMARK_OS_WINDOWS
  // On Linux, the CPUfreq subsystem exposes CPU information as files on the
  // local file system. If reading the exported files fails, then we may not be
  // running on Linux, so we silently ignore all the read errors.
  std::string res;
  for (int cpu = 0; cpu < num_cpus; ++cpu) {
    std::string governor_file =
        StrCat("/sys/devices/system/cpu/cpu", cpu, "/cpufreq/scaling_governor");
    if (ReadFromFile(governor_file, &res) && res != "performance") return true;
  }
#endif
  return false;
}

int CountSetBitsInCPUMap(std::string Val) {
  auto CountBits = [](std::string Part) {
    using CPUMask = std::bitset<sizeof(std::uintptr_t) * CHAR_BIT>;
    Part = "0x" + Part;
    CPUMask Mask(benchmark::stoul(Part, nullptr, 16));
    return static_cast<int>(Mask.count());
  };
  size_t Pos;
  int total = 0;
  while ((Pos = Val.find(',')) != std::string::npos) {
    total += CountBits(Val.substr(0, Pos));
    Val = Val.substr(Pos + 1);
  }
  if (!Val.empty()) {
    total += CountBits(Val);
  }
  return total;
}

BENCHMARK_MAYBE_UNUSED
std::vector<CPUInfo::CacheInfo> GetCacheSizesFromKVFS() {
  std::vector<CPUInfo::CacheInfo> res;
  std::string dir = "/sys/devices/system/cpu/cpu0/cache/";
  int Idx = 0;
  while (true) {
    CPUInfo::CacheInfo info;
    std::string FPath = StrCat(dir, "index", Idx++, "/");
    std::ifstream f(StrCat(FPath, "size").c_str());
    if (!f.is_open()) break;
    std::string suffix;
    f >> info.size;
    if (f.fail())
      PrintErrorAndDie("Failed while reading file '", FPath, "size'");
    if (f.good()) {
      f >> suffix;
      if (f.bad())
        PrintErrorAndDie(
            "Invalid cache size format: failed to read size suffix");
      else if (f && suffix != "K")
        PrintErrorAndDie("Invalid cache size format: Expected bytes ", suffix);
      else if (suffix == "K")
        info.size *= 1000;
    }
    if (!ReadFromFile(StrCat(FPath, "type"), &info.type))
      PrintErrorAndDie("Failed to read from file ", FPath, "type");
    if (!ReadFromFile(StrCat(FPath, "level"), &info.level))
      PrintErrorAndDie("Failed to read from file ", FPath, "level");
    std::string map_str;
    if (!ReadFromFile(StrCat(FPath, "shared_cpu_map"), &map_str))
      PrintErrorAndDie("Failed to read from file ", FPath, "shared_cpu_map");
    info.num_sharing = CountSetBitsInCPUMap(map_str);
    res.push_back(info);
  }

  return res;
}

#ifdef BENCHMARK_OS_MACOSX
std::vector<CPUInfo::CacheInfo> GetCacheSizesMacOSX() {
  std::vector<CPUInfo::CacheInfo> res;
  std::array<uint64_t, 4> CacheCounts{{0, 0, 0, 0}};
  GetSysctl("hw.cacheconfig", &CacheCounts);

  struct {
    std::string name;
    std::string type;
    int level;
    uint64_t num_sharing;
  } Cases[] = {{"hw.l1dcachesize", "Data", 1, CacheCounts[1]},
               {"hw.l1icachesize", "Instruction", 1, CacheCounts[1]},
               {"hw.l2cachesize", "Unified", 2, CacheCounts[2]},
               {"hw.l3cachesize", "Unified", 3, CacheCounts[3]}};
  for (auto& C : Cases) {
    int val;
    if (!GetSysctl(C.name, &val)) continue;
    CPUInfo::CacheInfo info;
    info.type = C.type;
    info.level = C.level;
    info.size = val;
    info.num_sharing = static_cast<int>(C.num_sharing);
    res.push_back(std::move(info));
  }
  return res;
}
#elif defined(BENCHMARK_OS_WINDOWS)
std::vector<CPUInfo::CacheInfo> GetCacheSizesWindows() {
  std::vector<CPUInfo::CacheInfo> res;
  DWORD buffer_size = 0;
  using PInfo = SYSTEM_LOGICAL_PROCESSOR_INFORMATION;
  using CInfo = CACHE_DESCRIPTOR;

  using UPtr = std::unique_ptr<PInfo, decltype(&std::free)>;
  GetLogicalProcessorInformation(nullptr, &buffer_size);
  UPtr buff((PInfo*)malloc(buffer_size), &std::free);
  if (!GetLogicalProcessorInformation(buff.get(), &buffer_size))
    PrintErrorAndDie("Failed during call to GetLogicalProcessorInformation: ",
                     GetLastError());

  PInfo* it = buff.get();
  PInfo* end = buff.get() + (buffer_size / sizeof(PInfo));

  for (; it != end; ++it) {
    if (it->Relationship != RelationCache) continue;
    using BitSet = std::bitset<sizeof(ULONG_PTR) * CHAR_BIT>;
    BitSet B(it->ProcessorMask);
    // To prevent duplicates, only consider caches where CPU 0 is specified
    if (!B.test(0)) continue;
    CInfo* Cache = &it->Cache;
    CPUInfo::CacheInfo C;
    C.num_sharing = static_cast<int>(B.count());
    C.level = Cache->Level;
    C.size = Cache->Size;
    switch (Cache->Type) {
      case CacheUnified:
        C.type = "Unified";
        break;
      case CacheInstruction:
        C.type = "Instruction";
        break;
      case CacheData:
        C.type = "Data";
        break;
      case CacheTrace:
        C.type = "Trace";
        break;
      default:
        C.type = "Unknown";
        break;
    }
    res.push_back(C);
  }
  return res;
}
#endif

std::vector<CPUInfo::CacheInfo> GetCacheSizes() {
#ifdef BENCHMARK_OS_MACOSX
  return GetCacheSizesMacOSX();
#elif defined(BENCHMARK_OS_WINDOWS)
  return GetCacheSizesWindows();
#else
  return GetCacheSizesFromKVFS();
#endif
}

std::string GetSystemName() {
#if defined(BENCHMARK_OS_WINDOWS)
  std::string str;
  const unsigned COUNT = MAX_COMPUTERNAME_LENGTH+1;
  TCHAR  hostname[COUNT] = {'\0'};
  DWORD DWCOUNT = COUNT;
  if (!GetComputerName(hostname, &DWCOUNT))
    return std::string("");
#ifndef UNICODE
  str = std::string(hostname, DWCOUNT);
#else
  //Using wstring_convert, Is deprecated in C++17
  using convert_type = std::codecvt_utf8<wchar_t>;
  std::wstring_convert<convert_type, wchar_t> converter;
  std::wstring wStr(hostname, DWCOUNT);
  str = converter.to_bytes(wStr);
#endif
  return str;
#else // defined(BENCHMARK_OS_WINDOWS)
#ifdef BENCHMARK_OS_MACOSX //Mac Doesnt have HOST_NAME_MAX defined
#define HOST_NAME_MAX 64
#endif
  char hostname[HOST_NAME_MAX];
  int retVal = gethostname(hostname, HOST_NAME_MAX);
  if (retVal != 0) return std::string("");
  return std::string(hostname);
#endif // Catch-all POSIX block.
}

int GetNumCPUs() {
#ifdef BENCHMARK_HAS_SYSCTL
  int NumCPU = -1;
  if (GetSysctl("hw.ncpu", &NumCPU)) return NumCPU;
  fprintf(stderr, "Err: %s\n", strerror(errno));
  std::exit(EXIT_FAILURE);
#elif defined(BENCHMARK_OS_WINDOWS)
  SYSTEM_INFO sysinfo;
  // Use memset as opposed to = {} to avoid GCC missing initializer false
  // positives.
  std::memset(&sysinfo, 0, sizeof(SYSTEM_INFO));
  GetSystemInfo(&sysinfo);
  return sysinfo.dwNumberOfProcessors;  // number of logical
                                        // processors in the current
                                        // group
#elif defined(BENCHMARK_OS_SOLARIS)
  // Returns -1 in case of a failure.
  int NumCPU = sysconf(_SC_NPROCESSORS_ONLN);
  if (NumCPU < 0) {
    fprintf(stderr,
            "sysconf(_SC_NPROCESSORS_ONLN) failed with error: %s\n",
            strerror(errno));
  }
  return NumCPU;
#else
  int NumCPUs = 0;
  int MaxID = -1;
  std::ifstream f("/proc/cpuinfo");
  if (!f.is_open()) {
    std::cerr << "failed to open /proc/cpuinfo\n";
    return -1;
  }
  const std::string Key = "processor";
  std::string ln;
  while (std::getline(f, ln)) {
    if (ln.empty()) continue;
    size_t SplitIdx = ln.find(':');
    std::string value;
#if defined(__s390__)
    // s390 has another format in /proc/cpuinfo
    // it needs to be parsed differently
    if (SplitIdx != std::string::npos) value = ln.substr(Key.size()+1,SplitIdx-Key.size()-1);
#else
    if (SplitIdx != std::string::npos) value = ln.substr(SplitIdx + 1);
#endif
    if (ln.size() >= Key.size() && ln.compare(0, Key.size(), Key) == 0) {
      NumCPUs++;
      if (!value.empty()) {
        int CurID = benchmark::stoi(value);
        MaxID = std::max(CurID, MaxID);
      }
    }
  }
  if (f.bad()) {
    std::cerr << "Failure reading /proc/cpuinfo\n";
    return -1;
  }
  if (!f.eof()) {
    std::cerr << "Failed to read to end of /proc/cpuinfo\n";
    return -1;
  }
  f.close();

  if ((MaxID + 1) != NumCPUs) {
    fprintf(stderr,
            "CPU ID assignments in /proc/cpuinfo seem messed up."
            " This is usually caused by a bad BIOS.\n");
  }
  return NumCPUs;
#endif
  BENCHMARK_UNREACHABLE();
}

double GetCPUCyclesPerSecond() {
#if defined BENCHMARK_OS_LINUX || defined BENCHMARK_OS_CYGWIN
  long freq;

  // If the kernel is exporting the tsc frequency use that. There are issues
  // where cpuinfo_max_freq cannot be relied on because the BIOS may be
  // exporintg an invalid p-state (on x86) or p-states may be used to put the
  // processor in a new mode (turbo mode). Essentially, those frequencies
  // cannot always be relied upon. The same reasons apply to /proc/cpuinfo as
  // well.
  if (ReadFromFile("/sys/devices/system/cpu/cpu0/tsc_freq_khz", &freq)
      // If CPU scaling is in effect, we want to use the *maximum* frequency,
      // not whatever CPU speed some random processor happens to be using now.
      || ReadFromFile("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq",
                      &freq)) {
    // The value is in kHz (as the file name suggests).  For example, on a
    // 2GHz warpstation, the file contains the value "2000000".
    return freq * 1000.0;
  }

  const double error_value = -1;
  double bogo_clock = error_value;

  std::ifstream f("/proc/cpuinfo");
  if (!f.is_open()) {
    std::cerr << "failed to open /proc/cpuinfo\n";
    return error_value;
  }

  auto startsWithKey = [](std::string const& Value, std::string const& Key) {
    if (Key.size() > Value.size()) return false;
    auto Cmp = [&](char X, char Y) {
      return std::tolower(X) == std::tolower(Y);
    };
    return std::equal(Key.begin(), Key.end(), Value.begin(), Cmp);
  };

  std::string ln;
  while (std::getline(f, ln)) {
    if (ln.empty()) continue;
    size_t SplitIdx = ln.find(':');
    std::string value;
    if (SplitIdx != std::string::npos) value = ln.substr(SplitIdx + 1);
    // When parsing the "cpu MHz" and "bogomips" (fallback) entries, we only
    // accept positive values. Some environments (virtual machines) report zero,
    // which would cause infinite looping in WallTime_Init.
    if (startsWithKey(ln, "cpu MHz")) {
      if (!value.empty()) {
        double cycles_per_second = benchmark::stod(value) * 1000000.0;
        if (cycles_per_second > 0) return cycles_per_second;
      }
    } else if (startsWithKey(ln, "bogomips")) {
      if (!value.empty()) {
        bogo_clock = benchmark::stod(value) * 1000000.0;
        if (bogo_clock < 0.0) bogo_clock = error_value;
      }
    }
  }
  if (f.bad()) {
    std::cerr << "Failure reading /proc/cpuinfo\n";
    return error_value;
  }
  if (!f.eof()) {
    std::cerr << "Failed to read to end of /proc/cpuinfo\n";
    return error_value;
  }
  f.close();
  // If we found the bogomips clock, but nothing better, we'll use it (but
  // we're not happy about it); otherwise, fallback to the rough estimation
  // below.
  if (bogo_clock >= 0.0) return bogo_clock;

#elif defined BENCHMARK_HAS_SYSCTL
  constexpr auto* FreqStr =
#if defined(BENCHMARK_OS_FREEBSD) || defined(BENCHMARK_OS_NETBSD)
      "machdep.tsc_freq";
#elif defined BENCHMARK_OS_OPENBSD
      "hw.cpuspeed";
#else
      "hw.cpufrequency";
#endif
  unsigned long long hz = 0;
#if defined BENCHMARK_OS_OPENBSD
  if (GetSysctl(FreqStr, &hz)) return hz * 1000000;
#else
  if (GetSysctl(FreqStr, &hz)) return hz;
#endif
  fprintf(stderr, "Unable to determine clock rate from sysctl: %s: %s\n",
          FreqStr, strerror(errno));

#elif defined BENCHMARK_OS_WINDOWS
  // In NT, read MHz from the registry. If we fail to do so or we're in win9x
  // then make a crude estimate.
  DWORD data, data_size = sizeof(data);
  if (IsWindowsXPOrGreater() &&
      SUCCEEDED(
          SHGetValueA(HKEY_LOCAL_MACHINE,
                      "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
                      "~MHz", nullptr, &data, &data_size)))
    return static_cast<double>((int64_t)data *
                               (int64_t)(1000 * 1000));  // was mhz
#elif defined (BENCHMARK_OS_SOLARIS)
  kstat_ctl_t *kc = kstat_open();
  if (!kc) {
    std::cerr << "failed to open /dev/kstat\n";
    return -1;
  }
  kstat_t *ksp = kstat_lookup(kc, (char*)"cpu_info", -1, (char*)"cpu_info0");
  if (!ksp) {
    std::cerr << "failed to lookup in /dev/kstat\n";
    return -1;
  }
  if (kstat_read(kc, ksp, NULL) < 0) {
    std::cerr << "failed to read from /dev/kstat\n";
    return -1;
  }
  kstat_named_t *knp =
      (kstat_named_t*)kstat_data_lookup(ksp, (char*)"current_clock_Hz");
  if (!knp) {
    std::cerr << "failed to lookup data in /dev/kstat\n";
    return -1;
  }
  if (knp->data_type != KSTAT_DATA_UINT64) {
    std::cerr << "current_clock_Hz is of unexpected data type: "
              << knp->data_type << "\n";
    return -1;
  }
  double clock_hz = knp->value.ui64;
  kstat_close(kc);
  return clock_hz;
#endif
  // If we've fallen through, attempt to roughly estimate the CPU clock rate.
  const int estimate_time_ms = 1000;
  const auto start_ticks = cycleclock::Now();
  SleepForMilliseconds(estimate_time_ms);
  return static_cast<double>(cycleclock::Now() - start_ticks);
}

std::vector<double> GetLoadAvg() {
#if defined BENCHMARK_OS_FREEBSD || defined(BENCHMARK_OS_LINUX) || \
    defined BENCHMARK_OS_MACOSX || defined BENCHMARK_OS_NETBSD ||  \
    defined BENCHMARK_OS_OPENBSD
  constexpr int kMaxSamples = 3;
  std::vector<double> res(kMaxSamples, 0.0);
  const int nelem = getloadavg(res.data(), kMaxSamples);
  if (nelem < 1) {
    res.clear();
  } else {
    res.resize(nelem);
  }
  return res;
#else
  return {};
#endif
}

}  // end namespace

const CPUInfo& CPUInfo::Get() {
  static const CPUInfo* info = new CPUInfo();
  return *info;
}

CPUInfo::CPUInfo()
    : num_cpus(GetNumCPUs()),
      cycles_per_second(GetCPUCyclesPerSecond()),
      caches(GetCacheSizes()),
      scaling_enabled(CpuScalingEnabled(num_cpus)),
      load_avg(GetLoadAvg()) {}


const SystemInfo& SystemInfo::Get() {
  static const SystemInfo* info = new SystemInfo();
  return *info;
}

SystemInfo::SystemInfo() : name(GetSystemName()) {}
}  // end namespace benchmark
