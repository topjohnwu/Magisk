use base::{LoggedResult, MappedFile, MutBytesExt, Utf8CStr};

// SAFETY: assert(buf.len() >= 1) && assert(len <= buf.len())
macro_rules! match_patterns {
    ($buf:ident, $($str:literal), *) => {{
        let mut len = if *$buf.get_unchecked(0) == b',' { 1 } else { 0 };
        let b = $buf.get_unchecked(len..);
        let found = if b.is_empty() {
            false
        }
        $(
        else if b.starts_with($str) {
            len += $str.len();
            true
        }
        )*
        else {
            false
        };
        if found {
            let b = $buf.get_unchecked(len..);
            if !b.is_empty() && b[0] == b'=' {
                for c in b.iter() {
                    if b" \n\0".contains(c) {
                        break;
                    }
                    len += 1;
                }
            }
            Some(len)
        } else {
            None
        }
    }};
}

fn remove_pattern(buf: &mut [u8], pattern_matcher: unsafe fn(&[u8]) -> Option<usize>) -> usize {
    let mut write = 0_usize;
    let mut read = 0_usize;
    let mut sz = buf.len();
    // SAFETY: assert(write <= read) && assert(read <= buf.len())
    unsafe {
        while read < buf.len() {
            if let Some(len) = pattern_matcher(buf.get_unchecked(read..)) {
                let skipped = buf.get_unchecked(read..(read + len));
                // SAFETY: all matching patterns are ASCII bytes
                let skipped = std::str::from_utf8_unchecked(skipped);
                eprintln!("Remove pattern [{skipped}]");
                sz -= len;
                read += len;
            } else {
                *buf.get_unchecked_mut(write) = *buf.get_unchecked(read);
                write += 1;
                read += 1;
            }
        }
    }
    if let Some(buf) = buf.get_mut(write..) {
        buf.fill(0);
    }
    sz
}

pub fn patch_verity(buf: &mut [u8]) -> usize {
    unsafe fn match_verity_pattern(buf: &[u8]) -> Option<usize> {
        unsafe {
            match_patterns!(
                buf,
                b"verifyatboot",
                b"verify",
                b"avb_keys",
                b"avb",
                b"support_scfs",
                b"fsverity"
            )
        }
    }

    remove_pattern(buf, match_verity_pattern)
}

pub fn patch_encryption(buf: &mut [u8]) -> usize {
    unsafe fn match_encryption_pattern(buf: &[u8]) -> Option<usize> {
        unsafe { match_patterns!(buf, b"forceencrypt", b"forcefdeorfbe", b"fileencryption") }
    }

    remove_pattern(buf, match_encryption_pattern)
}

fn hex2byte(hex: &[u8]) -> Vec<u8> {
    let mut v = Vec::with_capacity(hex.len() / 2);
    for bytes in hex.chunks(2) {
        if bytes.len() != 2 {
            break;
        }
        let high = bytes[0].to_ascii_uppercase() - b'0';
        let low = bytes[1].to_ascii_uppercase() - b'0';
        let h = if high > 9 { high - 7 } else { high };
        let l = if low > 9 { low - 7 } else { low };
        v.push((h << 4) | l);
    }
    v
}

pub fn hexpatch(file: &[u8], from: &[u8], to: &[u8]) -> bool {
    let res: LoggedResult<bool> = try {
        let file = Utf8CStr::from_bytes(file)?;
        let from = Utf8CStr::from_bytes(from)?;
        let to = Utf8CStr::from_bytes(to)?;

        let mut map = MappedFile::open_rw(file)?;
        let pattern = hex2byte(from.as_bytes());
        let patch = hex2byte(to.as_bytes());

        let v = map.patch(pattern.as_slice(), patch.as_slice());
        for off in &v {
            eprintln!("Patch @ {off:#010X} [{from}] -> [{to}]");
        }
        !v.is_empty()
    };
    res.unwrap_or(false)
}
