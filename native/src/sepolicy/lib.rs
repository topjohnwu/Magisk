#![feature(format_args_nl)]

use io::Cursor;
use std::io;
use std::io::{BufRead, BufReader};
use std::pin::Pin;

pub use base;
use base::libc::{O_CLOEXEC, O_RDONLY};
use base::{error, BufReadExt, FsPath, LoggedResult, Utf8CStr};

use crate::ffi::sepolicy;

mod statement;

mod rules;

#[cxx::bridge]
mod ffi {
    struct Xperm {
        low: u16,
        high: u16,
        reset: bool,
    }

    unsafe extern "C++" {
        #[namespace = "rust"]
        #[cxx_name = "Utf8CStr"]
        type Utf8CStrRef<'a> = base::ffi::Utf8CStrRef<'a>;

        include!("include/sepolicy.hpp");

        type sepolicy;
        fn allow(self: Pin<&mut sepolicy>, s: Vec<&str>, t: Vec<&str>, c: Vec<&str>, p: Vec<&str>);
        fn deny(self: Pin<&mut sepolicy>, s: Vec<&str>, t: Vec<&str>, c: Vec<&str>, p: Vec<&str>);
        fn auditallow(
            self: Pin<&mut sepolicy>,
            s: Vec<&str>,
            t: Vec<&str>,
            c: Vec<&str>,
            p: Vec<&str>,
        );
        fn dontaudit(
            self: Pin<&mut sepolicy>,
            s: Vec<&str>,
            t: Vec<&str>,
            c: Vec<&str>,
            p: Vec<&str>,
        );
        fn allowxperm(
            self: Pin<&mut sepolicy>,
            s: Vec<&str>,
            t: Vec<&str>,
            c: Vec<&str>,
            p: Vec<Xperm>,
        );
        fn auditallowxperm(
            self: Pin<&mut sepolicy>,
            s: Vec<&str>,
            t: Vec<&str>,
            c: Vec<&str>,
            p: Vec<Xperm>,
        );
        fn dontauditxperm(
            self: Pin<&mut sepolicy>,
            s: Vec<&str>,
            t: Vec<&str>,
            c: Vec<&str>,
            p: Vec<Xperm>,
        );
        fn permissive(self: Pin<&mut sepolicy>, t: Vec<&str>);
        fn enforce(self: Pin<&mut sepolicy>, t: Vec<&str>);
        fn typeattribute(self: Pin<&mut sepolicy>, t: Vec<&str>, a: Vec<&str>);
        #[cxx_name = "type"]
        fn type_(self: Pin<&mut sepolicy>, t: &str, a: Vec<&str>);
        fn attribute(self: Pin<&mut sepolicy>, t: &str);
        fn type_transition(
            self: Pin<&mut sepolicy>,
            s: &str,
            t: &str,
            c: &str,
            d: &str,
            o: Vec<&str>,
        );
        fn type_change(self: Pin<&mut sepolicy>, s: &str, t: &str, c: &str, d: &str);
        fn type_member(self: Pin<&mut sepolicy>, s: &str, t: &str, c: &str, d: &str);
        fn genfscon(self: Pin<&mut sepolicy>, s: &str, t: &str, c: &str);
        fn strip_dontaudit(self: Pin<&mut sepolicy>);
    }

    #[namespace = "rust"]
    extern "Rust" {
        fn load_rules(sepol: Pin<&mut sepolicy>, rules: &[u8]);
        fn load_rule_file(sepol: Pin<&mut sepolicy>, filename: Utf8CStrRef);
        fn parse_statement(sepol: Pin<&mut sepolicy>, statement: Utf8CStrRef);
        fn magisk_rules(sepol: Pin<&mut sepolicy>);
    }
}

trait SepolicyExt {
    fn load_rules(self: Pin<&mut Self>, rules: &[u8]);
    fn load_rule_file(self: Pin<&mut Self>, filename: &Utf8CStr);
    fn load_rules_from_reader<T: BufRead>(self: Pin<&mut Self>, reader: &mut T);
    fn parse_statement(self: Pin<&mut Self>, statement: &str);
}

trait SepolicyMagisk {
    fn magisk_rules(self: Pin<&mut Self>);
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
            if line.is_empty() {
                return true;
            }
            self.as_mut().parse_statement(line);
            true
        });
    }

    fn parse_statement(self: Pin<&mut Self>, statement: &str) {
        if let Err(_) = statement::parse_statement(self, statement) {
            error!("sepolicy rule syntax error: {statement}");
        }
    }
}

pub fn load_rule_file(sepol: Pin<&mut sepolicy>, filename: &Utf8CStr) {
    sepol.load_rule_file(filename);
}

pub fn load_rules(sepol: Pin<&mut sepolicy>, rules: &[u8]) {
    sepol.load_rules(rules);
}

pub fn parse_statement(sepol: Pin<&mut sepolicy>, statement: &Utf8CStr) {
    sepol.parse_statement(statement.as_str());
}

pub fn magisk_rules(sepol: Pin<&mut sepolicy>) {
    sepol.magisk_rules();
}
