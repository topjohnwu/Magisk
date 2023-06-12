use std::fs::File;
use std::io::{BufReader, Read, Seek, SeekFrom, Write};
use std::os::fd::{AsRawFd, FromRawFd};

use anyhow::{anyhow, Context};
use byteorder::{BigEndian, ReadBytesExt};
use protobuf::{EnumFull, Message};

use base::libc::c_char;
use base::{ptr_to_str_result, ReadSeekExt, StrErr};
use base::{ResultExt, WriteExt};

use crate::ffi;
use crate::update_metadata::install_operation::Type;
use crate::update_metadata::DeltaArchiveManifest;

macro_rules! bad_payload {
    ($msg:literal) => {
        anyhow!(concat!("invalid payload: ", $msg))
    };
    ($($args:tt)*) => {
        anyhow!("invalid payload: {}", format_args!($($args)*))
    };
}

const PAYLOAD_MAGIC: &str = "CrAU";

fn do_extract_boot_from_payload(
    in_path: &str,
    partition_name: Option<&str>,
    out_path: Option<&str>,
) -> anyhow::Result<()> {
    let mut reader = BufReader::new(if in_path == "-" {
        unsafe { File::from_raw_fd(0) }
    } else {
        File::open(in_path).with_context(|| format!("cannot open '{in_path}'"))?
    });

    let buf = &mut [0u8; 4];
    reader.read_exact(buf)?;

    if buf != PAYLOAD_MAGIC.as_bytes() {
        return Err(bad_payload!("invalid magic"));
    }

    let version = reader.read_u64::<BigEndian>()?;
    if version != 2 {
        return Err(bad_payload!("unsupported version: {}", version));
    }

    let manifest_len = reader.read_u64::<BigEndian>()? as usize;
    if manifest_len == 0 {
        return Err(bad_payload!("manifest length is zero"));
    }

    let manifest_sig_len = reader.read_u32::<BigEndian>()?;
    if manifest_sig_len == 0 {
        return Err(bad_payload!("manifest signature length is zero"));
    }

    let mut buf = Vec::new();
    buf.resize(manifest_len, 0u8);

    let manifest = {
        let manifest = &mut buf[..manifest_len];
        reader.read_exact(manifest)?;
        DeltaArchiveManifest::parse_from_bytes(manifest)?
    };
    if !manifest.has_minor_version() || manifest.minor_version() != 0 {
        return Err(bad_payload!(
            "delta payloads are not supported, please use a full payload file"
        ));
    }

    let block_size = manifest.block_size() as u64;

    let partition = match partition_name {
        None => {
            let boot = manifest
                .partitions
                .iter()
                .find(|p| p.partition_name() == "init_boot");
            let boot = match boot {
                Some(boot) => Some(boot),
                None => manifest
                    .partitions
                    .iter()
                    .find(|p| p.partition_name() == "boot"),
            };
            boot.ok_or(anyhow!("boot partition not found"))?
        }
        Some(name) => manifest
            .partitions
            .iter()
            .find(|p| p.partition_name() == name)
            .ok_or(anyhow!("partition '{name}' not found"))?,
    };

    let out_str: String;
    let out_path = match out_path {
        None => {
            out_str = format!("{}.img", partition.partition_name());
            out_str.as_str()
        }
        Some(s) => s,
    };

    let mut out_file =
        File::create(out_path).with_context(|| format!("cannot write to '{out_path}'"))?;

    // Skip the manifest signature
    reader.skip(manifest_sig_len as usize)?;

    // Sort the install operations with data_offset so we will only ever need to seek forward
    // This makes it possible to support non-seekable input file descriptors
    let mut operations = partition.operations.clone();
    operations.sort_by_key(|e| e.data_offset.unwrap_or(0));
    let mut curr_data_offset: u64 = 0;

    for operation in operations.iter() {
        let data_len = operation
            .data_length
            .ok_or(bad_payload!("data length not found"))? as usize;

        let data_offset = operation
            .data_offset
            .ok_or(bad_payload!("data offset not found"))?;

        let data_type = operation
            .type_
            .ok_or(bad_payload!("operation type not found"))?
            .enum_value()
            .map_err(|_| bad_payload!("operation type not valid"))?;

        buf.resize(data_len, 0u8);
        let data = &mut buf[..data_len];

        // Skip to the next offset and read data
        let skip = data_offset - curr_data_offset;
        reader.skip(skip as usize)?;
        reader.read_exact(data)?;
        curr_data_offset = data_offset + data_len as u64;

        let out_offset = operation
            .dst_extents
            .get(0)
            .ok_or(bad_payload!("dst extents not found"))?
            .start_block
            .ok_or(bad_payload!("start block not found"))?
            * block_size;

        match data_type {
            Type::REPLACE => {
                out_file.seek(SeekFrom::Start(out_offset))?;
                out_file.write_all(data)?;
            }
            Type::ZERO => {
                for ext in operation.dst_extents.iter() {
                    let out_seek = ext
                        .start_block
                        .ok_or(bad_payload!("start block not found"))?
                        * block_size;
                    let num_blocks = ext.num_blocks.ok_or(bad_payload!("num blocks not found"))?;
                    out_file.seek(SeekFrom::Start(out_seek))?;
                    out_file.write_zeros(num_blocks as usize)?;
                }
            }
            Type::REPLACE_BZ | Type::REPLACE_XZ => {
                out_file.seek(SeekFrom::Start(out_offset))?;
                if !ffi::decompress(data, out_file.as_raw_fd()) {
                    return Err(bad_payload!("decompression failed"));
                }
            }
            _ => {
                return Err(bad_payload!(
                    "unsupported operation type: {}",
                    data_type.descriptor().name()
                ));
            }
        };
    }

    Ok(())
}

pub fn extract_boot_from_payload(
    in_path: *const c_char,
    partition: *const c_char,
    out_path: *const c_char,
) -> bool {
    fn inner(
        in_path: *const c_char,
        partition: *const c_char,
        out_path: *const c_char,
    ) -> anyhow::Result<()> {
        let in_path = ptr_to_str_result(in_path)?;
        let partition = match ptr_to_str_result(partition) {
            Ok(s) => Some(s),
            Err(StrErr::NullPointerError) => None,
            Err(e) => Err(e)?,
        };
        let out_path = match ptr_to_str_result(out_path) {
            Ok(s) => Some(s),
            Err(StrErr::NullPointerError) => None,
            Err(e) => Err(e)?,
        };
        do_extract_boot_from_payload(in_path, partition, out_path)
            .context("Failed to extract from payload")?;
        Ok(())
    }
    inner(in_path, partition, out_path).log().is_ok()
}
