#include <string.h>

#include "format.h"

#define MATCH(s) (len >= (sizeof(s) - 1) && memcmp(buf, s, sizeof(s) - 1) == 0)

format_t check_fmt(const void *buf, size_t len) {
	if (MATCH(CHROMEOS_MAGIC)) {
		return CHROMEOS;
	} else if (MATCH(BOOT_MAGIC)) {
		return AOSP;
	} else if (MATCH(ELF32_MAGIC)) {
		return ELF32;
	} else if (MATCH(ELF64_MAGIC)) {
		return ELF64;
	} else if (MATCH(GZIP_MAGIC)) {
		return GZIP;
	} else if (MATCH(LZOP_MAGIC)) {
		return LZOP;
	} else if (MATCH(XZ_MAGIC)) {
		return XZ;
	} else if (len >= 13 && memcmp(buf, "\x5d\x00\x00", 3) == 0
			&& (((char *)buf)[12] == '\xff' || ((char *)buf)[12] == '\x00')) {
		return LZMA;
	} else if (MATCH(BZIP_MAGIC)) {
		return BZIP2;
	} else if (MATCH(LZ4_MAGIC)) {
		return LZ4;
	} else if (MATCH(LZ4_LEG_MAGIC)) {
		return LZ4_LEGACY;
	} else if (MATCH(MTK_MAGIC)) {
		return MTK;
	} else if (MATCH(DTB_MAGIC)) {
		return DTB;
	} else if (MATCH(DHTB_MAGIC)) {
		return DHTB;
	} else if (MATCH(TEGRABLOB_MAGIC)) {
		return BLOB;
	} else {
		return UNKNOWN;
	}
}

void get_fmt_name(format_t fmt, char *name) {
	const char *s;
	switch (fmt) {
		case CHROMEOS:
			s = "chromeos";
			break;
		case AOSP:
			s = "aosp";
			break;
		case GZIP:
			s = "gzip";
			break;
		case LZOP:
			s = "lzop";
			break;
		case XZ:
			s = "xz";
			break;
		case LZMA:
			s = "lzma";
			break;
		case BZIP2:
			s = "bzip2";
			break;
		case LZ4:
			s = "lz4";
			break;
		case LZ4_LEGACY:
			s = "lz4_legacy";
			break;
		case MTK:
			s = "mtk";
			break;
		case DTB:
			s = "dtb";
			break;
		default:
			s = "raw";
	}
	strcpy(name, s);
}
