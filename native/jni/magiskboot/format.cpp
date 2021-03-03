#include "format.hpp"

Name2Fmt name2fmt;
Fmt2Name fmt2name;
Fmt2Ext fmt2ext;

#define CHECKED_MATCH(s) (len >= (sizeof(s) - 1) && BUFFER_MATCH(buf, s))

format_t check_fmt(const void *buf, size_t len) {
    if (CHECKED_MATCH(CHROMEOS_MAGIC)) {
        return CHROMEOS;
    } else if (CHECKED_MATCH(BOOT_MAGIC)) {
        return AOSP;
    } else if (CHECKED_MATCH(VENDOR_BOOT_MAGIC)) {
        return AOSP_VENDOR;
    } else if (CHECKED_MATCH(GZIP1_MAGIC) || CHECKED_MATCH(GZIP2_MAGIC)) {
        return GZIP;
    } else if (CHECKED_MATCH(LZOP_MAGIC)) {
        return LZOP;
    } else if (CHECKED_MATCH(XZ_MAGIC)) {
        return XZ;
    } else if (len >= 13 && memcmp(buf, "\x5d\x00\x00", 3) == 0
            && (((char *)buf)[12] == '\xff' || ((char *)buf)[12] == '\x00')) {
        return LZMA;
    } else if (CHECKED_MATCH(BZIP_MAGIC)) {
        return BZIP2;
    } else if (CHECKED_MATCH(LZ41_MAGIC) || CHECKED_MATCH(LZ42_MAGIC)) {
        return LZ4;
    } else if (CHECKED_MATCH(LZ4_LEG_MAGIC)) {
        return LZ4_LEGACY;
    } else if (CHECKED_MATCH(MTK_MAGIC)) {
        return MTK;
    } else if (CHECKED_MATCH(DTB_MAGIC)) {
        return DTB;
    } else if (CHECKED_MATCH(DHTB_MAGIC)) {
        return DHTB;
    } else if (CHECKED_MATCH(TEGRABLOB_MAGIC)) {
        return BLOB;
    } else {
        return UNKNOWN;
    }
}

const char *Fmt2Name::operator[](format_t fmt) {
    switch (fmt) {
        case GZIP:
            return "gzip";
        case LZOP:
            return "lzop";
        case XZ:
            return "xz";
        case LZMA:
            return "lzma";
        case BZIP2:
            return "bzip2";
        case LZ4:
            return "lz4";
        case LZ4_LEGACY:
            return "lz4_legacy";
        case LZ4_LG:
            return "lz4_lg";
        case DTB:
            return "dtb";
        default:
            return "raw";
    }
}

const char *Fmt2Ext::operator[](format_t fmt) {
    switch (fmt) {
        case GZIP:
            return ".gz";
        case LZOP:
            return ".lzo";
        case XZ:
            return ".xz";
        case LZMA:
            return ".lzma";
        case BZIP2:
            return ".bz2";
        case LZ4:
        case LZ4_LEGACY:
        case LZ4_LG:
            return ".lz4";
        default:
            return "";
    }
}

#define CHECK(s, f) else if (name == s) return f;

format_t Name2Fmt::operator[](std::string_view name) {
    if (0) {}
    CHECK("gzip", GZIP)
    CHECK("xz", XZ)
    CHECK("lzma", LZMA)
    CHECK("bzip2", BZIP2)
    CHECK("lz4", LZ4)
    CHECK("lz4_legacy", LZ4_LEGACY)
    CHECK("lz4_lg", LZ4_LG)
    else return UNKNOWN;
}
