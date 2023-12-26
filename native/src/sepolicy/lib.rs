use io::Cursor;
use std::io;
use std::io::{BufRead, BufReader};
use std::pin::Pin;

pub use base;
use base::libc::{O_CLOEXEC, O_RDONLY};
use base::{BufReadExt, FsPath, LoggedResult, Utf8CStr};

use crate::ffi::sepolicy;

#[cxx::bridge]
mod ffi {
    unsafe extern "C++" {
        #[namespace = "rust"]
        #[cxx_name = "Utf8CStr"]
        type Utf8CStrRef<'a> = base::ffi::Utf8CStrRef<'a>;

        include!("include/sepolicy.hpp");

        type sepolicy;
        fn parse_statement(self: Pin<&mut sepolicy>, stmt: &str);
    }

    #[namespace = "rust"]
    extern "Rust" {
        fn load_rules(sepol: Pin<&mut sepolicy>, rules: &[u8]);
        fn load_rule_file(sepol: Pin<&mut sepolicy>, filename: Utf8CStrRef);
    }
}

trait SepolicyExt {
    fn load_rules(self: Pin<&mut Self>, rules: &[u8]);
    fn load_rule_file(self: Pin<&mut Self>, filename: &Utf8CStr);
    fn load_rules_from_reader<T: BufRead>(self: Pin<&mut Self>, reader: &mut T);
}

impl SepolicyExt for sepolicy {
    fn load_rules(self: Pin<&mut sepolicy>, rules: &[u8]) {
        let mut cursor = Cursor::new(rules);
        self.load_rules_from_reader(&mut cursor);
    }

    fn load_rule_file(self: Pin<&mut sepolicy>, filename: &Utf8CStr) {
        fn inner(sepol: Pin<&mut sepolicy>, filename: &Utf8CStr) -> LoggedResult<()> {
            let file = FsPath::from(filename).open(O_RDONLY | O_CLOEXEC)?;
            let mut reader = BufReader::new(file);
            sepol.load_rules_from_reader(&mut reader);
            Ok(())
        }
        inner(self, filename).ok();
    }

    fn load_rules_from_reader<T: BufRead>(mut self: Pin<&mut sepolicy>, reader: &mut T) {
        reader.foreach_lines(|line| {
            let line = line.trim();
            self.as_mut().parse_statement(line);
            true
        });
    }
}

pub fn load_rule_file(sepol: Pin<&mut sepolicy>, filename: &Utf8CStr) {
    sepol.load_rule_file(filename);
}

pub fn load_rules(sepol: Pin<&mut sepolicy>, rules: &[u8]) {
    sepol.load_rules(rules);
}
