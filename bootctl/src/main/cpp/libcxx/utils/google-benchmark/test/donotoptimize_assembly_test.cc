#include <benchmark/benchmark.h>

#ifdef __clang__
#pragma clang diagnostic ignored "-Wreturn-type"
#endif

extern "C" {

extern int ExternInt;
extern int ExternInt2;
extern int ExternInt3;

inline int Add42(int x) { return x + 42; }

struct NotTriviallyCopyable {
  NotTriviallyCopyable();
  explicit NotTriviallyCopyable(int x) : value(x) {}
  NotTriviallyCopyable(NotTriviallyCopyable const&);
  int value;
};

struct Large {
  int value;
  int data[2];
};

}
// CHECK-LABEL: test_with_rvalue:
extern "C" void test_with_rvalue() {
  benchmark::DoNotOptimize(Add42(0));
  // CHECK: movl $42, %eax
  // CHECK: ret
}

// CHECK-LABEL: test_with_large_rvalue:
extern "C" void test_with_large_rvalue() {
  benchmark::DoNotOptimize(Large{ExternInt, {ExternInt, ExternInt}});
  // CHECK: ExternInt(%rip)
  // CHECK: movl %eax, -{{[0-9]+}}(%[[REG:[a-z]+]]
  // CHECK: movl %eax, -{{[0-9]+}}(%[[REG]])
  // CHECK: movl %eax, -{{[0-9]+}}(%[[REG]])
  // CHECK: ret
}

// CHECK-LABEL: test_with_non_trivial_rvalue:
extern "C" void test_with_non_trivial_rvalue() {
  benchmark::DoNotOptimize(NotTriviallyCopyable(ExternInt));
  // CHECK: mov{{l|q}} ExternInt(%rip)
  // CHECK: ret
}

// CHECK-LABEL: test_with_lvalue:
extern "C" void test_with_lvalue() {
  int x = 101;
  benchmark::DoNotOptimize(x);
  // CHECK-GNU: movl $101, %eax
  // CHECK-CLANG: movl $101, -{{[0-9]+}}(%[[REG:[a-z]+]])
  // CHECK: ret
}

// CHECK-LABEL: test_with_large_lvalue:
extern "C" void test_with_large_lvalue() {
  Large L{ExternInt, {ExternInt, ExternInt}};
  benchmark::DoNotOptimize(L);
  // CHECK: ExternInt(%rip)
  // CHECK: movl %eax, -{{[0-9]+}}(%[[REG:[a-z]+]])
  // CHECK: movl %eax, -{{[0-9]+}}(%[[REG]])
  // CHECK: movl %eax, -{{[0-9]+}}(%[[REG]])
  // CHECK: ret
}

// CHECK-LABEL: test_with_non_trivial_lvalue:
extern "C" void test_with_non_trivial_lvalue() {
  NotTriviallyCopyable NTC(ExternInt);
  benchmark::DoNotOptimize(NTC);
  // CHECK: ExternInt(%rip)
  // CHECK: movl %eax, -{{[0-9]+}}(%[[REG:[a-z]+]])
  // CHECK: ret
}

// CHECK-LABEL: test_with_const_lvalue:
extern "C" void test_with_const_lvalue() {
  const int x = 123;
  benchmark::DoNotOptimize(x);
  // CHECK: movl $123, %eax
  // CHECK: ret
}

// CHECK-LABEL: test_with_large_const_lvalue:
extern "C" void test_with_large_const_lvalue() {
  const Large L{ExternInt, {ExternInt, ExternInt}};
  benchmark::DoNotOptimize(L);
  // CHECK: ExternInt(%rip)
  // CHECK: movl %eax, -{{[0-9]+}}(%[[REG:[a-z]+]])
  // CHECK: movl %eax, -{{[0-9]+}}(%[[REG]])
  // CHECK: movl %eax, -{{[0-9]+}}(%[[REG]])
  // CHECK: ret
}

// CHECK-LABEL: test_with_non_trivial_const_lvalue:
extern "C" void test_with_non_trivial_const_lvalue() {
  const NotTriviallyCopyable Obj(ExternInt);
  benchmark::DoNotOptimize(Obj);
  // CHECK: mov{{q|l}} ExternInt(%rip)
  // CHECK: ret
}

// CHECK-LABEL: test_div_by_two:
extern "C" int test_div_by_two(int input) {
  int divisor = 2;
  benchmark::DoNotOptimize(divisor);
  return input / divisor;
  // CHECK: movl $2, [[DEST:.*]]
  // CHECK: idivl [[DEST]]
  // CHECK: ret
}

// CHECK-LABEL: test_inc_integer:
extern "C" int test_inc_integer() {
  int x = 0;
  for (int i=0; i < 5; ++i)
    benchmark::DoNotOptimize(++x);
  // CHECK: movl $1, [[DEST:.*]]
  // CHECK: {{(addl \$1,|incl)}} [[DEST]]
  // CHECK: {{(addl \$1,|incl)}} [[DEST]]
  // CHECK: {{(addl \$1,|incl)}} [[DEST]]
  // CHECK: {{(addl \$1,|incl)}} [[DEST]]
  // CHECK-CLANG: movl [[DEST]], %eax
  // CHECK: ret
  return x;
}

// CHECK-LABEL: test_pointer_rvalue
extern "C" void test_pointer_rvalue() {
  // CHECK: movl $42, [[DEST:.*]]
  // CHECK: leaq [[DEST]], %rax
  // CHECK-CLANG: movq %rax, -{{[0-9]+}}(%[[REG:[a-z]+]])
  // CHECK: ret
  int x = 42;
  benchmark::DoNotOptimize(&x);
}

// CHECK-LABEL: test_pointer_const_lvalue:
extern "C" void test_pointer_const_lvalue() {
  // CHECK: movl $42, [[DEST:.*]]
  // CHECK: leaq [[DEST]], %rax
  // CHECK-CLANG: movq %rax, -{{[0-9]+}}(%[[REG:[a-z]+]])
  // CHECK: ret
  int x = 42;
  int * const xp = &x;
  benchmark::DoNotOptimize(xp);
}

// CHECK-LABEL: test_pointer_lvalue:
extern "C" void test_pointer_lvalue() {
  // CHECK: movl $42, [[DEST:.*]]
  // CHECK: leaq [[DEST]], %rax
  // CHECK-CLANG: movq %rax, -{{[0-9]+}}(%[[REG:[a-z+]+]])
  // CHECK: ret
  int x = 42;
  int *xp = &x;
  benchmark::DoNotOptimize(xp);
}
