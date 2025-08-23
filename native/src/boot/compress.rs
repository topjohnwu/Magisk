use crate::ffi::{FileFormat, check_fmt};
use base::libc::{O_RDONLY, O_TRUNC, O_WRONLY};
use base::{Chunker, FileOrStd, LoggedResult, Utf8CStr, Utf8CString, WriteExt, error, log_err};
use bytemuck::bytes_of_mut;
use bzip2::{Compression as BzCompression, write::BzDecoder, write::BzEncoder};
use flate2::{Compression as GzCompression, write::GzEncoder, write::MultiGzDecoder};
use lz4::{
    BlockMode, BlockSize, ContentChecksum, Encoder as LZ4FrameEncoder,
    EncoderBuilder as LZ4FrameEncoderBuilder, block::CompressionMode, liblz4::BlockChecksum,
};
use std::cell::Cell;
use std::fmt::Write as FmtWrite;
use std::fs::File;
use std::io::{BufWriter, Read, Write};
use std::mem::ManuallyDrop;
use std::num::NonZeroU64;
use std::ops::DerefMut;
use std::os::fd::{FromRawFd, RawFd};
use xz2::{
    stream::{Check as LzmaCheck, Filters as LzmaFilters, LzmaOptions, Stream as LzmaStream},
    write::{XzDecoder, XzEncoder},
};
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

finish_impl!(GzEncoder<W>, MultiGzDecoder<W>, BzEncoder<W>, XzEncoder<W>);

macro_rules! finish_impl_ref {
    ($($t:ty),*) => {$(
        impl<W: Write> WriteFinish<W> for $t {
            fn finish(mut self: Box<Self>) -> std::io::Result<W> {
                Self::finish(self.as_mut())
            }
        }
    )*}
}

finish_impl_ref!(BzDecoder<W>, XzDecoder<W>);

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

// Adapt Reader to Writer

// In case some decoders don't support the Write trait, instead of pushing data into the
// decoder, we have no choice but to pull data out of it. So first, we create a "fake" reader
// that does not own any data as a placeholder. In the Writer adapter struct, when data
// is fed in, we call FakeReader::set_data to forward this data as the "source" of the
// decoder. Next, we pull data out of the decoder, and finally, forward the decoded data to output.

struct FakeReader(Cell<&'static [u8]>);

impl FakeReader {
    fn new() -> FakeReader {
        FakeReader(Cell::new(&[]))
    }

    // SAFETY: the lifetime of the buffer is between the invocation of
    // this method and the invocation of FakeReader::clear. There is currently
    // no way to represent this with Rust's lifetime marker, so we transmute all
    // lifetimes away and make the users of this struct manually manage the lifetime.
    // It is the responsibility of the caller to ensure the underlying reference does not
    // live longer than it should.
    unsafe fn set_data(&self, data: &[u8]) {
        let buf: &'static [u8] = unsafe { std::mem::transmute(data) };
        self.0.set(buf)
    }

    fn clear(&self) {
        self.0.set(&[])
    }
}

impl Read for FakeReader {
    fn read(&mut self, buf: &mut [u8]) -> std::io::Result<usize> {
        let data = self.0.get();
        let len = std::cmp::min(buf.len(), data.len());
        buf[..len].copy_from_slice(&data[..len]);
        self.0.set(&data[len..]);
        Ok(len)
    }
}

// LZ4FrameDecoder

struct LZ4FrameDecoder<W: Write> {
    write: W,
    decoder: lz4::Decoder<FakeReader>,
}

impl<W: Write> LZ4FrameDecoder<W> {
    fn new(write: W) -> Self {
        let fake = FakeReader::new();
        let decoder = lz4::Decoder::new(fake).unwrap();
        LZ4FrameDecoder { write, decoder }
    }
}

impl<W: Write> Write for LZ4FrameDecoder<W> {
    fn write(&mut self, buf: &[u8]) -> std::io::Result<usize> {
        self.write_all(buf)?;
        Ok(buf.len())
    }

    fn flush(&mut self) -> std::io::Result<()> {
        Ok(())
    }

    fn write_all(&mut self, buf: &[u8]) -> std::io::Result<()> {
        // SAFETY: buf is removed from the reader immediately after usage
        unsafe { self.decoder.reader().set_data(buf) };
        std::io::copy(&mut self.decoder, &mut self.write)?;
        self.decoder.reader().clear();
        Ok(())
    }
}

impl<W: Write> WriteFinish<W> for LZ4FrameDecoder<W> {
    fn finish(self: Box<Self>) -> std::io::Result<W> {
        let (_, r) = self.decoder.finish();
        r?;
        Ok(self.write)
    }
}

// LZ4BlockArchive format
//
// len:  |   4   |          4            |           n           | ... |           4             |
// data: | magic | compressed block size | compressed block data | ... | total uncompressed size |

// LZ4BlockEncoder

const LZ4_BLOCK_SIZE: usize = 0x800000;
const LZ4HC_CLEVEL_MAX: i32 = 12;
const LZ4_MAGIC: &[u8] = b"\x02\x21\x4c\x18";

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
            self.write.write_all(LZ4_MAGIC)?;
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

struct LZ4BlockDecoder<W: Write> {
    write: W,
    chunker: Chunker,
    out_buf: Box<[u8]>,
    curr_block_size: usize,
}

impl<W: Write> LZ4BlockDecoder<W> {
    fn new(write: W) -> Self {
        LZ4BlockDecoder {
            write,
            chunker: Chunker::new(size_of::<u32>()),
            // SAFETY: all bytes will be initialized before it is used
            out_buf: unsafe { Box::new_uninit_slice(LZ4_BLOCK_SIZE).assume_init() },
            curr_block_size: 0,
        }
    }

    fn decode_block(write: &mut W, out_buf: &mut [u8], chunk: &[u8]) -> std::io::Result<()> {
        let decompressed_size =
            lz4::block::decompress_to_buffer(chunk, Some(LZ4_BLOCK_SIZE as i32), out_buf)?;
        write.write_all(&out_buf[..decompressed_size])
    }
}

impl<W: Write> Write for LZ4BlockDecoder<W> {
    fn write(&mut self, buf: &[u8]) -> std::io::Result<usize> {
        self.write_all(buf)?;
        Ok(buf.len())
    }

    fn flush(&mut self) -> std::io::Result<()> {
        Ok(())
    }

    fn write_all(&mut self, mut buf: &[u8]) -> std::io::Result<()> {
        while !buf.is_empty() {
            let (b, chunk) = self.chunker.add_data(buf);
            buf = b;
            if let Some(chunk) = chunk {
                if chunk == LZ4_MAGIC {
                    // Skip magic, read next u32
                    continue;
                }
                if self.curr_block_size == 0 {
                    // We haven't got the current block size yet, try read it
                    let mut next_u32: u32 = 0;
                    bytes_of_mut(&mut next_u32).copy_from_slice(chunk);

                    if next_u32 > lz4::block::compress_bound(LZ4_BLOCK_SIZE)? as u32 {
                        // This is the LG format trailer, EOF
                        continue;
                    }

                    // Update chunker to read next block
                    self.curr_block_size = next_u32 as usize;
                    self.chunker.set_chunk_size(self.curr_block_size);
                    continue;
                }

                // Actually decode the block
                Self::decode_block(&mut self.write, &mut self.out_buf, chunk)?;

                // Reset for the next block
                self.curr_block_size = 0;
                self.chunker.set_chunk_size(size_of::<u32>());
            }
        }
        Ok(())
    }
}

impl<W: Write> WriteFinish<W> for LZ4BlockDecoder<W> {
    fn finish(mut self: Box<Self>) -> std::io::Result<W> {
        let chunk = self.chunker.get_available();
        if !chunk.is_empty() {
            return Err(std::io::Error::new(
                std::io::ErrorKind::Interrupted,
                "Finish ran before end of compressed stream",
            ));
        }
        Ok(self.write)
    }
}

// Top-level APIs

pub fn get_encoder<'a, W: Write + 'a>(format: FileFormat, w: W) -> Box<dyn WriteFinish<W> + 'a> {
    match format {
        FileFormat::XZ => {
            let opt = LzmaOptions::new_preset(9).unwrap();
            let stream =
                LzmaStream::new_stream_encoder(LzmaFilters::new().lzma2(&opt), LzmaCheck::Crc32)
                    .unwrap();
            Box::new(XzEncoder::new_stream(w, stream))
        }
        FileFormat::LZMA => {
            let opt = LzmaOptions::new_preset(9).unwrap();
            let stream = LzmaStream::new_lzma_encoder(&opt).unwrap();
            Box::new(XzEncoder::new_stream(w, stream))
        }
        FileFormat::BZIP2 => Box::new(BzEncoder::new(w, BzCompression::best())),
        FileFormat::LZ4 => {
            let encoder = LZ4FrameEncoderBuilder::new()
                .block_size(BlockSize::Max4MB)
                .block_mode(BlockMode::Independent)
                .checksum(ContentChecksum::ChecksumEnabled)
                .block_checksum(BlockChecksum::BlockChecksumEnabled)
                .level(9)
                .auto_flush(true)
                .build(w)
                .unwrap();
            Box::new(encoder)
        }
        FileFormat::LZ4_LEGACY => Box::new(LZ4BlockEncoder::new(w, false)),
        FileFormat::LZ4_LG => Box::new(LZ4BlockEncoder::new(w, true)),
        FileFormat::ZOPFLI => {
            // These options are already better than gzip -9
            let opt = ZopfliOptions {
                iteration_count: NonZeroU64::new(1).unwrap(),
                maximum_block_splits: 1,
                ..Default::default()
            };
            Box::new(ZopFliEncoder::new_buffered(opt, BlockType::Dynamic, w).unwrap())
        }
        FileFormat::GZIP => Box::new(GzEncoder::new(w, GzCompression::best())),
        _ => unreachable!(),
    }
}

pub fn get_decoder<'a, W: Write + 'a>(format: FileFormat, w: W) -> Box<dyn WriteFinish<W> + 'a> {
    match format {
        FileFormat::XZ | FileFormat::LZMA => {
            let stream = LzmaStream::new_auto_decoder(u64::MAX, 0).unwrap();
            Box::new(XzDecoder::new_stream(w, stream))
        }
        FileFormat::BZIP2 => Box::new(BzDecoder::new(w)),
        FileFormat::LZ4 => Box::new(LZ4FrameDecoder::new(w)),
        FileFormat::LZ4_LG | FileFormat::LZ4_LEGACY => Box::new(LZ4BlockDecoder::new(w)),
        FileFormat::ZOPFLI | FileFormat::GZIP => Box::new(MultiGzDecoder::new(w)),
        _ => unreachable!(),
    }
}

// C++ FFI

pub fn compress_bytes(format: FileFormat, in_bytes: &[u8], out_fd: RawFd) {
    let mut out_file = unsafe { ManuallyDrop::new(File::from_raw_fd(out_fd)) };

    let mut encoder = get_encoder(format, out_file.deref_mut());
    let _: LoggedResult<()> = try {
        encoder.write_all(in_bytes)?;
        encoder.finish()?;
    };
}

pub fn decompress_bytes(format: FileFormat, in_bytes: &[u8], out_fd: RawFd) {
    let mut out_file = unsafe { ManuallyDrop::new(File::from_raw_fd(out_fd)) };

    let mut decoder = get_decoder(format, out_file.deref_mut());
    let _: LoggedResult<()> = try {
        decoder.write_all(in_bytes)?;
        decoder.finish()?;
    };
}

// Command-line entry points

pub(crate) fn decompress_cmd(
    infile: &mut String,
    outfile: Option<&mut String>,
) -> LoggedResult<()> {
    let infile = Utf8CStr::from_string(infile);
    let outfile = outfile.map(Utf8CStr::from_string);

    let in_std = infile == "-";
    let mut rm_in = false;

    let mut buf = [0u8; 4096];

    let input = if in_std {
        FileOrStd::StdIn
    } else {
        FileOrStd::File(infile.open(O_RDONLY)?)
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
            FileOrStd::File(outfile.create(O_WRONLY | O_TRUNC, 0o644)?)
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
        FileOrStd::File(outfile.create(O_WRONLY | O_TRUNC, 0o644)?)
    };

    let mut decoder = get_decoder(format, output.as_file());
    decoder.write_all(buf)?;
    std::io::copy(&mut input.as_file(), decoder.as_mut())?;
    decoder.finish()?;

    if rm_in {
        infile.remove()?;
    }

    Ok(())
}

pub(crate) fn compress_cmd(
    method: FileFormat,
    infile: &mut String,
    outfile: Option<&mut String>,
) -> LoggedResult<()> {
    if method == FileFormat::UNKNOWN {
        error!("Unsupported compression format");
    }

    let infile = Utf8CStr::from_string(infile);
    let outfile = outfile.map(Utf8CStr::from_string);

    let in_std = infile == "-";
    let mut rm_in = false;

    let input = if in_std {
        FileOrStd::StdIn
    } else {
        FileOrStd::File(infile.open(O_RDONLY)?)
    };

    let output = if let Some(outfile) = outfile {
        if outfile == "-" {
            FileOrStd::StdOut
        } else {
            FileOrStd::File(outfile.create(O_WRONLY | O_TRUNC, 0o644)?)
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
        let outfile = outfile.create(O_WRONLY | O_TRUNC, 0o644)?;
        FileOrStd::File(outfile)
    };

    let mut encoder = get_encoder(method, output.as_file());
    std::io::copy(&mut input.as_file(), encoder.as_mut())?;
    encoder.finish()?;

    if rm_in {
        infile.remove()?;
    }
    Ok(())
}
