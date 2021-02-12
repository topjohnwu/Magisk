#pragma once

#include <utils.hpp>

struct mmap_data {
    uint8_t *buf = nullptr;
    size_t sz = 0;

    mmap_data() = default;
    mmap_data(const mmap_data&) = delete;
    mmap_data(mmap_data &&other) { consume(other); }
    ~mmap_data() { if (buf) munmap(buf, sz); }
    mmap_data& operator=(mmap_data &&other) { consume(other); return *this; }

    using str_pairs = std::initializer_list<std::pair<std::string_view, std::string_view>>;
    int patch(str_pairs list);
    bool contains(std::string_view pattern);

    static mmap_data rw(const char *name);
    static mmap_data ro(const char *name);

private:
    void consume(mmap_data &other);
};
