use crate::ffi::FileFormat;
use base::{Utf8CStr, cstr, libc};
use std::fmt::{Display, Formatter};
use std::str::FromStr;

impl FromStr for FileFormat {
    type Err = ();

    fn from_str(s: &str) -> Result<Self, Self::Err> {
        match s {
            "gzip" => Ok(Self::GZIP),
            "xz" => Ok(Self::XZ),
            "lzma" => Ok(Self::LZMA),
            "bzip2" => Ok(Self::BZIP2),
            "lz4" => Ok(Self::LZ4),
            "lz4_legacy" => Ok(Self::LZ4_LEGACY),
            "lz4_lg" => Ok(Self::LZ4_LG),
            _ => Err(()),
        }
    }
}

impl Display for FileFormat {
    fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
        f.write_str(self.as_cstr())
    }
}

impl FileFormat {
    fn as_cstr(&self) -> &'static Utf8CStr {
        match *self {
            Self::GZIP => cstr!("gzip"),
            Self::LZOP => cstr!("lzop"),
            Self::XZ => cstr!("xz"),
            Self::LZMA => cstr!("lzma"),
            Self::BZIP2 => cstr!("bzip2"),
            Self::LZ4 => cstr!("lz4"),
            Self::LZ4_LEGACY => cstr!("lz4_legacy"),
            Self::LZ4_LG => cstr!("lz4_lg"),
            Self::DTB => cstr!("dtb"),
            Self::ZIMAGE => cstr!("zimage"),
            _ => cstr!("raw"),
        }
    }
}

impl FileFormat {
    pub fn ext(&self) -> &'static str {
        match *self {
            Self::GZIP => "gz",
            Self::LZOP => "lzo",
            Self::XZ => "xz",
            Self::LZMA => "lzma",
            Self::BZIP2 => "bz2",
            Self::LZ4 | Self::LZ4_LEGACY | Self::LZ4_LG => "lz4",
            _ => "",
        }
    }

    pub fn is_compressed(&self) -> bool {
        matches!(
            *self,
            Self::GZIP
                | Self::XZ
                | Self::LZMA
                | Self::BZIP2
                | Self::LZ4
                | Self::LZ4_LEGACY
                | Self::LZ4_LG
        )
    }

    pub fn formats() -> String {
        [
            Self::GZIP,
            Self::XZ,
            Self::LZMA,
            Self::BZIP2,
            Self::LZ4,
            Self::LZ4_LEGACY,
            Self::LZ4_LG,
        ]
        .map(|f| f.to_string())
        .join(" ")
    }
}

// C++ FFI

pub fn fmt2name(fmt: FileFormat) -> *const libc::c_char {
    fmt.as_cstr().as_ptr()
}

pub fn fmt_compressed(fmt: FileFormat) -> bool {
    fmt.is_compressed()
}

pub fn fmt_compressed_any(fmt: FileFormat) -> bool {
    fmt.is_compressed() || matches!(fmt, FileFormat::LZOP)
}

const CHROMEOS_MAGIC: &[u8] = b"CHROMEOS";
const BOOT_MAGIC: &[u8] = b"ANDROID!";
const VENDOR_BOOT_MAGIC: &[u8] = b"VNDRBOOT";
const GZIP2_MAGIC: &[u8] = b"\x1f\x9e";
const LZOP_MAGIC: &[u8] = b"\x89LZO\x00\x0d\x0a\x1a\x0a";
const XZ_MAGIC: &[u8] = b"\xfd7zXZ\x00";
const BZIP_MAGIC: &[u8] = b"BZh";
const LZ41_MAGIC: &[u8] = b"\x03\x21\x4c\x18";
const LZ42_MAGIC: &[u8] = b"\x04\x22\x4d\x18";
const LZ4_LEG_MAGIC: &[u8] = b"\x02\x21\x4c\x18";
const MTK_MAGIC: &[u8] = b"\x88\x16\x88\x58";
const DTB_MAGIC: &[u8] = b"\xd0\x0d\xfe\xed";
const DHTB_MAGIC: &[u8] = b"\x44\x48\x54\x42\x01\x00\x00\x00";
const TEGRABLOB_MAGIC: &[u8] = b"-SIGNED-BY-SIGNBLOB-";
const ZIMAGE_MAGIC: &[u8] = b"\x18\x28\x6f\x01";

fn crc32_ieee(buf: &[u8]) -> u32 {
    let mut crc = 0xffff_ffffu32;
    for &byte in buf {
        crc ^= byte as u32;
        for _ in 0..8 {
            crc = if crc & 1 != 0 {
                (crc >> 1) ^ 0xedb8_8320
            } else {
                crc >> 1
            };
        }
    }
    !crc
}

fn check_xz(buf: &[u8]) -> bool {
    // 0..6 : Header magic: 0xFD, '7', 'z', 'X', 'Z', 0x00
    // 6..8 : Stream Flags (byte 0: reserved 0x00, byte 1: check type 0/1/4/10)
    // 8..12: CRC32 of stream flags, little endian
    if buf.len() < 12 || !buf.starts_with(XZ_MAGIC) {
        return false;
    }
    if buf[6] != 0x00 || (buf[7] & 0xf0) != 0 {
        return false;
    }
    let check_id = buf[7] & 0x0f;
    if !matches!(check_id, 0 | 1 | 4 | 10) {
        return false;
    }
    let Ok(crc_bytes) = buf[8..12].try_into() else {
        return false;
    };
    crc32_ieee(&buf[6..8]) == u32::from_le_bytes(crc_bytes)
}

fn check_gzip(buf: &[u8]) -> bool {
    // RFC 1952: ID1=0x1f, ID2=0x8b, CM=8 (deflate), FLG bits 5-7 reserved (0)
    if buf.len() >= 10 && buf[0] == 0x1f && buf[1] == 0x8b && buf[2] == 8 && (buf[3] & 0xe0) == 0 {
        return true;
    }
    buf.starts_with(GZIP2_MAGIC)
}

fn check_bzip2(buf: &[u8]) -> bool {
    // BZIP2 stream: 'B','Z','h' + '1'..='9' + compressed block magic '\x31\x41\x59\x26\x53\x59'
    buf.len() >= 10
        && buf.starts_with(BZIP_MAGIC)
        && (b'1'..=b'9').contains(&buf[3])
        && &buf[4..10] == b"\x31\x41\x59\x26\x53\x59"
}

fn guess_lzma(buf: &[u8]) -> bool {
    // 0     : (pb * 5 + lp) * 9 + lc
    // 1 - 4 : dict size, must be 2^n
    // 5 - 12: all 0xFF
    if buf.len() <= 13 || buf[0] != 0x5d {
        return false;
    }
    let dict_sz = match buf[1..5].try_into() {
        Ok(bytes) => u32::from_le_bytes(bytes),
        Err(_) => return false,
    };
    if dict_sz == 0 || (dict_sz & (dict_sz - 1)) != 0 {
        return false;
    }
    buf[5..13] == [0xff; 8]
}

pub fn check_fmt(buf: &[u8]) -> FileFormat {
    if buf.starts_with(CHROMEOS_MAGIC) {
        FileFormat::CHROMEOS
    } else if buf.starts_with(BOOT_MAGIC) {
        FileFormat::AOSP
    } else if buf.starts_with(VENDOR_BOOT_MAGIC) {
        FileFormat::AOSP_VENDOR
    } else if check_gzip(buf) {
        FileFormat::GZIP
    } else if buf.starts_with(LZOP_MAGIC) {
        FileFormat::LZOP
    } else if check_xz(buf) {
        FileFormat::XZ
    } else if guess_lzma(buf) {
        FileFormat::LZMA
    } else if check_bzip2(buf) {
        FileFormat::BZIP2
    } else if buf.starts_with(LZ41_MAGIC) || buf.starts_with(LZ42_MAGIC) {
        FileFormat::LZ4
    } else if buf.starts_with(LZ4_LEG_MAGIC) {
        FileFormat::LZ4_LEGACY
    } else if buf.starts_with(MTK_MAGIC) {
        FileFormat::MTK
    } else if buf.starts_with(DTB_MAGIC) {
        FileFormat::DTB
    } else if buf.starts_with(DHTB_MAGIC) {
        FileFormat::DHTB
    } else if buf.starts_with(TEGRABLOB_MAGIC) {
        FileFormat::BLOB
    } else if buf.len() >= 0x28 && &buf[0x24..0x28] == ZIMAGE_MAGIC {
        FileFormat::ZIMAGE
    } else {
        FileFormat::UNKNOWN
    }
}
