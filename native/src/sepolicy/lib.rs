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

fn load_rules_from_reader<T: BufRead>(mut sepol: Pin<&mut sepolicy>, reader: &mut T) {
    reader.foreach_lines(|line| {
        let bytes = line.trim().as_bytes();
        unsafe {
            sepol
                .as_mut()
                .parse_statement(bytes.as_ptr().cast(), bytes.len() as i32);
        }
        true
    });
}

pub fn load_rule_file(sepol: Pin<&mut sepolicy>, filename: &[u8]) {
    fn inner(sepol: Pin<&mut sepolicy>, filename: &[u8]) -> anyhow::Result<()> {
        let filename = str::from_utf8(filename)?;
        let mut reader = BufReader::new(File::open(filename)?);
        load_rules_from_reader(sepol, &mut reader);
        Ok(())
    }
    inner(sepol, filename).ok_or_log();
}

pub fn load_rules(sepol: Pin<&mut sepolicy>, rules: &[u8]) {
    let mut cursor = Cursor::new(rules);
    load_rules_from_reader(sepol, &mut cursor);
}
