#include <base.hpp>

#include "magiskboot.hpp"

#define MATCH(p) else if (strncmp(s + skip, p, sizeof(p) - 1) == 0) skip += (sizeof(p) - 1)

static int skip_verity_pattern(const char *s) {
    int skip = s[0] == ',';

    if (0) {}
    MATCH("verifyatboot");
    MATCH("verify");
    MATCH("avb_keys");
    MATCH("avb");
    MATCH("support_scfs");
    MATCH("fsverity");
    else return -1;

    if (s[skip] == '=') {
        while (!strchr(" \n,", s[skip]))
            ++skip;
    }
    return skip;
}

static int skip_encryption_pattern(const char *s) {
    int skip = s[0] == ',';

    if (0) {}
    MATCH("forceencrypt");
    MATCH("forcefdeorfbe");
    MATCH("fileencryption");
    else return -1;

    if (s[skip] == '=') {
        while (!strchr(" \n,", s[skip]))
            ++skip;
    }
    return skip;
}

static size_t remove_pattern(byte_data data, int(*pattern_skip)(const char *)) {
    char *src = reinterpret_cast<char *>(data.buf());
    size_t sz = data.sz();
    int write = 0;
    int read = 0;
    while (read < data.sz()) {
        if (int skip = pattern_skip(src + read); skip > 0) {
            fprintf(stderr, "Remove pattern [%.*s]\n", skip, src + read);
            sz -= skip;
            read += skip;
        } else {
            src[write++] = src[read++];
        }
    }
    memset(src + write, 0, data.sz() - write);
    return sz;
}

size_t patch_verity(rust::Slice<uint8_t> data) {
    return remove_pattern(data, skip_verity_pattern);
}

size_t patch_encryption(rust::Slice<uint8_t> data) {
    return remove_pattern(data, skip_encryption_pattern);
}
