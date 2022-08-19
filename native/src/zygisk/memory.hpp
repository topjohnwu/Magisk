#pragma once

#include <map>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-builtins"
#include <parallel_hashmap/phmap.h>
#pragma clang diagnostic pop

#include <base.hpp>

namespace jni_hook {

struct memory_block {
    static void *allocate(size_t sz);
    static void deallocate(void *, size_t) { /* Monotonic increase */ }
    static void release();
};

template<class T>
using allocator = stateless_allocator<T, memory_block>;

using string = std::basic_string<char, std::char_traits<char>, allocator<char>>;

// Use node_hash_map since it will use less memory because we are using a monotonic allocator
template<class K, class V>
using hash_map = phmap::node_hash_map<K, V,
    phmap::priv::hash_default_hash<K>,
    phmap::priv::hash_default_eq<K>,
    allocator<std::pair<const K, V>>
>;

template<class K, class V>
using tree_map = std::map<K, V,
    std::less<K>,
    allocator<std::pair<const K, V>>
>;

} // namespace jni_hook

// Provide heterogeneous lookup for jni_hook::string
namespace phmap::priv {
template <> struct HashEq<jni_hook::string> : StringHashEqT<char> {};
} // namespace phmap::priv
