#include <benchmark/benchmark.h>

#ifdef __clang__
#pragma clang diagnostic ignored "-Wreturn-type"
#endif

extern "C" {

extern int ExternInt;
extern int ExternInt2;
extern int ExternInt3;

}

// CHECK-LABEL: test_basic:
extern "C" void test_basic() {
  int x;
  benchmark::DoNotOptimize(&x);
  x = 101;
  benchmark::ClobberMemory();
  // CHECK: leaq [[DEST:[^,]+]], %rax
  // CHECK: movl $101, [[DEST]]
  // CHECK: ret
}

// CHECK-LABEL: test_redundant_store:
extern "C" void test_redundant_store() {
  ExternInt = 3;
  benchmark::ClobberMemory();
  ExternInt = 51;
  // CHECK-DAG: ExternInt
  // CHECK-DAG: movl $3
  // CHECK: movl $51
}

// CHECK-LABEL: test_redundant_read:
extern "C" void test_redundant_read() {
  int x;
  benchmark::DoNotOptimize(&x);
  x = ExternInt;
  benchmark::ClobberMemory();
  x = ExternInt2;
  // CHECK: leaq [[DEST:[^,]+]], %rax
  // CHECK: ExternInt(%rip)
  // CHECK: movl %eax, [[DEST]]
  // CHECK-NOT: ExternInt2
  // CHECK: ret
}

// CHECK-LABEL: test_redundant_read2:
extern "C" void test_redundant_read2() {
  int x;
  benchmark::DoNotOptimize(&x);
  x = ExternInt;
  benchmark::ClobberMemory();
  x = ExternInt2;
  benchmark::ClobberMemory();
  // CHECK: leaq [[DEST:[^,]+]], %rax
  // CHECK: ExternInt(%rip)
  // CHECK: movl %eax, [[DEST]]
  // CHECK: ExternInt2(%rip)
  // CHECK: movl %eax, [[DEST]]
  // CHECK: ret
}
