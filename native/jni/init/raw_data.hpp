#pragma once

#include <utils.hpp>

struct data_holder {
    uint8_t *buf = nullptr;
    size_t sz = 0;
    using str_pairs = std::initializer_list<std::pair<std::string_view, std::string_view>>;
    int patch(str_pairs list);
    bool contains(std::string_view pattern);
protected:
    void consume(data_holder &other);
};

enum data_type { HEAP, MMAP };
template <data_type T>
struct auto_data : public data_holder {
    auto_data<T>() = default;
    auto_data<T>(const auto_data&) = delete;
    auto_data<T>(auto_data<T> &&other) { consume(other); }
    ~auto_data<T>() {}
    auto_data<T>& operator=(auto_data<T> &&other) { consume(other); return *this; }
};
template <> inline auto_data<MMAP>::~auto_data<MMAP>() { if (buf) munmap(buf, sz); }
template <> inline auto_data<HEAP>::~auto_data<HEAP>() { free(buf); }

namespace raw_data {
    auto_data<HEAP> read(const char *name);
    auto_data<HEAP> read(int fd);
    auto_data<MMAP> mmap_rw(const char *name);
    auto_data<MMAP> mmap_ro(const char *name);
}
