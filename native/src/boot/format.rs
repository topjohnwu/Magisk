use crate::ffi::FileFormat;
use base::{Utf8CStr, cstr, libc};
use std::fmt::{Display, Formatter};
use std::str::FromStr;

impl FromStr for FileFormat {
    type Err = ();

    fn from_str(s: &str) -> Result<Self, Self::Err> {
        match s {
            "gzip" => Ok(Self::GZIP),
            "zopfli" => Ok(Self::ZOPFLI),
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
            Self::ZOPFLI => cstr!("zopfli"),
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
            Self::GZIP | Self::ZOPFLI => "gz",
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
                | Self::ZOPFLI
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
            Self::ZOPFLI,
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
