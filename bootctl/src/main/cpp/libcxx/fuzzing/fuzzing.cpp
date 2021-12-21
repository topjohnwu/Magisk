// -*- C++ -*-
//===------------------------- fuzzing.cpp -------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

//  A set of routines to use when fuzzing the algorithms in libc++
//  Each one tests a single algorithm.
//
//  They all have the form of:
//      int `algorithm`(const uint8_t *data, size_t size);
//
//  They perform the operation, and then check to see if the results are correct.
//  If so, they return zero, and non-zero otherwise.
//
//  For example, sort calls std::sort, then checks two things:
//      (1) The resulting vector is sorted
//      (2) The resulting vector contains the same elements as the original data.



#include "fuzzing.h"
#include <vector>
#include <algorithm>
#include <functional>
#include <regex>
#include <cassert>

#include <iostream>

//  If we had C++14, we could use the four iterator version of is_permutation and equal

namespace fuzzing {

//  This is a struct we can use to test the stable_XXX algorithms.
//  perform the operation on the key, then check the order of the payload.

struct stable_test {
    uint8_t key;
    size_t payload;

    stable_test(uint8_t k) : key(k), payload(0) {}
    stable_test(uint8_t k, size_t p) : key(k), payload(p) {}
    };

void swap(stable_test &lhs, stable_test &rhs)
{
    using std::swap;
    swap(lhs.key,     rhs.key);
    swap(lhs.payload, rhs.payload);
}

struct key_less
{
    bool operator () (const stable_test &lhs, const stable_test &rhs) const
    {
        return lhs.key < rhs.key;
    }
};

struct payload_less
{
    bool operator () (const stable_test &lhs, const stable_test &rhs) const
    {
        return lhs.payload < rhs.payload;
    }
};

struct total_less
{
    bool operator () (const stable_test &lhs, const stable_test &rhs) const
    {
        return lhs.key == rhs.key ? lhs.payload < rhs.payload : lhs.key < rhs.key;
    }
};

bool operator==(const stable_test &lhs, const stable_test &rhs)
{
    return lhs.key == rhs.key && lhs.payload == rhs.payload;
}


template<typename T>
struct is_even
{
    bool operator () (const T &t) const
    {
        return t % 2 == 0;
    }
};


template<>
struct is_even<stable_test>
{
    bool operator () (const stable_test &t) const
    {
        return t.key % 2 == 0;
    }
};

typedef std::vector<uint8_t> Vec;
typedef std::vector<stable_test> StableVec;
typedef StableVec::const_iterator SVIter;

//  Cheap version of is_permutation
//  Builds a set of buckets for each of the key values.
//  Sums all the payloads.
//  Not 100% perfect, but _way_ faster
bool is_permutation(SVIter first1, SVIter last1, SVIter first2)
{
    size_t xBuckets[256]  = {0};
    size_t xPayloads[256] = {0};
    size_t yBuckets[256]  = {0};
    size_t yPayloads[256] = {0};

    for (; first1 != last1; ++first1, ++first2)
    {
        xBuckets [first1->key]++;
        xPayloads[first1->key] += first1->payload;

        yBuckets [first2->key]++;
        yPayloads[first2->key] += first2->payload;
    }

    for (size_t i = 0; i < 256; ++i)
    {
        if (xBuckets[i]  != yBuckets[i])
            return false;
        if (xPayloads[i] != yPayloads[i])
            return false;
    }

    return true;
}

template <typename Iter1, typename Iter2>
bool is_permutation(Iter1 first1, Iter1 last1, Iter2 first2)
{
    static_assert((std::is_same<typename std::iterator_traits<Iter1>::value_type, uint8_t>::value), "");
    static_assert((std::is_same<typename std::iterator_traits<Iter2>::value_type, uint8_t>::value), "");

    size_t xBuckets[256]  = {0};
    size_t yBuckets[256]  = {0};

    for (; first1 != last1; ++first1, ++first2)
    {
        xBuckets [*first1]++;
        yBuckets [*first2]++;
    }

    for (size_t i = 0; i < 256; ++i)
        if (xBuckets[i]  != yBuckets[i])
            return false;

    return true;
}

//  == sort ==
int sort(const uint8_t *data, size_t size)
{
    Vec working(data, data + size);
    std::sort(working.begin(), working.end());

    if (!std::is_sorted(working.begin(), working.end())) return 1;
    if (!fuzzing::is_permutation(data, data + size, working.cbegin())) return 99;
    return 0;
}


//  == stable_sort ==
int stable_sort(const uint8_t *data, size_t size)
{
    StableVec input;
    for (size_t i = 0; i < size; ++i)
        input.push_back(stable_test(data[i], i));
    StableVec working = input;
    std::stable_sort(working.begin(), working.end(), key_less());

    if (!std::is_sorted(working.begin(), working.end(), key_less()))   return 1;
    auto iter = working.begin();
    while (iter != working.end())
    {
        auto range = std::equal_range(iter, working.end(), *iter, key_less());
        if (!std::is_sorted(range.first, range.second, total_less())) return 2;
        iter = range.second;
    }
    if (!fuzzing::is_permutation(input.cbegin(), input.cend(), working.cbegin())) return 99;
    return 0;
}

//  == partition ==
int partition(const uint8_t *data, size_t size)
{
    Vec working(data, data + size);
    auto iter = std::partition(working.begin(), working.end(), is_even<uint8_t>());

    if (!std::all_of (working.begin(), iter, is_even<uint8_t>())) return 1;
    if (!std::none_of(iter,   working.end(), is_even<uint8_t>())) return 2;
    if (!fuzzing::is_permutation(data, data + size, working.cbegin())) return 99;
    return 0;
}


//  == partition_copy ==
int partition_copy(const uint8_t *data, size_t size)
{
    Vec v1, v2;
    auto iter = std::partition_copy(data, data + size,
        std::back_inserter<Vec>(v1), std::back_inserter<Vec>(v2),
        is_even<uint8_t>());

//  The two vectors should add up to the original size
    if (v1.size() + v2.size() != size) return 1;

//  All of the even values should be in the first vector, and none in the second
    if (!std::all_of (v1.begin(), v1.end(), is_even<uint8_t>())) return 2;
    if (!std::none_of(v2.begin(), v2.end(), is_even<uint8_t>())) return 3;

//  Every value in both vectors has to be in the original

//	Make a copy of the input, and sort it
    Vec v0{data, data + size};
    std::sort(v0.begin(), v0.end());

//	Sort each vector and ensure that all of the elements appear in the original input
    std::sort(v1.begin(), v1.end());
    if (!std::includes(v0.begin(), v0.end(), v1.begin(), v1.end())) return 4;

    std::sort(v2.begin(), v2.end());
    if (!std::includes(v0.begin(), v0.end(), v2.begin(), v2.end())) return 5;

//  This, while simple, is really slow - 20 seconds on a 500K element input.
//     for (auto v: v1)
//         if (std::find(data, data + size, v) == data + size) return 4;
//
//     for (auto v: v2)
//         if (std::find(data, data + size, v) == data + size) return 5;

    return 0;
}

//  == stable_partition ==
int stable_partition (const uint8_t *data, size_t size)
{
    StableVec input;
    for (size_t i = 0; i < size; ++i)
        input.push_back(stable_test(data[i], i));
    StableVec working = input;
    auto iter = std::stable_partition(working.begin(), working.end(), is_even<stable_test>());

    if (!std::all_of (working.begin(), iter, is_even<stable_test>())) return 1;
    if (!std::none_of(iter,   working.end(), is_even<stable_test>())) return 2;
    if (!std::is_sorted(working.begin(), iter, payload_less()))   return 3;
    if (!std::is_sorted(iter,   working.end(), payload_less()))   return 4;
    if (!fuzzing::is_permutation(input.cbegin(), input.cend(), working.cbegin())) return 99;
    return 0;
}

//  == nth_element ==
//  use the first element as a position into the data
int nth_element (const uint8_t *data, size_t size)
{
    if (size <= 1) return 0;
    const size_t partition_point = data[0] % size;
    Vec working(data + 1, data + size);
    const auto partition_iter = working.begin() + partition_point;
    std::nth_element(working.begin(), partition_iter, working.end());

//  nth may be the end iterator, in this case nth_element has no effect.
    if (partition_iter == working.end())
    {
        if (!std::equal(data + 1, data + size, working.begin())) return 98;
    }
    else
    {
        const uint8_t nth = *partition_iter;
        if (!std::all_of(working.begin(), partition_iter, [=](uint8_t v) { return v <= nth; }))
            return 1;
        if (!std::all_of(partition_iter, working.end(),   [=](uint8_t v) { return v >= nth; }))
            return 2;
        if (!fuzzing::is_permutation(data + 1, data + size, working.cbegin())) return 99;
        }

    return 0;
}

//  == partial_sort ==
//  use the first element as a position into the data
int partial_sort (const uint8_t *data, size_t size)
{
    if (size <= 1) return 0;
    const size_t sort_point = data[0] % size;
    Vec working(data + 1, data + size);
    const auto sort_iter = working.begin() + sort_point;
    std::partial_sort(working.begin(), sort_iter, working.end());

    if (sort_iter != working.end())
    {
        const uint8_t nth = *std::min_element(sort_iter, working.end());
        if (!std::all_of(working.begin(), sort_iter, [=](uint8_t v) { return v <= nth; }))
            return 1;
        if (!std::all_of(sort_iter, working.end(),   [=](uint8_t v) { return v >= nth; }))
            return 2;
    }
    if (!std::is_sorted(working.begin(), sort_iter)) return 3;
    if (!fuzzing::is_permutation(data + 1, data + size, working.cbegin())) return 99;

    return 0;
}


//  == partial_sort_copy ==
//  use the first element as a count
int partial_sort_copy (const uint8_t *data, size_t size)
{
    if (size <= 1) return 0;
    const size_t num_results = data[0] % size;
    Vec results(num_results);
    (void) std::partial_sort_copy(data + 1, data + size, results.begin(), results.end());

//  The results have to be sorted
    if (!std::is_sorted(results.begin(), results.end())) return 1;
//  All the values in results have to be in the original data
    for (auto v: results)
        if (std::find(data + 1, data + size, v) == data + size) return 2;

//  The things in results have to be the smallest N in the original data
    Vec sorted(data + 1, data + size);
    std::sort(sorted.begin(), sorted.end());
    if (!std::equal(results.begin(), results.end(), sorted.begin())) return 3;
    return 0;
}

//  The second sequence has been "uniqued"
template <typename Iter1, typename Iter2>
static bool compare_unique(Iter1 first1, Iter1 last1, Iter2 first2, Iter2 last2)
{
    assert(first1 != last1 && first2 != last2);
    if (*first1 != *first2) return false;

    uint8_t last_value = *first1;
    ++first1; ++first2;
    while(first1 != last1 && first2 != last2)
    {
    //  Skip over dups in the first sequence
        while (*first1 == last_value)
            if (++first1 == last1) return false;
        if (*first1 != *first2) return false;
        last_value = *first1;
        ++first1; ++first2;
    }

//  Still stuff left in the 'uniqued' sequence - oops
    if (first1 == last1 && first2 != last2) return false;

//  Still stuff left in the original sequence - better be all the same
    while (first1 != last1)
    {
        if (*first1 != last_value) return false;
        ++first1;
    }
    return true;
}

//  == unique ==
int unique (const uint8_t *data, size_t size)
{
    Vec working(data, data + size);
    std::sort(working.begin(), working.end());
    Vec results = working;
    Vec::iterator new_end = std::unique(results.begin(), results.end());
    Vec::iterator it;   // scratch iterator

//  Check the size of the unique'd sequence.
//  it should only be zero if the input sequence was empty.
    if (results.begin() == new_end)
        return working.size() == 0 ? 0 : 1;

//  'results' is sorted
    if (!std::is_sorted(results.begin(), new_end)) return 2;

//  All the elements in 'results' must be different
    it = results.begin();
    uint8_t prev_value = *it++;
    for (; it != new_end; ++it)
    {
        if (*it == prev_value) return 3;
        prev_value = *it;
    }

//  Every element in 'results' must be in 'working'
    for (it = results.begin(); it != new_end; ++it)
        if (std::find(working.begin(), working.end(), *it) == working.end())
            return 4;

//  Every element in 'working' must be in 'results'
    for (auto v : working)
        if (std::find(results.begin(), new_end, v) == new_end)
            return 5;

    return 0;
}

//  == unique_copy ==
int unique_copy (const uint8_t *data, size_t size)
{
    Vec working(data, data + size);
    std::sort(working.begin(), working.end());
    Vec results;
    (void) std::unique_copy(working.begin(), working.end(),
                            std::back_inserter<Vec>(results));
    Vec::iterator it;   // scratch iterator

//  Check the size of the unique'd sequence.
//  it should only be zero if the input sequence was empty.
    if (results.size() == 0)
        return working.size() == 0 ? 0 : 1;

//  'results' is sorted
    if (!std::is_sorted(results.begin(), results.end())) return 2;

//  All the elements in 'results' must be different
    it = results.begin();
    uint8_t prev_value = *it++;
    for (; it != results.end(); ++it)
    {
        if (*it == prev_value) return 3;
        prev_value = *it;
    }

//  Every element in 'results' must be in 'working'
    for (auto v : results)
        if (std::find(working.begin(), working.end(), v) == working.end())
            return 4;

//  Every element in 'working' must be in 'results'
    for (auto v : working)
        if (std::find(results.begin(), results.end(), v) == results.end())
            return 5;

    return 0;
}


// --   regex fuzzers
static int regex_helper(const uint8_t *data, size_t size, std::regex::flag_type flag)
{
    if (size > 0)
    {
        try
        {
            std::string s((const char *)data, size);
            std::regex re(s, flag);
            return std::regex_match(s, re) ? 1 : 0;
        }
        catch (std::regex_error &ex) {}
    }
    return 0;
}


int regex_ECMAScript (const uint8_t *data, size_t size)
{
    (void) regex_helper(data, size, std::regex_constants::ECMAScript);
    return 0;
}

int regex_POSIX (const uint8_t *data, size_t size)
{
    (void) regex_helper(data, size, std::regex_constants::basic);
    return 0;
}

int regex_extended (const uint8_t *data, size_t size)
{
    (void) regex_helper(data, size, std::regex_constants::extended);
    return 0;
}

int regex_awk (const uint8_t *data, size_t size)
{
    (void) regex_helper(data, size, std::regex_constants::awk);
    return 0;
}

int regex_grep (const uint8_t *data, size_t size)
{
    (void) regex_helper(data, size, std::regex_constants::grep);
    return 0;
}

int regex_egrep (const uint8_t *data, size_t size)
{
    (void) regex_helper(data, size, std::regex_constants::egrep);
    return 0;
}

// --   heap fuzzers
int make_heap (const uint8_t *data, size_t size)
{
    Vec working(data, data + size);
    std::make_heap(working.begin(), working.end());

    if (!std::is_heap(working.begin(), working.end())) return 1;
    if (!fuzzing::is_permutation(data, data + size, working.cbegin())) return 99;
    return 0;
}

int push_heap (const uint8_t *data, size_t size)
{
    if (size < 2) return 0;

//  Make a heap from the first half of the data
    Vec working(data, data + size);
    auto iter = working.begin() + (size / 2);
    std::make_heap(working.begin(), iter);
    if (!std::is_heap(working.begin(), iter)) return 1;

//  Now push the rest onto the heap, one at a time
    ++iter;
    for (; iter != working.end(); ++iter) {
        std::push_heap(working.begin(), iter);
        if (!std::is_heap(working.begin(), iter)) return 2;
        }

    if (!fuzzing::is_permutation(data, data + size, working.cbegin())) return 99;
    return 0;
}

int pop_heap (const uint8_t *data, size_t size)
{
    if (size < 2) return 0;
    Vec working(data, data + size);
    std::make_heap(working.begin(), working.end());

//  Pop things off, one at a time
    auto iter = --working.end();
    while (iter != working.begin()) {
        std::pop_heap(working.begin(), iter);
        if (!std::is_heap(working.begin(), --iter)) return 2;
        }

    return 0;
}


// --   search fuzzers
int search (const uint8_t *data, size_t size)
{
    if (size < 2) return 0;

    const size_t pat_size = data[0] * (size - 1) / std::numeric_limits<uint8_t>::max();
    assert(pat_size <= size - 1);
    const uint8_t *pat_begin = data + 1;
    const uint8_t *pat_end   = pat_begin + pat_size;
    const uint8_t *data_end  = data + size;
    assert(pat_end <= data_end);
//  std::cerr << "data[0] = " << size_t(data[0]) << " ";
//  std::cerr << "Pattern size = " << pat_size << "; corpus is " << size - 1 << std::endl;
    auto it = std::search(pat_end, data_end, pat_begin, pat_end);
    if (it != data_end) // not found
        if (!std::equal(pat_begin, pat_end, it))
            return 1;
    return 0;
}

template <typename S>
static int search_helper (const uint8_t *data, size_t size)
{
    if (size < 2) return 0;

    const size_t pat_size = data[0] * (size - 1) / std::numeric_limits<uint8_t>::max();
    const uint8_t *pat_begin = data + 1;
    const uint8_t *pat_end   = pat_begin + pat_size;
    const uint8_t *data_end  = data + size;

    auto it = std::search(pat_end, data_end, S(pat_begin, pat_end));
    if (it != data_end) // not found
        if (!std::equal(pat_begin, pat_end, it))
            return 1;
    return 0;
}

//  These are still in std::experimental
// int search_boyer_moore (const uint8_t *data, size_t size)
// {
//  return search_helper<std::boyer_moore_searcher<const uint8_t *>>(data, size);
// }
//
// int search_boyer_moore_horspool (const uint8_t *data, size_t size)
// {
//  return search_helper<std::boyer_moore_horspool_searcher<const uint8_t *>>(data, size);
// }


// --   set operation fuzzers
template <typename S>
static void set_helper (const uint8_t *data, size_t size, Vec &v1, Vec &v2)
{
    assert(size > 1);

    const size_t pat_size = data[0] * (size - 1) / std::numeric_limits<uint8_t>::max();
    const uint8_t *pat_begin = data + 1;
    const uint8_t *pat_end   = pat_begin + pat_size;
    const uint8_t *data_end  = data + size;
    v1.assign(pat_begin, pat_end);
    v2.assign(pat_end, data_end);

    std::sort(v1.begin(), v1.end());
    std::sort(v2.begin(), v2.end());
}

} // namespace fuzzing
