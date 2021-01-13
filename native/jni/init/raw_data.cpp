#include "raw_data.hpp"

using namespace std;

int mmap_data::patch(str_pairs list) {
    if (buf == nullptr)
        return 0;
    int count = 0;
    for (uint8_t *p = buf, *eof = buf + sz; p < eof; ++p) {
        for (auto [from, to] : list) {
            if (memcmp(p, from.data(), from.length() + 1) == 0) {
                LOGD("Replace [%s] -> [%s]\n", from.data(), to.data());
                memset(p, 0, from.length());
                memcpy(p, to.data(), to.length());
                ++count;
                p += from.length();
            }
        }
    }
    return count;
}

bool mmap_data::contains(string_view pattern) {
    if (buf == nullptr)
        return false;
    for (uint8_t *p = buf, *eof = buf + sz; p < eof; ++p) {
        if (memcmp(p, pattern.data(), pattern.length() + 1) == 0) {
            LOGD("Found pattern [%s]\n", pattern.data());
            return true;
        }
    }
    return false;
}

void mmap_data::consume(mmap_data &other) {
    buf = other.buf;
    sz = other.sz;
    other.buf = nullptr;
    other.sz = 0;
}

mmap_data mmap_data::rw(const char *name) {
    mmap_data data;
    mmap_rw(name, data.buf, data.sz);
    return data;
}

mmap_data mmap_data::ro(const char *name) {
    mmap_data data;
    mmap_ro(name, data.buf, data.sz);
    return data;
}
