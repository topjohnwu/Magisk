use base::log_err;
use lzma_rs::xz_decompress;
use std::fs::File;
use std::io::Cursor;
use std::os::fd::{FromRawFd, RawFd};

pub fn unxz(fd: i32, buf: &[u8]) -> bool {
    let mut reader = Cursor::new(buf);
    let mut writer = unsafe { File::from_raw_fd(RawFd::from(fd)) };
    match xz_decompress(&mut reader, &mut writer) {
        Err(e) => {
            log_err!("unxz: {}", e);
            false
        }
        Ok(_) => true,
    }
}
