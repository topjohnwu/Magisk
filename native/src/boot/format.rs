use crate::ffi::FileFormat;
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
        match *self {
            Self::GZIP => write!(f, "gzip"),
            Self::ZOPFLI => write!(f, "zopfli"),
            Self::LZOP => write!(f, "lzop"),
            Self::XZ => write!(f, "xz"),
            Self::LZMA => write!(f, "lzma"),
            Self::BZIP2 => write!(f, "bzip2"),
            Self::LZ4 => write!(f, "lz4"),
            Self::LZ4_LEGACY => write!(f, "lz4_legacy"),
            Self::LZ4_LG => write!(f, "lz4_lg"),
            Self::DTB => write!(f, "dtb"),
            Self::ZIMAGE => write!(f, "zimage"),
            _ => write!(f, "raw"),
        }
    }
}

impl FileFormat {
    pub fn ext(&self) -> &'static str {
        match *self {
            Self::GZIP | Self::ZOPFLI => ".gz",
            Self::LZOP => ".lzo",
            Self::XZ => ".xz",
            Self::LZMA => ".lzma",
            Self::BZIP2 => ".bz2",
            Self::LZ4 | Self::LZ4_LEGACY | Self::LZ4_LG => ".lz4",
            _ => "",
        }
    }

    pub fn is_compressed(&self) -> bool {
        matches!(
            *self,
            Self::GZIP
                | Self::ZOPFLI
                | Self::LZOP
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
            Self::LZOP,
        ]
        .map(|f| f.to_string())
        .join(" ")
    }
}
