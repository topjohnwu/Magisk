use io::Cursor;
use std::fs::File;
use std::io::{BufRead, BufReader};
use std::pin::Pin;
use std::{io, str};

pub use base;
use base::*;

use crate::ffi::sepolicy;

#[cxx::bridge]
mod ffi {
    unsafe extern "C++" {
        include!("include/sepolicy.hpp");

        type sepolicy;
        unsafe fn parse_statement(self: Pin<&mut sepolicy>, stmt: *const c_char, len: i32);
    }

    #[namespace = "rust"]
    extern "Rust" {
        fn load_rules(sepol: Pin<&mut sepolicy>, rules: &[u8]);
        fn load_rule_file(sepol: Pin<&mut sepolicy>, filename: &[u8]);
    }
}

trait SepolicyExt {
    fn load_rules(self: Pin<&mut Self>, rules: &[u8]);
    fn load_rule_file(self: Pin<&mut Self>, filename: &[u8]);
    fn load_rules_from_reader<T: BufRead>(self: Pin<&mut Self>, reader: &mut T);
}

impl SepolicyExt for sepolicy {
    fn load_rules(self: Pin<&mut sepolicy>, rules: &[u8]) {
        let mut cursor = Cursor::new(rules);
        self.load_rules_from_reader(&mut cursor);
    }

    fn load_rule_file(self: Pin<&mut sepolicy>, filename: &[u8]) {
        fn inner(sepol: Pin<&mut sepolicy>, filename: &[u8]) -> LoggedResult<()> {
            let filename = str::from_utf8(filename)?;
            let mut reader = BufReader::new(File::open(filename)?);
            sepol.load_rules_from_reader(&mut reader);
            Ok(())
        }
        inner(self, filename).ok();
    }

    fn load_rules_from_reader<T: BufRead>(mut self: Pin<&mut sepolicy>, reader: &mut T) {
        reader.foreach_lines(|line| {
            let bytes = line.trim().as_bytes();
            unsafe {
                self.as_mut()
                    .parse_statement(bytes.as_ptr().cast(), bytes.len() as i32);
            }
            true
        });
    }
}

pub fn load_rule_file(sepol: Pin<&mut sepolicy>, filename: &[u8]) {
    sepol.load_rule_file(filename);
}

pub fn load_rules(sepol: Pin<&mut sepolicy>, rules: &[u8]) {
    sepol.load_rules(rules);
}
