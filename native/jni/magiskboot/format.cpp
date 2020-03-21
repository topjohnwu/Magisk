#include <string.h>

#include "format.hpp"

std::map<std::string_view, format_t> name2fmt;
Fmt2Name fmt2name;
Fmt2Ext fmt2ext;

class FormatInit {
public:
	FormatInit() {
		name2fmt["gzip"] = GZIP;
		name2fmt["xz"] = XZ;
		name2fmt["lzma"] = LZMA;
		name2fmt["bzip2"] = BZIP2;
		name2fmt["lz4"] = LZ4;
		name2fmt["lz4_legacy"] = LZ4_LEGACY;
	}
};

static FormatInit init;

#define MATCH(s) (len >= (sizeof(s) - 1) && memcmp(buf, s, sizeof(s) - 1) == 0)

format_t check_fmt(const void *buf, size_t len) {
	if (MATCH(CHROMEOS_MAGIC)) {
		return CHROMEOS;
	} else if (MATCH(BOOT_MAGIC)) {
		return AOSP;
	} else if (MATCH(GZIP1_MAGIC) || MATCH(GZIP2_MAGIC)) {
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
	} else if (MATCH(LZ41_MAGIC) || MATCH(LZ42_MAGIC)) {
		return LZ4;
	} else if (MATCH(LZ4_LEG_MAGIC)) {
		return LZ4_LEGACY;
	} else if (MATCH(MTK_MAGIC)) {
		return MTK;
	} else if (MATCH(DHTB_MAGIC)) {
		return DHTB;
	} else if (MATCH(TEGRABLOB_MAGIC)) {
		return BLOB;
	} else {
		return UNKNOWN;
	}
}

const char *Fmt2Name::operator[](format_t fmt) {
	switch (fmt) {
		case CHROMEOS:
			return "chromeos";
		case AOSP:
			return "aosp";
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
		case MTK:
			return "mtk";
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
			return ".lz4";
		default:
			return "";
	}
}
