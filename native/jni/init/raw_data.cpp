#include "raw_data.hpp"

using namespace std;

int data_holder::patch(str_pairs list) {
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

bool data_holder::contains(string_view pattern) {
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

void data_holder::consume(data_holder &other) {
    buf = other.buf;
    sz = other.sz;
    other.buf = nullptr;
    other.sz = 0;
}

auto_data<HEAP> raw_data::read(int fd) {
    auto_data<HEAP> data;
    fd_full_read(fd, data.buf, data.sz);
    return data;
}

auto_data<HEAP> raw_data::read(const char *name) {
    auto_data<HEAP> data;
    full_read(name, data.buf, data.sz);
    return data;
}

auto_data<MMAP> raw_data::mmap_rw(const char *name) {
    auto_data<MMAP> data;
    ::mmap_rw(name, data.buf, data.sz);
    return data;
}

auto_data<MMAP> raw_data::mmap_ro(const char *name) {
    auto_data<MMAP> data;
    ::mmap_ro(name, data.buf, data.sz);
    return data;
}
