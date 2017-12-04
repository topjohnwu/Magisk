#ifndef _TYPES_H_
#define _TYPES_H_

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
    DTB
} file_t;

#define COMPRESSED(type)  (type >= GZIP && type <= LZ4_LEGACY)

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

extern char *SUP_LIST[];
extern char *SUP_EXT_LIST[];
extern file_t SUP_TYPE_LIST[];

file_t check_type(const void *buf);
void get_type_name(file_t type, char *name);

#endif
