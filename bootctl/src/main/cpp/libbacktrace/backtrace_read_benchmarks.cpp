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
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <unistd.h>

#include <memory>
#include <vector>

#include <benchmark/benchmark.h>

#include <backtrace/Backtrace.h>

#define AT_COMMON_SIZES Arg(1)->Arg(4)->Arg(8)->Arg(16)->Arg(100)->Arg(200)->Arg(500)->Arg(1024)

static void Attach(pid_t pid) {
  if (ptrace(PTRACE_ATTACH, pid, 0, 0) == -1) {
    perror("Failed to attach");
    abort();
  }

  siginfo_t si;
  // Wait for up to 5 seconds.
  for (size_t i = 0; i < 5000; i++) {
    if (ptrace(PTRACE_GETSIGINFO, pid, 0, &si) == 0) {
      return;
    }
    usleep(1000);
  }
  printf("Remote process failed to stop in five seconds.\n");
  abort();
}

class ScopedPidReaper {
 public:
  ScopedPidReaper(pid_t pid) : pid_(pid) {}
  ~ScopedPidReaper() {
    kill(pid_, SIGKILL);
    waitpid(pid_, nullptr, 0);
  }

 private:
  pid_t pid_;
};

static size_t ProcessVmRead(pid_t pid, uint64_t remote_src, void* dst, size_t len) {
  struct iovec dst_iov = {
      .iov_base = dst, .iov_len = len,
  };

  struct iovec src_iov = {
      .iov_base = reinterpret_cast<void*>(remote_src), .iov_len = len,
  };

  ssize_t rc = process_vm_readv(pid, &dst_iov, 1, &src_iov, 1, 0);
  return rc == -1 ? 0 : rc;
}

static bool PtraceReadLong(pid_t pid, uint64_t addr, long* value) {
  // ptrace() returns -1 and sets errno when the operation fails.
  // To disambiguate -1 from a valid result, we clear errno beforehand.
  errno = 0;
  *value = ptrace(PTRACE_PEEKTEXT, pid, reinterpret_cast<void*>(addr), nullptr);
  if (*value == -1 && errno) {
    return false;
  }
  return true;
}

static size_t PtraceRead(pid_t pid, uint64_t addr, void* dst, size_t bytes) {
  size_t bytes_read = 0;
  long data;
  for (size_t i = 0; i < bytes / sizeof(long); i++) {
    if (!PtraceReadLong(pid, addr, &data)) {
      return bytes_read;
    }
    memcpy(dst, &data, sizeof(long));
    dst = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(dst) + sizeof(long));
    addr += sizeof(long);
    bytes_read += sizeof(long);
  }

  size_t left_over = bytes & (sizeof(long) - 1);
  if (left_over) {
    if (!PtraceReadLong(pid, addr, &data)) {
      return bytes_read;
    }
    memcpy(dst, &data, left_over);
    bytes_read += left_over;
  }
  return bytes_read;
}

static void CreateRemoteProcess(size_t size, void** map, pid_t* pid) {
  *map = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (*map == MAP_FAILED) {
    perror("Can't allocate memory");
    abort();
  }
  memset(*map, 0xaa, size);

  if ((*pid = fork()) == 0) {
    for (volatile int i = 0;; i++)
      ;
    exit(1);
  }
  if (*pid < 0) {
    perror("Failed to fork");
    abort();
  }
  Attach(*pid);
  // Don't need this map in the current process any more.
  munmap(*map, size);
}

static void BM_read_with_ptrace(benchmark::State& state) {
  void* map;
  pid_t pid;
  CreateRemoteProcess(state.range(0), &map, &pid);
  ScopedPidReaper reap(pid);

  std::vector<uint8_t> read_buffer(state.range(0));
  uint64_t addr = reinterpret_cast<uint64_t>(map);
  while (state.KeepRunning()) {
    if (PtraceRead(pid, addr, read_buffer.data(), read_buffer.size()) != read_buffer.size()) {
      printf("Unexpected bad read.\n");
      abort();
    }
  }
  ptrace(PTRACE_DETACH, pid, 0, 0);
}
BENCHMARK(BM_read_with_ptrace)->AT_COMMON_SIZES;

static void BM_read_with_process_vm_read(benchmark::State& state) {
  void* map;
  pid_t pid;
  CreateRemoteProcess(state.range(0), &map, &pid);
  ScopedPidReaper reap(pid);

  std::vector<uint8_t> read_buffer(state.range(0));
  uint64_t addr = reinterpret_cast<uint64_t>(map);
  while (state.KeepRunning()) {
    if (ProcessVmRead(pid, addr, read_buffer.data(), read_buffer.size()) != read_buffer.size()) {
      printf("Unexpected bad read.\n");
      abort();
    }
  }
  ptrace(PTRACE_DETACH, pid, 0, 0);
}
BENCHMARK(BM_read_with_process_vm_read)->AT_COMMON_SIZES;

static void BM_read_with_backtrace_object(benchmark::State& state) {
  void* map;
  pid_t pid;
  CreateRemoteProcess(state.range(0), &map, &pid);
  ScopedPidReaper reap(pid);

  std::unique_ptr<Backtrace> backtrace(Backtrace::Create(pid, BACKTRACE_CURRENT_THREAD));
  if (backtrace.get() == nullptr) {
    printf("Failed to create backtrace.\n");
    abort();
  }

  uint64_t addr = reinterpret_cast<uint64_t>(map);
  std::vector<uint8_t> read_buffer(state.range(0));
  while (state.KeepRunning()) {
    if (backtrace->Read(addr, read_buffer.data(), read_buffer.size()) != read_buffer.size()) {
      printf("Unexpected bad read.\n");
      abort();
    }
  }
  ptrace(PTRACE_DETACH, pid, 0, 0);
}
BENCHMARK(BM_read_with_backtrace_object)->AT_COMMON_SIZES;
