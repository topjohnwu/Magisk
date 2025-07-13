#include "boot-rs.hpp"
#include "format.hpp"

Name2Fmt name2fmt;
Fmt2Name fmt2name;
Fmt2Ext fmt2ext;

#define CHECKED_MATCH(s) (len >= (sizeof(s) - 1) && BUFFER_MATCH(buf, s))

FileFormat check_fmt(const void *buf, size_t len) {
    if (CHECKED_MATCH(CHROMEOS_MAGIC)) {
        return FileFormat::CHROMEOS;
    } else if (CHECKED_MATCH(BOOT_MAGIC)) {
        return FileFormat::AOSP;
    } else if (CHECKED_MATCH(VENDOR_BOOT_MAGIC)) {
        return FileFormat::AOSP_VENDOR;
    } else if (CHECKED_MATCH(GZIP1_MAGIC) || CHECKED_MATCH(GZIP2_MAGIC)) {
        return FileFormat::GZIP;
    } else if (CHECKED_MATCH(LZOP_MAGIC)) {
        return FileFormat::LZOP;
    } else if (CHECKED_MATCH(XZ_MAGIC)) {
        return FileFormat::XZ;
    } else if (len >= 13 && memcmp(buf, "\x5d\x00\x00", 3) == 0
            && (((char *)buf)[12] == '\xff' || ((char *)buf)[12] == '\x00')) {
        return FileFormat::LZMA;
    } else if (CHECKED_MATCH(BZIP_MAGIC)) {
        return FileFormat::BZIP2;
    } else if (CHECKED_MATCH(LZ41_MAGIC) || CHECKED_MATCH(LZ42_MAGIC)) {
        return FileFormat::LZ4;
    } else if (CHECKED_MATCH(LZ4_LEG_MAGIC)) {
        return FileFormat::LZ4_LEGACY;
    } else if (CHECKED_MATCH(MTK_MAGIC)) {
        return FileFormat::MTK;
    } else if (CHECKED_MATCH(DTB_MAGIC)) {
        return FileFormat::DTB;
    } else if (CHECKED_MATCH(DHTB_MAGIC)) {
        return FileFormat::DHTB;
    } else if (CHECKED_MATCH(TEGRABLOB_MAGIC)) {
        return FileFormat::BLOB;
    } else if (len >= 0x28 && memcmp(&((char *)buf)[0x24], ZIMAGE_MAGIC, 4) == 0) {
        return FileFormat::ZIMAGE;
    } else {
        return FileFormat::UNKNOWN;
    }
}

const char *Fmt2Name::operator[](FileFormat fmt) {
    switch (fmt) {
        case FileFormat::GZIP:
            return "gzip";
        case FileFormat::ZOPFLI:
            return "zopfli";
        case FileFormat::LZOP:
            return "lzop";
        case FileFormat::XZ:
            return "xz";
        case FileFormat::LZMA:
            return "lzma";
        case FileFormat::BZIP2:
            return "bzip2";
        case FileFormat::LZ4:
            return "lz4";
        case FileFormat::LZ4_LEGACY:
            return "lz4_legacy";
        case FileFormat::LZ4_LG:
            return "lz4_lg";
        case FileFormat::DTB:
            return "dtb";
        case FileFormat::ZIMAGE:
            return "zimage";
        default:
            return "raw";
    }
}

const char *Fmt2Ext::operator[](FileFormat fmt) {
    switch (fmt) {
        case FileFormat::GZIP:
        case FileFormat::ZOPFLI:
            return ".gz";
        case FileFormat::LZOP:
            return ".lzo";
        case FileFormat::XZ:
            return ".xz";
        case FileFormat::LZMA:
            return ".lzma";
        case FileFormat::BZIP2:
            return ".bz2";
        case FileFormat::LZ4:
        case FileFormat::LZ4_LEGACY:
        case FileFormat::LZ4_LG:
            return ".lz4";
        default:
            return "";
    }
}

#define CHECK(s, f) else if (name == s) return f;

FileFormat Name2Fmt::operator[](std::string_view name) {
    if (0) {}
    CHECK("gzip", FileFormat::GZIP)
    CHECK("zopfli", FileFormat::ZOPFLI)
    CHECK("xz", FileFormat::XZ)
    CHECK("lzma", FileFormat::LZMA)
    CHECK("bzip2", FileFormat::BZIP2)
    CHECK("lz4", FileFormat::LZ4)
    CHECK("lz4_legacy", FileFormat::LZ4_LEGACY)
    CHECK("lz4_lg", FileFormat::LZ4_LG)
    else return FileFormat::UNKNOWN;
}
