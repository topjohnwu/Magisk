// -*- C++ -*-
//===------------------------ unique_copy.cpp -----------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// XFAIL

#include "fuzzing.h"
#include <cassert>
#include <cstring> // for strlen

const char * test_cases[] = {
	"",
	"s",
	"bac",
	"bacasf"
	"lkajseravea",
	"adsfkajdsfjkas;lnc441324513,34535r34525234"
	};

const size_t k_num_tests = sizeof(test_cases)/sizeof(test_cases[0]);


int main ()
{
	for (size_t i = 0; i < k_num_tests; ++i)
		{
		const size_t   size = std::strlen(test_cases[i]);
		const uint8_t *data = (const uint8_t *) test_cases[i];
		assert(0 == fuzzing::unique_copy(data, size));
		}
	return 0;
}
