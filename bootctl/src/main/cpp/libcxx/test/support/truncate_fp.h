//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

inline long double truncate_fp(long double val) {
  volatile long double sink = val;
  return sink;
}

inline double truncate_fp(double val) {
  volatile double sink = val;
  return sink;
}

inline float truncate_fp(float val) {
  volatile float sink = val;
  return sink;
}
