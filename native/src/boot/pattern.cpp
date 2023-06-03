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

static bool remove_pattern(byte_data &data, int(*pattern_skip)(const char *)) {
    char *src = reinterpret_cast<char *>(data.buf());
    size_t orig_sz = data.sz();
    int write = 0;
    int read = 0;
    while (read < orig_sz) {
        if (int skip = pattern_skip(src + read); skip > 0) {
            fprintf(stderr, "Remove pattern [%.*s]\n", skip, src + read);
            data.sz() -= skip;
            read += skip;
        } else {
            src[write++] = src[read++];
        }
    }
    memset(src + write, 0, orig_sz - write);
    return data.sz() != orig_sz;
}

bool patch_verity(byte_data &data) {
    return remove_pattern(data, skip_verity_pattern);
}

bool patch_encryption(byte_data &data) {
    return remove_pattern(data, skip_encryption_pattern);
}
