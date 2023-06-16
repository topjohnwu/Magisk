use std::fs::File;
use std::io;
use std::io::{Read, Seek, SeekFrom};
use std::os::fd::{FromRawFd, RawFd};

use base::*;

const EOCD_MAGIC: u32 = 0x06054B50;
const APK_SIGNING_BLOCK_MAGIC: [u8; 16] = *b"APK Sig Block 42";
const SIGNATURE_SCHEME_V2_MAGIC: u32 = 0x7109871A;

macro_rules! bad_apk {
    ($msg:literal) => {
        io::Error::new(io::ErrorKind::InvalidData, concat!("cert: ", $msg))
    };
}

/*
 * A v2/v3 signed APK has the format as following
 *
 * +---------------+
 * | zip content   |
 * +---------------+
 * | signing block |
 * +---------------+
 * | central dir   |
 * +---------------+
 * | EOCD          |
 * +---------------+
 *
 * Scan from end of file to find EOCD, and figure our way back to the
 * offset of the signing block. Next, directly extract the certificate
 * from the v2 signature block.
 *
 * All structures above are mostly just for documentation purpose.
 *
 * This method extracts the first certificate of the first signer
 * within the APK v2 signature block.
 */
pub fn read_certificate(fd: RawFd, version: i32) -> Vec<u8> {
    fn inner(apk: &mut File, version: i32) -> Result<Vec<u8>, io::Error> {
        let mut u32_value = 0u32;
        let mut u64_value = 0u64;

        // Find EOCD
        for i in 0u16.. {
            let mut comment_sz = 0u16;
            apk.seek(SeekFrom::End(-(comment_sz.bytes_size() as i64) - i as i64))?;
            apk.read_exact(comment_sz.as_raw_bytes_mut())?;

            if comment_sz == i {
                apk.seek(SeekFrom::Current(-22))?;
                let mut magic = 0u32;
                apk.read_exact(magic.as_raw_bytes_mut())?;
                if magic == EOCD_MAGIC {
                    break;
                }
            }
            if i == 0xffff {
                return Err(bad_apk!("invalid APK format"));
            }
        }

        // We are now at EOCD + sizeof(magic)
        // Seek and read central_dir_off to find the start of the central directory
        let mut central_dir_off = 0u32;
        apk.seek(SeekFrom::Current(12))?;

        apk.read_exact(central_dir_off.as_raw_bytes_mut())?;

        // Code for parse APK comment to get version code
        if version >= 0 {
            let mut comment_sz = 0u16;
            apk.read_exact(comment_sz.as_raw_bytes_mut())?;
            let mut comment = vec![0u8; comment_sz as usize];
            apk.read_exact(&mut comment)?;
            let mut comment = io::Cursor::new(&comment);
            let mut apk_ver = 0;
            comment.foreach_props(|k, v| {
                if k == "versionCode" {
                    apk_ver = v.trim().parse::<i32>().unwrap_or(0);
                    false
                } else {
                    true
                }
            });
            if version > apk_ver {
                return Err(bad_apk!("APK version too low"));
            }
        }

        // Next, find the start of the APK signing block
        apk.seek(SeekFrom::Start((central_dir_off - 24) as u64))?;
        apk.read_exact(u64_value.as_raw_bytes_mut())?; // u64_value = block_sz_
        let mut magic = [0u8; 16];
        apk.read_exact(magic.as_raw_bytes_mut())?;
        if magic != APK_SIGNING_BLOCK_MAGIC {
            return Err(bad_apk!("invalid signing block magic"));
        }
        let mut signing_blk_sz = 0u64;
        apk.seek(SeekFrom::Current(
            -(u64_value as i64) - (signing_blk_sz.bytes_size() as i64),
        ))?;
        apk.read_exact(signing_blk_sz.as_raw_bytes_mut())?;
        if signing_blk_sz != u64_value {
            return Err(bad_apk!("invalid signing block size"));
        }

        // Finally, we are now at the beginning of the id-value pair sequence
        loop {
            apk.read_exact(u64_value.as_raw_bytes_mut())?; // id-value pair length
            if u64_value == signing_blk_sz {
                break;
            }

            let mut id = 0u32;
            apk.read_exact(id.as_raw_bytes_mut())?;
            if id == SIGNATURE_SCHEME_V2_MAGIC {
                // Skip [signer sequence length] + [1st signer length] + [signed data length]
                apk.seek(SeekFrom::Current((u32_value.bytes_size() * 3) as i64))?;

                apk.read_exact(u32_value.as_raw_bytes_mut())?; // digest sequence length
                apk.seek(SeekFrom::Current(u32_value as i64))?; // skip all digests

                apk.seek(SeekFrom::Current(u32_value.bytes_size() as i64))?; // cert sequence length
                apk.read_exact(u32_value.as_raw_bytes_mut())?; // 1st cert length

                let mut cert = vec![0; u32_value as usize];
                apk.read_exact(cert.as_mut())?;
                return Ok(cert);
            } else {
                // Skip this id-value pair
                apk.seek(SeekFrom::Current(
                    u64_value as i64 - (id.bytes_size() as i64),
                ))?;
            }
        }

        Err(bad_apk!("cannot find certificate"))
    }
    inner(unsafe { &mut File::from_raw_fd(libc::dup(fd)) }, version)
        .log()
        .unwrap_or(vec![])
}
