use std::fs::File;
use std::io;
use std::io::{BufReader, ErrorKind, Read, Seek, SeekFrom, Write};
use std::os::fd::AsRawFd;

use byteorder::{BigEndian, ReadBytesExt};
use protobuf::{EnumFull, Message};

use base::libc::c_char;
use base::ptr_to_str;
use base::{ResultExt, WriteExt};

use crate::ffi;
use crate::update_metadata::install_operation::Type;
use crate::update_metadata::DeltaArchiveManifest;

macro_rules! data_err {
    ($fmt:expr) => { io::Error::new(ErrorKind::InvalidData, format!($fmt)) };
    ($fmt:expr, $($args:tt)*) => {
        io::Error::new(ErrorKind::InvalidData, format!($fmt, $($args)*))
    };
}

static PAYLOAD_MAGIC: &str = "CrAU";

fn do_extract_boot_from_payload(in_path: &str, out_path: &str) -> io::Result<()> {
    let mut reader = BufReader::new(File::open(in_path)?);

    let buf = &mut [0u8; 4];
    reader.read_exact(buf)?;

    if buf != PAYLOAD_MAGIC.as_bytes() {
        return Err(data_err!("invalid payload magic"));
    }

    let version = reader.read_u64::<BigEndian>()?;
    if version != 2 {
        return Err(data_err!("unsupported version {}", version));
    }

    let manifest_len = reader.read_u64::<BigEndian>()?;
    if manifest_len == 0 {
        return Err(data_err!("manifest length is zero"));
    }

    let manifest_sig_len = reader.read_u32::<BigEndian>()?;
    if manifest_sig_len == 0 {
        return Err(data_err!("manifest signature length is zero"));
    }

    let mut buf = vec![0; manifest_len as usize];

    reader.read_exact(&mut buf)?;

    let manifest = DeltaArchiveManifest::parse_from_bytes(&buf)?;
    if !manifest.has_minor_version() || manifest.minor_version() != 0 {
        return Err(data_err!(
            "delta payloads are not supported, please use a full payload file"
        ));
    }
    if !manifest.has_block_size() {
        return Err(data_err!("block size not found"));
    }

    let boot = manifest.partitions.iter().find(|partition| {
        partition.has_partition_name() && partition.partition_name() == "init_boot"
    });
    let boot = match boot {
        Some(boot) => Some(boot),
        None => manifest.partitions.iter().find(|partition| {
            partition.has_partition_name() && partition.partition_name() == "boot"
        }),
    };
    let boot = boot.ok_or(data_err!("boot partition not found"))?;

    let base_offset = reader.stream_position()? + manifest_sig_len as u64;

    let block_size = manifest
        .block_size
        .ok_or(data_err!("block size not found"))? as u64;

    let mut out_file = File::create(out_path)?;

    for operation in boot.operations.iter() {
        let data_len = operation
            .data_length
            .ok_or(data_err!("data length not found"))?;

        let data_offset = operation
            .data_offset
            .ok_or(data_err!("data offset not found"))?;

        let data_type = operation
            .type_
            .ok_or(data_err!("operation type not found"))?;

        let data_type = data_type
            .enum_value()
            .map_err(|_| data_err!("operation type not valid"))?;

        let mut buf = vec![0; data_len as usize];

        reader.seek(SeekFrom::Start(base_offset + data_offset))?;
        reader.read_exact(&mut buf)?;

        let out_offset = operation
            .dst_extents
            .get(0)
            .ok_or(data_err!("dst extents not found"))?
            .start_block
            .ok_or(data_err!("start block not found"))?
            * block_size;

        match data_type {
            Type::REPLACE => {
                out_file.seek(SeekFrom::Start(out_offset))?;
                out_file.write_all(&buf)?;
            }
            Type::ZERO => {
                for ext in operation.dst_extents.iter() {
                    let out_seek =
                        ext.start_block.ok_or(data_err!("start block not found"))? * block_size;
                    let num_blocks = ext.num_blocks.ok_or(data_err!("num blocks not found"))?;
                    out_file.seek(SeekFrom::Start(out_seek))?;
                    out_file.write_zeros(num_blocks as usize)?;
                }
            }
            Type::REPLACE_BZ | Type::REPLACE_XZ => {
                out_file.seek(SeekFrom::Start(out_offset))?;
                unsafe {
                    if !ffi::decompress(buf.as_ptr(), buf.len() as u64, out_file.as_raw_fd()) {
                        return Err(data_err!("decompression failed"));
                    }
                }
            }
            _ => {
                return Err(data_err!(
                    "unsupported operation type: {}",
                    data_type.descriptor().name()
                ));
            }
        };
    }

    Ok(())
}

pub fn extract_boot_from_payload(in_path: *const c_char, out_path: *const c_char) -> bool {
    let in_path = ptr_to_str(in_path);
    let out_path = ptr_to_str(out_path);
    do_extract_boot_from_payload(in_path, out_path)
        .msg_on_error(format_args!("Failed to extract boot from payload"))
        .is_ok()
}
