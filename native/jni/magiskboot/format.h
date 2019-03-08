#pragma once

#include <map>
#include <string_view>

typedef enum {
	UNKNOWN,
/* Boot formats */
	CHROMEOS,
	AOSP,
	ELF32,
	ELF64,
	DHTB,
	BLOB,
/* Compression formats */
	GZIP,
	XZ,
	LZMA,
	BZIP2,
	LZ4,
	LZ4_LEGACY,
/* Misc */
	LZOP,
	MTK,
	DTB,
} format_t;

#define COMPRESSED(fmt)  ((fmt) >= GZIP && (fmt) <= LZ4_LEGACY)

#define BOOT_MAGIC      "ANDROID!"
#define CHROMEOS_MAGIC  "CHROMEOS"
#define ELF32_MAGIC     "\x7f""ELF\x01"
#define ELF64_MAGIC     "\x7f""ELF\x02"
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
#define NOOKHD_MAGIC    "Green Loader"
#define NOOKHD_NEW_MAGIC "eMMC boot.img+secondloader"
#define NOOKHD_PRE_HEADER_SZ 1048576
#define ACCLAIM_MAGIC   "BauwksBoot"
#define ACCLAIM_PRE_HEADER_SZ 262144

class Fmt2Name {
public:
	const char *operator[](format_t fmt);
};

class Fmt2Ext {
public:
	const char *operator[](format_t fmt);
};

format_t check_fmt(const void *buf, size_t len);

extern std::map<std::string_view, format_t> name2fmt;
extern Fmt2Name fmt2name;
extern Fmt2Ext fmt2ext;
