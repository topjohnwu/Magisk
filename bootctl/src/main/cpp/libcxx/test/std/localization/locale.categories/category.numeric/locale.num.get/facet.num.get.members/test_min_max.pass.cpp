//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include <limits>
#include <sstream>
#include <iostream>
#include <cassert>
#include <iostream>

using namespace std;

template <class T>
bool check_stream_failed(std::string const& val) {
    istringstream ss(val);
    T result;
    return !(ss >> result);
}

template<typename T>
void check_limits()
{
    const bool is_unsigned = std::is_unsigned<T>::value;
    T minv = numeric_limits<T>::min();
    T maxv = numeric_limits<T>::max();

    ostringstream miniss, maxiss;
    assert(miniss << minv);
    assert(maxiss << maxv);
    std::string mins = miniss.str();
    std::string maxs = maxiss.str();

    istringstream maxoss(maxs), minoss(mins);

    T new_minv, new_maxv;
    assert(maxoss >> new_maxv);
    assert(minoss >> new_minv);

    assert(new_minv == minv);
    assert(new_maxv == maxv);

    maxs[maxs.size() - 1]++;
    assert(check_stream_failed<T>(maxs));
    if (!is_unsigned) {
        mins[mins.size() - 1]++;
        assert(check_stream_failed<T>(mins));
    }
}

int main()
{
    check_limits<short>();
    check_limits<unsigned short>();
    check_limits<int>();
    check_limits<unsigned int>();
    check_limits<long>();
    check_limits<unsigned long>();
    check_limits<long long>();
    check_limits<unsigned long long>();
}
