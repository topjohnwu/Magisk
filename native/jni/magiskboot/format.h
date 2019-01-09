#ifndef _FORMAT_H_
#define _FORMAT_H_

typedef enum {
	UNKNOWN,
	CHROMEOS,
	AOSP,
	ELF32,
	ELF64,
	GZIP,
	LZOP,
	XZ,
	LZMA,
	BZIP2,
	LZ4,
	LZ4_LEGACY,
	MTK,
	DTB,
	DHTB,
	BLOB
} format_t;

#define COMPRESSED(fmt)  (fmt >= GZIP && fmt <= LZ4_LEGACY)

#define BOOT_MAGIC      "ANDROID!"
#define CHROMEOS_MAGIC  "CHROMEOS"
#define ELF32_MAGIC     "\x7f""ELF\x01"
#define ELF64_MAGIC     "\x7f""ELF\x02"
#define GZIP_MAGIC      "\x1f\x8b\x08\x00"
#define LZOP_MAGIC      "\x89\x4c\x5a\x4f\x00\x0d\x0a\x1a\x0a"
#define XZ_MAGIC        "\xfd""7zXZ\x00"
#define BZIP_MAGIC      "BZh"
#define LZ4_MAGIC       "\x04\x22\x4d\x18"
#define LZ4_LEG_MAGIC   "\x02\x21\x4c\x18"
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

#define SUP_LIST      ((const char *[]) { "gzip", "xz", "lzma", "bzip2", "lz4", "lz4_legacy", NULL })
#define SUP_EXT_LIST  ((const char *[]) { "gz", "xz", "lzma", "bz2", "lz4", "lz4", NULL })

format_t check_fmt(const void *buf, size_t len);
void get_fmt_name(format_t fmt, char *name);

#endif
