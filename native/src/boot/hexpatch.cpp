#include "magiskboot.hpp"

using namespace std;

static void hex2byte(string_view hex, uint8_t *buf) {
    char high, low;
    for (int i = 0, length = hex.length(); i < length; i += 2) {
        high = toupper(hex[i]) - '0';
        low = toupper(hex[i + 1]) - '0';
        buf[i / 2] = ((high > 9 ? high - 7 : high) << 4) + (low > 9 ? low - 7 : low);
    }
}

int hexpatch(const char *file, string_view from, string_view to) {
    mmap_data m(file, true);

    vector<uint8_t> pattern(from.length() / 2);
    vector<uint8_t> patch(to.length() / 2);

    hex2byte(from, pattern.data());
    hex2byte(to, patch.data());

    return m.patch({ make_pair(pattern, patch) }) > 0 ? 0 : 1;
}
