use crate::ffi::{FileFormat, check_fmt};
use base::nix::fcntl::OFlag;
use base::{Chunker, FileOrStd, LoggedResult, ReadExt, Utf8CStr, Utf8CString, WriteExt, log_err};
use bzip2::Compression as BzCompression;
use bzip2::read::BzDecoder;
use bzip2::write::BzEncoder;
use flate2::Compression as GzCompression;
use flate2::read::MultiGzDecoder;
use flate2::write::GzEncoder;
use lz4::block::CompressionMode;
use lz4::liblz4::BlockChecksum;
use lz4::{
    BlockMode, BlockSize, ContentChecksum, Decoder as LZ4FrameDecoder, Encoder as LZ4FrameEncoder,
    EncoderBuilder as LZ4FrameEncoderBuilder,
};
use lzma_rust2::{CheckType, LzmaOptions, LzmaReader, LzmaWriter, XzOptions, XzReader, XzWriter};
use std::cmp::min;
use std::fmt::Write as FmtWrite;
use std::fs::File;
use std::io::{BufWriter, Cursor, Read, Write};
use std::mem::ManuallyDrop;
use std::num::NonZeroU64;
use std::ops::DerefMut;
use std::os::fd::{FromRawFd, RawFd};
use zopfli::{BlockType, GzipEncoder as ZopFliEncoder, Options as ZopfliOptions};

pub trait WriteFinish<W: Write>: Write {
    fn finish(self: Box<Self>) -> std::io::Result<W>;
}

// Boilerplate for existing types

macro_rules! finish_impl {
    ($($t:ty),*) => {$(
        impl<W: Write> WriteFinish<W> for $t {
            fn finish(self: Box<Self>) -> std::io::Result<W> {
                Self::finish(*self)
            }
        }
    )*}
}

finish_impl!(GzEncoder<W>, BzEncoder<W>, XzWriter<W>, LzmaWriter<W>);

impl<W: Write> WriteFinish<W> for BufWriter<ZopFliEncoder<W>> {
    fn finish(self: Box<Self>) -> std::io::Result<W> {
        let inner = self.into_inner()?;
        ZopFliEncoder::finish(inner)
    }
}

impl<W: Write> WriteFinish<W> for LZ4FrameEncoder<W> {
    fn finish(self: Box<Self>) -> std::io::Result<W> {
        let (w, r) = Self::finish(*self);
        r?;
        Ok(w)
    }
}

// LZ4BlockArchive format
//
// len:  |   4   |          4            |           n           | ... |           4             |
// data: | magic | compressed block size | compressed block data | ... | total uncompressed size |

// LZ4BlockEncoder

const LZ4_BLOCK_SIZE: usize = 0x800000;
const LZ4HC_CLEVEL_MAX: i32 = 12;
const LZ4_MAGIC: u32 = 0x184c2102;

struct LZ4BlockEncoder<W: Write> {
    write: W,
    chunker: Chunker,
    out_buf: Box<[u8]>,
    total: u32,
    is_lg: bool,
}

impl<W: Write> LZ4BlockEncoder<W> {
    fn new(write: W, is_lg: bool) -> Self {
        let out_sz = lz4::block::compress_bound(LZ4_BLOCK_SIZE).unwrap_or(LZ4_BLOCK_SIZE);
        LZ4BlockEncoder {
            write,
            chunker: Chunker::new(LZ4_BLOCK_SIZE),
            // SAFETY: all bytes will be initialized before it is used
            out_buf: unsafe { Box::new_uninit_slice(out_sz).assume_init() },
            total: 0,
            is_lg,
        }
    }

    fn encode_block(write: &mut W, out_buf: &mut [u8], chunk: &[u8]) -> std::io::Result<()> {
        let compressed_size = lz4::block::compress_to_buffer(
            chunk,
            Some(CompressionMode::HIGHCOMPRESSION(LZ4HC_CLEVEL_MAX)),
            false,
            out_buf,
        )?;
        let block_size = compressed_size as u32;
        write.write_pod(&block_size)?;
        write.write_all(&out_buf[..compressed_size])
    }
}

impl<W: Write> Write for LZ4BlockEncoder<W> {
    fn write(&mut self, buf: &[u8]) -> std::io::Result<usize> {
        self.write_all(buf)?;
        Ok(buf.len())
    }

    fn flush(&mut self) -> std::io::Result<()> {
        Ok(())
    }

    fn write_all(&mut self, mut buf: &[u8]) -> std::io::Result<()> {
        if self.total == 0 {
            // Write header
            self.write.write_pod(&LZ4_MAGIC)?;
        }

        self.total += buf.len() as u32;
        while !buf.is_empty() {
            let (b, chunk) = self.chunker.add_data(buf);
            buf = b;
            if let Some(chunk) = chunk {
                Self::encode_block(&mut self.write, &mut self.out_buf, chunk)?;
            }
        }
        Ok(())
    }
}

impl<W: Write> WriteFinish<W> for LZ4BlockEncoder<W> {
    fn finish(mut self: Box<Self>) -> std::io::Result<W> {
        let chunk = self.chunker.get_available();
        if !chunk.is_empty() {
            Self::encode_block(&mut self.write, &mut self.out_buf, chunk)?;
        }
        if self.is_lg {
            self.write.write_pod(&self.total)?;
        }
        Ok(self.write)
    }
}

// LZ4BlockDecoder

struct LZ4BlockDecoder<R: Read> {
    read: R,
    in_buf: Box<[u8]>,
    out_buf: Box<[u8]>,
    out_len: usize,
    out_pos: usize,
}

impl<R: Read> LZ4BlockDecoder<R> {
    fn new(read: R) -> Self {
        let compressed_sz = lz4::block::compress_bound(LZ4_BLOCK_SIZE).unwrap_or(LZ4_BLOCK_SIZE);
        Self {
            read,
            in_buf: unsafe { Box::new_uninit_slice(compressed_sz).assume_init() },
            out_buf: unsafe { Box::new_uninit_slice(LZ4_BLOCK_SIZE).assume_init() },
            out_len: 0,
            out_pos: 0,
        }
    }
}

impl<R: Read> Read for LZ4BlockDecoder<R> {
    fn read(&mut self, buf: &mut [u8]) -> std::io::Result<usize> {
        if self.out_pos == self.out_len {
            let mut block_size: u32 = 0;
            if let Err(e) = self.read.read_pod(&mut block_size) {
                return if e.kind() == std::io::ErrorKind::UnexpectedEof {
                    Ok(0)
                } else {
                    Err(e)
                };
            }
            if block_size == LZ4_MAGIC {
                self.read.read_pod(&mut block_size)?;
            }

            let block_size = block_size as usize;

            if block_size > self.in_buf.len() {
                // This may be the LG format trailer, EOF
                return Ok(0);
            }

            // Read the entire compressed block
            let compressed_block = &mut self.in_buf[..block_size];
            if let Ok(len) = self.read.read(compressed_block) {
                if len == 0 {
                    // We hit EOF, that's fine
                    return Ok(0);
                } else if len != block_size {
                    let remain = &mut compressed_block[len..];
                    self.read.read_exact(remain)?;
                }
            }

            self.out_len = lz4::block::decompress_to_buffer(
                compressed_block,
                Some(LZ4_BLOCK_SIZE as i32),
                &mut self.out_buf,
            )?;
            self.out_pos = 0;
        }
        let copy_len = min(buf.len(), self.out_len - self.out_pos);
        buf[..copy_len].copy_from_slice(&self.out_buf[self.out_pos..self.out_pos + copy_len]);
        self.out_pos += copy_len;
        Ok(copy_len)
    }
}

// Top-level APIs

pub fn get_encoder<'a, W: Write + 'a>(
    format: FileFormat,
    w: W,
) -> std::io::Result<Box<dyn WriteFinish<W> + 'a>> {
    Ok(match format {
        FileFormat::XZ => {
            let mut opt = XzOptions::with_preset(9);
            opt.set_check_sum_type(CheckType::Crc32);
            Box::new(XzWriter::new(w, opt)?)
        }
        FileFormat::LZMA => Box::new(LzmaWriter::new_use_header(
            w,
            &LzmaOptions::with_preset(9),
            None,
        )?),
        FileFormat::BZIP2 => Box::new(BzEncoder::new(w, BzCompression::best())),
        FileFormat::LZ4 => {
            let encoder = LZ4FrameEncoderBuilder::new()
                .block_size(BlockSize::Max4MB)
                .block_mode(BlockMode::Independent)
                .checksum(ContentChecksum::ChecksumEnabled)
                .block_checksum(BlockChecksum::BlockChecksumEnabled)
                .level(9)
                .auto_flush(true)
                .build(w)?;
            Box::new(encoder)
        }
        FileFormat::LZ4_LEGACY => Box::new(LZ4BlockEncoder::new(w, false)),
        FileFormat::LZ4_LG => Box::new(LZ4BlockEncoder::new(w, true)),
        FileFormat::ZOPFLI => {
            // These options are already better than gzip -9
            let opt = ZopfliOptions {
                iteration_count: unsafe { NonZeroU64::new_unchecked(1) },
                maximum_block_splits: 1,
                ..Default::default()
            };
            Box::new(ZopFliEncoder::new_buffered(opt, BlockType::Dynamic, w)?)
        }
        FileFormat::GZIP => Box::new(GzEncoder::new(w, GzCompression::best())),
        _ => unreachable!(),
    })
}

pub fn get_decoder<'a, R: Read + 'a>(
    format: FileFormat,
    r: R,
) -> std::io::Result<Box<dyn Read + 'a>> {
    Ok(match format {
        FileFormat::XZ => Box::new(XzReader::new(r, true)),
        FileFormat::LZMA => Box::new(LzmaReader::new_mem_limit(r, u32::MAX, None)?),
        FileFormat::BZIP2 => Box::new(BzDecoder::new(r)),
        FileFormat::LZ4 => Box::new(LZ4FrameDecoder::new(r)?),
        FileFormat::LZ4_LG | FileFormat::LZ4_LEGACY => Box::new(LZ4BlockDecoder::new(r)),
        FileFormat::ZOPFLI | FileFormat::GZIP => Box::new(MultiGzDecoder::new(r)),
        _ => unreachable!(),
    })
}

// C++ FFI

pub fn compress_bytes(format: FileFormat, in_bytes: &[u8], out_fd: RawFd) {
    let mut out_file = unsafe { ManuallyDrop::new(File::from_raw_fd(out_fd)) };

    let _: LoggedResult<()> = try {
        let mut encoder = get_encoder(format, out_file.deref_mut())?;
        std::io::copy(&mut Cursor::new(in_bytes), encoder.deref_mut())?;
        encoder.finish()?;
    };
}

pub fn decompress_bytes(format: FileFormat, in_bytes: &[u8], out_fd: RawFd) {
    let mut out_file = unsafe { ManuallyDrop::new(File::from_raw_fd(out_fd)) };

    let _: LoggedResult<()> = try {
        let mut decoder = get_decoder(format, in_bytes)?;
        std::io::copy(decoder.as_mut(), out_file.deref_mut())?;
    };
}

// Command-line entry points

pub(crate) fn decompress_cmd(infile: &Utf8CStr, outfile: Option<&Utf8CStr>) -> LoggedResult<()> {
    let in_std = infile == "-";
    let mut rm_in = false;

    let mut buf = [0u8; 4096];

    let input = if in_std {
        FileOrStd::StdIn
    } else {
        FileOrStd::File(infile.open(OFlag::O_RDONLY)?)
    };

    // First read some bytes for format detection
    let len = input.as_file().read(&mut buf)?;
    let buf = &buf[..len];

    let format = check_fmt(buf);

    eprintln!("Detected format: {format}");

    if !format.is_compressed() {
        return log_err!("Input file is not a supported type!");
    }

    // If user did not provide outfile, infile has to be either
    // <path>.[ext], or "-". Outfile will be either <path> or "-".
    // If the input does not have proper format, abort.

    let output = if let Some(outfile) = outfile {
        if outfile == "-" {
            FileOrStd::StdOut
        } else {
            FileOrStd::File(outfile.create(OFlag::O_WRONLY | OFlag::O_TRUNC, 0o644)?)
        }
    } else if in_std {
        FileOrStd::StdOut
    } else {
        // Strip out extension and remove input
        let outfile = if let Some((outfile, ext)) = infile.rsplit_once('.')
            && ext == format.ext()
        {
            Utf8CString::from(outfile)
        } else {
            return log_err!("Input file is not a supported type!");
        };

        rm_in = true;
        eprintln!("Decompressing to [{outfile}]");
        FileOrStd::File(outfile.create(OFlag::O_WRONLY | OFlag::O_TRUNC, 0o644)?)
    };

    let mut decoder = get_decoder(format, Cursor::new(buf).chain(input.as_file()))?;
    std::io::copy(decoder.as_mut(), &mut output.as_file())?;

    if rm_in {
        infile.remove()?;
    }

    Ok(())
}

pub(crate) fn compress_cmd(
    method: FileFormat,
    infile: &Utf8CStr,
    outfile: Option<&Utf8CStr>,
) -> LoggedResult<()> {
    let in_std = infile == "-";
    let mut rm_in = false;

    let input = if in_std {
        FileOrStd::StdIn
    } else {
        FileOrStd::File(infile.open(OFlag::O_RDONLY)?)
    };

    let output = if let Some(outfile) = outfile {
        if outfile == "-" {
            FileOrStd::StdOut
        } else {
            FileOrStd::File(outfile.create(OFlag::O_WRONLY | OFlag::O_TRUNC, 0o644)?)
        }
    } else if in_std {
        FileOrStd::StdOut
    } else {
        let mut outfile = Utf8CString::default();
        outfile.write_str(infile).ok();
        outfile.write_char('.').ok();
        outfile.write_str(method.ext()).ok();
        eprintln!("Compressing to [{outfile}]");
        rm_in = true;
        let outfile = outfile.create(OFlag::O_WRONLY | OFlag::O_TRUNC, 0o644)?;
        FileOrStd::File(outfile)
    };

    let mut encoder = get_encoder(method, output.as_file())?;
    std::io::copy(&mut input.as_file(), encoder.as_mut())?;
    encoder.finish()?;

    if rm_in {
        infile.remove()?;
    }
    Ok(())
}
