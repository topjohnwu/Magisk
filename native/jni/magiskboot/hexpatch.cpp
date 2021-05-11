#include <sys/mman.h>

#include <utils.hpp>

#include "magiskboot.hpp"

using namespace std;

static void hex2byte(const char *hex, uint8_t *buf) {
    char high, low;
    for (int i = 0, length = strlen(hex); i < length; i += 2) {
        high = toupper(hex[i]) - '0';
        low = toupper(hex[i + 1]) - '0';
        buf[i / 2] = ((high > 9 ? high - 7 : high) << 4) + (low > 9 ? low - 7 : low);
    }
}

int hexpatch(const char *image, const char *from, const char *to) {
    int patched = 1;

    uint8_t *buf;
    size_t sz;
    mmap_rw(image, buf, sz);
    run_finally f([=]{ munmap(buf, sz); });

    vector<uint8_t> pattern(strlen(from) / 2);
    vector<uint8_t> patch(strlen(to) / 2);

    hex2byte(from, pattern.data());
    hex2byte(to, patch.data());

    uint8_t * const end = buf + sz;
    for (uint8_t *curr = buf; curr < end; curr += pattern.size()) {
        curr = static_cast<uint8_t*>(memmem(curr, end - curr, pattern.data(), pattern.size()));
        if (curr == nullptr)
            return patched;
        fprintf(stderr, "Patch @ %08X [%s] -> [%s]\n", (unsigned)(curr - buf), from, to);
        memset(curr, 0, pattern.size());
        memcpy(curr, patch.data(), patch.size());
        patched = 0;
    }

    return patched;
}
