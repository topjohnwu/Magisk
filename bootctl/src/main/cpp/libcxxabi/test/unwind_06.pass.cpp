//===------------------------- unwind_06.cpp ------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: libcxxabi-no-exceptions

#include <exception>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

// Compile with -Os to get compiler uses float registers to hold float variables

double get_(int x) { return (double)x; }

double (* volatile get)(int) = get_;

volatile int counter;

double try1(bool v) {
  double a = get(0);
  double b = get(1);
  for (counter = 100; counter; --counter)
    a += get(1) + b;
  if (v) throw 10;
  return get(0)+a+b;
}

double try2(bool v) {
  double a = get(0);
  double b = get(1);
  double c = get(2);
  for (counter = 100; counter; --counter)
    a += get(1) + b + c;
  if (v) throw 10;
  return get(0)+a+b+c;
}

double try3(bool v) {
  double a = get(0);
  double b = get(1);
  double c = get(2);
  double d = get(3);
  for (counter = 100; counter; --counter)
    a += get(1) + b + c + d;
  if (v) throw 10;
  return get(0)+a+b+c+d;
}

double try4(bool v) {
  double a = get(0);
  double b = get(0);
  double c = get(0);
  double d = get(0);
  double e = get(0);
  for (counter = 100; counter; --counter)
    a += get(1) + b+c+d+e;
  if (v) throw 10;
  return get(0)+a+b+c+d+e;
}

double try5(bool v) {
  double a = get(0);
  double b = get(0);
  double c = get(0);
  double d = get(0);
  double e = get(0);
  double f = get(0);
  for (counter = 100; counter; --counter)
    a += get(1) + b+c+d+e+f;
  if (v) throw 10;
  return get(0)+a+b+c+d+e+f;
}

double try6(bool v) {
  double a = get(0);
  double b = get(0);
  double c = get(0);
  double d = get(0);
  double e = get(0);
  double f = get(0);
  double g = get(0);
  for (counter = 100; counter; --counter)
    a += get(1) + b+c+d+e+f+g;
  if (v) throw 10;
  return get(0)+a+b+c+d+e+f+g;
}

double try7(bool v) {
  double a = get(0);
  double b = get(0);
  double c = get(0);
  double d = get(0);
  double e = get(0);
  double f = get(0);
  double g = get(0);
  double h = get(0);
  for (counter = 100; counter; --counter)
    a += get(1) + b+c+d+e+f+g+h;
  if (v) throw 10;
  return get(0)+a+b+c+d+e+f+g+h;
}

double try8(bool v) {
  double a = get(0);
  double b = get(0);
  double c = get(0);
  double d = get(0);
  double e = get(0);
  double f = get(0);
  double g = get(0);
  double h = get(0);
  double i = get(0);
  for (counter = 100; counter; --counter)
    a += get(1) + b+c+d+e+f+g+h+i;
  if (v) throw 10;
  return get(0)+a+b+c+d+e+f+g+h+i;
}





double foo()
{
  double a = get(1);
  double b = get(2);
  double c = get(3);
  double d = get(4);
  double e = get(5);
  double f = get(6);
  double g = get(7);
  double h = get(8);
  try {
    try1(true);    
  }
  catch (int e) {
  }
  assert(a == get(1));
  assert(b == get(2));
  assert(c == get(3));
  assert(d == get(4));
  assert(e == get(5));
  assert(f == get(6));
  assert(g == get(7));
  assert(h == get(8));
  
  try {
    try2(true);    
  }
  catch (int e) {
  }
  assert(a == get(1));
  assert(b == get(2));
  assert(c == get(3));
  assert(d == get(4));
  assert(e == get(5));
  assert(f == get(6));
  assert(g == get(7));
  assert(h == get(8));
  
  try {
    try3(true);    
  }
  catch (int e) {
  }
  assert(a == get(1));
  assert(b == get(2));
  assert(c == get(3));
  assert(d == get(4));
  assert(e == get(5));
  assert(f == get(6));
  assert(g == get(7));
  assert(h == get(8));
  
  try {
    try4(true);    
  }
  catch (int e) {
  }
  assert(a == get(1));
  assert(b == get(2));
  assert(c == get(3));
  assert(d == get(4));
  assert(e == get(5));
  assert(f == get(6));
  assert(g == get(7));
  assert(h == get(8));
  
  try {
    try5(true);    
  }
  catch (int e) {
  }
  assert(a == get(1));
  assert(b == get(2));
  assert(c == get(3));
  assert(d == get(4));
  assert(e == get(5));
  assert(f == get(6));
  assert(g == get(7));
  assert(h == get(8));
  
  try {
    try6(true);    
  }
  catch (int e) {
  }
  assert(a == get(1));
  assert(b == get(2));
  assert(c == get(3));
  assert(d == get(4));
  assert(e == get(5));
  assert(f == get(6));
  assert(g == get(7));
  assert(h == get(8));
  
  try {
    try7(true);    
  }
  catch (int e) {
  }
  assert(a == get(1));
  assert(b == get(2));
  assert(c == get(3));
  assert(d == get(4));
  assert(e == get(5));
  assert(f == get(6));
  assert(g == get(7));
  assert(h == get(8));
  
  try {
    try8(true);    
  }
  catch (int e) {
  }
  assert(a == get(1));
  assert(b == get(2));
  assert(c == get(3));
  assert(d == get(4));
  assert(e == get(5));
  assert(f == get(6));
  assert(g == get(7));
  assert(h == get(8));

  return a+b+c+d+e+f+g+h;
}



int main()
{
  foo();
}
