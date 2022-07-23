use std::cmp::min;
use std::fmt;
use std::fmt::Arguments;

struct BufFmtWriter<'a> {
    buf: &'a mut [u8],
    used: usize,
}

impl<'a> BufFmtWriter<'a> {
    fn new(buf: &'a mut [u8]) -> Self {
        BufFmtWriter { buf, used: 0 }
    }
}

impl<'a> fmt::Write for BufFmtWriter<'a> {
    // The buffer should always be null terminated
    fn write_str(&mut self, s: &str) -> fmt::Result {
        if self.used >= self.buf.len() - 1 {
            // Silent truncate
            return Ok(());
        }
        let remain = &mut self.buf[self.used..];
        let s_bytes = s.as_bytes();
        let copied = min(s_bytes.len(), remain.len() - 1);
        remain[..copied].copy_from_slice(&s_bytes[..copied]);
        self.used += copied;
        self.buf[self.used] = b'\0';
        // Silent truncate
        Ok(())
    }
}

pub fn fmt_to_buf(buf: &mut [u8], args: Arguments) -> usize {
    let mut w = BufFmtWriter::new(buf);
    if let Ok(()) = fmt::write(&mut w, args) {
        w.used
    } else {
        0
    }
}
