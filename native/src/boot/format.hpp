#pragma once

#include <string_view>

enum class FileFormat : ::std::uint8_t;

#define COMPRESSED(fmt)      ((+fmt) >= +FileFormat::GZIP && (+fmt) < +FileFormat::LZOP)
#define COMPRESSED_ANY(fmt)  ((+fmt) >= +FileFormat::GZIP && (+fmt) <= +FileFormat::LZOP)

#define BUFFER_MATCH(buf, s) (memcmp(buf, s, sizeof(s) - 1) == 0)
#define BUFFER_CONTAIN(buf, sz, s) (memmem(buf, sz, s, sizeof(s) - 1) != nullptr)

#define BOOT_MAGIC      "ANDROID!"
#define VENDOR_BOOT_MAGIC "VNDRBOOT"
#define CHROMEOS_MAGIC  "CHROMEOS"
#define GZIP1_MAGIC     "\x1f\x8b"
#define GZIP2_MAGIC     "\x1f\x9e"
#define LZOP_MAGIC      "\x89""LZO"
#define XZ_MAGIC        "\xfd""7zXZ"
#define BZIP_MAGIC      "BZh"
#define LZ4_LEG_MAGIC   "\x02\x21\x4c\x18"
#define LZ41_MAGIC      "\x03\x21\x4c\x18"
#define LZ42_MAGIC      "\x04\x22\x4d\x18"
#define MTK_MAGIC       "\x88\x16\x88\x58"
#define DTB_MAGIC       "\xd0\x0d\xfe\xed"
#define LG_BUMP_MAGIC   "\x41\xa9\xe4\x67\x74\x4d\x1d\x1b\xa4\x29\xf2\xec\xea\x65\x52\x79"
#define DHTB_MAGIC      "\x44\x48\x54\x42\x01\x00\x00\x00"
#define SEANDROID_MAGIC "SEANDROIDENFORCE"
#define TEGRABLOB_MAGIC "-SIGNED-BY-SIGNBLOB-"
#define NOOKHD_RL_MAGIC "Red Loader"
#define NOOKHD_GL_MAGIC "Green Loader"
#define NOOKHD_GR_MAGIC "Green Recovery"
#define NOOKHD_EB_MAGIC "eMMC boot.img+secondloader"
#define NOOKHD_ER_MAGIC "eMMC recovery.img+secondloader"
#define NOOKHD_PRE_HEADER_SZ 1048576
#define ACCLAIM_MAGIC   "BauwksBoot"
#define ACCLAIM_PRE_HEADER_SZ 262144
#define AMONET_MICROLOADER_MAGIC "microloader"
#define AMONET_MICROLOADER_SZ 1024
#define AVB_FOOTER_MAGIC "AVBf"
#define AVB_MAGIC "AVB0"
#define ZIMAGE_MAGIC "\x18\x28\x6f\x01"

class Fmt2Name {
public:
    const char *operator[](FileFormat fmt);
};

class Fmt2Ext {
public:
    const char *operator[](FileFormat fmt);
};

class Name2Fmt {
public:
    FileFormat operator[](std::string_view name);
};

FileFormat check_fmt(const void *buf, size_t len);

static inline FileFormat check_fmt(rust::Slice<const uint8_t> bytes) {
    return check_fmt(bytes.data(), bytes.size());
}

extern Name2Fmt name2fmt;
extern Fmt2Name fmt2name;
extern Fmt2Ext fmt2ext;
