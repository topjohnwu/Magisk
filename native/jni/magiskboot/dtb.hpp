#pragma once

#include <stdint.h>

#define DT_TABLE_MAGIC  "\xd7\xb7\xab\x1e"
#define QCDT_MAGIC      "QCDT"
#define DTBH_MAGIC      "DTBH"
#define PXADT_MAGIC     "PXA-DT"
#define PXA19xx_MAGIC   "PXA-19xx"
#define SPRD_MAGIC      "SPRD"

struct qcdt_hdr {
    char magic[4];          /* "QCDT" */
    uint32_t version;       /* QCDT version */
    uint32_t num_dtbs;      /* Number of DTBs */
} __attribute__((packed));

struct qctable_v1 {
    uint32_t cpu_info[3];   /* Some CPU info */
    uint32_t offset;        /* DTB offset in QCDT */
    uint32_t len;           /* DTB size */
} __attribute__((packed));

struct qctable_v2 {
    uint32_t cpu_info[4];   /* Some CPU info */
    uint32_t offset;        /* DTB offset in QCDT */
    uint32_t len;           /* DTB size */
} __attribute__((packed));

struct qctable_v3 {
    uint32_t cpu_info[8];   /* Some CPU info */
    uint32_t offset;        /* DTB offset in QCDT */
    uint32_t len;           /* DTB size */
} __attribute__((packed));

struct dtbh_hdr {
    char magic[4];          /* "DTBH" */
    uint32_t version;       /* DTBH version */
    uint32_t num_dtbs;      /* Number of DTBs */
} __attribute__((packed));

struct bhtable_v2 {
    uint32_t cpu_info[5];   /* Some CPU info */
    uint32_t offset;        /* DTB offset in DTBH */
    uint32_t len;           /* DTB size */
    uint32_t space;         /* 0x00000020 */
} __attribute__((packed));

struct pxadt_hdr {
    char magic[6];          /* "PXA-DT" */
    uint32_t version;       /* PXA-* version */
    uint32_t num_dtbs;      /* Number of DTBs */
} __attribute__((packed));

struct pxa19xx_hdr {
    char magic[8];          /* "PXA-19xx" */
    uint32_t version;       /* PXA-* version */
    uint32_t num_dtbs;      /* Number of DTBs */
} __attribute__((packed));

struct pxatable_v1 {
    uint32_t cpu_info[2];   /* Some CPU info */
    uint32_t offset;        /* DTB offset in PXA-* */
    uint32_t len;           /* DTB size */
} __attribute__((packed));

struct sprd_hdr {
    char magic[4];          /* "SPRD" */
    uint32_t version;       /* SPRD version */
    uint32_t num_dtbs;      /* Number of DTBs */
} __attribute__((packed));

struct sprdtable_v1 {
    uint32_t cpu_info[3];   /* Some CPU info */
    uint32_t offset;        /* DTB offset in SPRD */
    uint32_t len;           /* DTB size */
} __attribute__((packed));

/* AOSP DTB/DTBO partition layout */

struct dt_table_header {
    uint32_t magic;             /* DT_TABLE_MAGIC */
    uint32_t total_size;        /* includes dt_table_header + all dt_table_entry */
    uint32_t header_size;       /* sizeof(dt_table_header) */

    uint32_t dt_entry_size;     /* sizeof(dt_table_entry) */
    uint32_t num_dtbs;          /* number of dt_table_entry */
    uint32_t dt_entries_offset; /* offset to the first dt_table_entry */

    uint32_t page_size;         /* flash page size we assume */
    uint32_t version;           /* DTBO image version */
} __attribute__((packed));

struct dt_table_entry {
    uint32_t len;           /* DTB size */
    uint32_t offset;

    uint32_t id;
    uint32_t rev;
    uint32_t flags;

    uint32_t custom[3];
} __attribute__((packed));
