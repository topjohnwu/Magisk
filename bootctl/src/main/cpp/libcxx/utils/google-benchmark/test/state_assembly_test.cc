#include <benchmark/benchmark.h>

#ifdef __clang__
#pragma clang diagnostic ignored "-Wreturn-type"
#endif

// clang-format off
extern "C" {
  extern int ExternInt;
  benchmark::State& GetState();
  void Fn();
}
// clang-format on

using benchmark::State;

// CHECK-LABEL: test_for_auto_loop:
extern "C" int test_for_auto_loop() {
  State& S = GetState();
  int x = 42;
  // CHECK: 	[[CALL:call(q)*]]	_ZN9benchmark5State16StartKeepRunningEv
  // CHECK-NEXT: testq %rbx, %rbx
  // CHECK-NEXT: je [[LOOP_END:.*]]

  for (auto _ : S) {
    // CHECK: .L[[LOOP_HEAD:[a-zA-Z0-9_]+]]:
    // CHECK-GNU-NEXT: subq $1, %rbx
    // CHECK-CLANG-NEXT: {{(addq \$1,|incq)}} %rax
    // CHECK-NEXT: jne .L[[LOOP_HEAD]]
    benchmark::DoNotOptimize(x);
  }
  // CHECK: [[LOOP_END]]:
  // CHECK: [[CALL]]	_ZN9benchmark5State17FinishKeepRunningEv

  // CHECK: movl $101, %eax
  // CHECK: ret
  return 101;
}

// CHECK-LABEL: test_while_loop:
extern "C" int test_while_loop() {
  State& S = GetState();
  int x = 42;

  // CHECK: j{{(e|mp)}} .L[[LOOP_HEADER:[a-zA-Z0-9_]+]]
  // CHECK-NEXT: .L[[LOOP_BODY:[a-zA-Z0-9_]+]]:
  while (S.KeepRunning()) {
    // CHECK-GNU-NEXT: subq $1, %[[IREG:[a-z]+]]
    // CHECK-CLANG-NEXT: {{(addq \$-1,|decq)}} %[[IREG:[a-z]+]]
    // CHECK: movq %[[IREG]], [[DEST:.*]]
    benchmark::DoNotOptimize(x);
  }
  // CHECK-DAG: movq [[DEST]], %[[IREG]]
  // CHECK-DAG: testq %[[IREG]], %[[IREG]]
  // CHECK-DAG: jne .L[[LOOP_BODY]]
  // CHECK-DAG: .L[[LOOP_HEADER]]:

  // CHECK: cmpb $0
  // CHECK-NEXT: jne .L[[LOOP_END:[a-zA-Z0-9_]+]]
  // CHECK: [[CALL:call(q)*]] _ZN9benchmark5State16StartKeepRunningEv

  // CHECK: .L[[LOOP_END]]:
  // CHECK: [[CALL]] _ZN9benchmark5State17FinishKeepRunningEv

  // CHECK: movl $101, %eax
  // CHECK: ret
  return 101;
}
