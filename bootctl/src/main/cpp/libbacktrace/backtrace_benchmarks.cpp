/*
 * Copyright (C) 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <string>

#include <android-base/file.h>
#include <android-base/threads.h>

#include <benchmark/benchmark.h>

#include <backtrace/Backtrace.h>
#include <backtrace/BacktraceMap.h>
#include <unwindstack/Memory.h>

constexpr size_t kNumMaps = 2000;

static bool CountMaps(pid_t pid, size_t* num_maps) {
  // Minimize the calls that might allocate memory. If too much memory
  // gets allocated, then this routine will add extra maps and the next
  // call will fail to get the same number of maps as before.
  int fd =
      open((std::string("/proc/") + std::to_string(pid) + "/maps").c_str(), O_RDONLY | O_CLOEXEC);
  if (fd == -1) {
    fprintf(stderr, "Cannot open map file for pid %d: %s\n", pid, strerror(errno));
    return false;
  }
  *num_maps = 0;
  while (true) {
    char buffer[2048];
    ssize_t bytes = read(fd, buffer, sizeof(buffer));
    if (bytes <= 0) {
      break;
    }
    // Count the '\n'.
    for (size_t i = 0; i < static_cast<size_t>(bytes); i++) {
      if (buffer[i] == '\n') {
        ++*num_maps;
      }
    }
  }

  close(fd);
  return true;
}

static void CreateMap(benchmark::State& state, BacktraceMap* (*map_func)(pid_t, bool)) {
  // Create a remote process so that the map data is exactly the same.
  // Also, so that we can create a set number of maps.
  pid_t pid;
  if ((pid = fork()) == 0) {
    size_t num_maps;
    if (!CountMaps(getpid(), &num_maps)) {
      exit(1);
    }
    // Create uniquely named maps.
    std::vector<void*> maps;
    for (size_t i = num_maps; i < kNumMaps; i++) {
      int flags = PROT_READ | PROT_WRITE;
      // Alternate page type to make sure a map entry is added for each call.
      if ((i % 2) == 0) {
        flags |= PROT_EXEC;
      }
      void* memory = mmap(nullptr, PAGE_SIZE, flags, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
      if (memory == MAP_FAILED) {
        fprintf(stderr, "Failed to create map: %s\n", strerror(errno));
        exit(1);
      }
      memset(memory, 0x1, PAGE_SIZE);
#if defined(PR_SET_VMA)
      if (prctl(PR_SET_VMA, PR_SET_VMA_ANON_NAME, memory, PAGE_SIZE, "test_map") == -1) {
        fprintf(stderr, "Failed: %s\n", strerror(errno));
      }
#endif
      maps.push_back(memory);
    }

    if (!CountMaps(getpid(), &num_maps)) {
      exit(1);
    }

    if (num_maps < kNumMaps) {
      fprintf(stderr, "Maps set incorrectly: %zu found, %zu expected at least.\n", num_maps,
              kNumMaps);
      std::string str;
      android::base::ReadFileToString("/proc/self/maps", &str);
      fprintf(stderr, "%s\n", str.c_str());
      exit(1);
    }

    // Wait for an hour at most.
    sleep(3600);
    exit(1);
  } else if (pid < 0) {
    fprintf(stderr, "Fork failed: %s\n", strerror(errno));
    return;
  }

  size_t num_maps = 0;
  for (size_t i = 0; i < 2000; i++) {
    if (CountMaps(pid, &num_maps) && num_maps >= kNumMaps) {
      break;
    }
    usleep(1000);
  }
  if (num_maps < kNumMaps) {
    fprintf(stderr, "Timed out waiting for the number of maps available: %zu\n", num_maps);
    return;
  }

  while (state.KeepRunning()) {
    BacktraceMap* map = map_func(pid, false);
    if (map == nullptr) {
      fprintf(stderr, "Failed to create map\n");
      return;
    }
    delete map;
  }

  kill(pid, SIGKILL);
  waitpid(pid, nullptr, 0);
}

static void BM_create_map(benchmark::State& state) {
  CreateMap(state, BacktraceMap::Create);
}
BENCHMARK(BM_create_map);

using BacktraceCreateFn = decltype(Backtrace::Create);

static void CreateBacktrace(benchmark::State& state, BacktraceMap* map, BacktraceCreateFn fn) {
  while (state.KeepRunning()) {
    std::unique_ptr<Backtrace> backtrace(fn(getpid(), android::base::GetThreadId(), map));
    backtrace->Unwind(0);
  }
}

static void BM_create_backtrace(benchmark::State& state) {
  std::unique_ptr<BacktraceMap> backtrace_map(BacktraceMap::Create(getpid()));
  CreateBacktrace(state, backtrace_map.get(), Backtrace::Create);
}
BENCHMARK(BM_create_backtrace);

BENCHMARK_MAIN();
