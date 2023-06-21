use base::{MappedFile, MutBytesExt, ResultExt, Utf8CStr};

fn hex2byte(hex: &[u8]) -> Vec<u8> {
    let mut v = Vec::new();
    v.reserve(hex.len() / 2);
    for bytes in hex.chunks(2) {
        if bytes.len() != 2 {
            break;
        }
        let high = bytes[0].to_ascii_uppercase() - b'0';
        let low = bytes[1].to_ascii_uppercase() - b'0';
        let h = if high > 9 { high - 7 } else { high };
        let l = if low > 9 { low - 7 } else { low };
        v.push(h << 4 | l);
    }
    v
}

pub fn hexpatch(file: &[u8], from: &[u8], to: &[u8]) -> bool {
    fn inner(file: &[u8], from: &[u8], to: &[u8]) -> anyhow::Result<bool> {
        let file = Utf8CStr::from_bytes(file)?;
        let from = Utf8CStr::from_bytes(from)?;
        let to = Utf8CStr::from_bytes(to)?;

        let mut map = MappedFile::open_rw(file)?;
        let pattern = hex2byte(from.as_bytes());
        let patch = hex2byte(to.as_bytes());

        let v = map.patch(pattern.as_slice(), patch.as_slice());
        for off in &v {
            eprintln!("Patch @ {:#010X} [{}] -> [{}]", off, from, to);
        }

        Ok(!v.is_empty())
    }
    inner(file, from, to).log().unwrap_or(false)
}
