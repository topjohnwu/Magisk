#include <string.h>

#include "bootimg.h"
#include "format.h"

format_t check_fmt(const void *buf) {
	if (memcmp(buf, CHROMEOS_MAGIC, 8) == 0) {
		return CHROMEOS;
	} else if (memcmp(buf, BOOT_MAGIC, 8) == 0) {
		return AOSP;
	} else if (memcmp(buf, ELF32_MAGIC, 5) == 0) {
		return ELF32;
	} else if (memcmp(buf, ELF64_MAGIC, 5) == 0) {
		return ELF64;
	} else if (memcmp(buf, GZIP_MAGIC, 4) == 0) {
		return GZIP;
	} else if (memcmp(buf, LZOP_MAGIC, 9) == 0) {
		return LZOP;
	} else if (memcmp(buf, XZ_MAGIC, 6) == 0) {
		return XZ;
	} else if (memcmp(buf, "\x5d\x00\x00", 3) == 0
			&& (((char *)buf)[12] == '\xff' || ((char *)buf)[12] == '\x00')) {
		return LZMA;
	} else if (memcmp(buf, BZIP_MAGIC, 3) == 0) {
		return BZIP2;
	} else if (memcmp(buf, LZ4_MAGIC, 4) == 0) {
		return LZ4;
	} else if (memcmp(buf, LZ4_LEG_MAGIC, 4) == 0) {
		return LZ4_LEGACY;
	} else if (memcmp(buf, MTK_MAGIC, 4) == 0) {
		return MTK;
	} else if (memcmp(buf, DTB_MAGIC, 4) == 0) {
		return DTB;
	} else if (memcmp(buf, DHTB_MAGIC, 8) == 0) {
		return DHTB;
	} else if (memcmp(buf, TEGRABLOB_MAGIC, 20) == 0) {
		return BLOB;
	} else {
		return UNKNOWN;
	}
}

void get_fmt_name(format_t fmt, char *name) {
	char *s;
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
