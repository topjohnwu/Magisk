#![feature(format_args_nl)]
#![feature(try_blocks)]

use io::Cursor;
use std::fmt::Write;
use std::io;
use std::io::{BufRead, BufReader};
use std::pin::Pin;

pub use base;
use base::libc::{O_CLOEXEC, O_RDONLY};
use base::{BufReadExt, FsPath, LoggedResult, Utf8CStr};
use statement::{parse_statement, print_statement_help};

use crate::ffi::sepolicy;

mod rules;
mod statement;

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
        fn type_transition(self: Pin<&mut sepolicy>, s: &str, t: &str, c: &str, d: &str, o: &str);
        fn type_change(self: Pin<&mut sepolicy>, s: &str, t: &str, c: &str, d: &str);
        fn type_member(self: Pin<&mut sepolicy>, s: &str, t: &str, c: &str, d: &str);
        fn genfscon(self: Pin<&mut sepolicy>, s: &str, t: &str, c: &str);
        #[allow(dead_code)]
        fn strip_dontaudit(self: Pin<&mut sepolicy>);
    }

    #[namespace = "rust"]
    extern "Rust" {
        fn load_rules(sepol: Pin<&mut sepolicy>, rules: &[u8]);
        fn load_rule_file(sepol: Pin<&mut sepolicy>, filename: Utf8CStrRef);
        fn parse_statement(sepol: Pin<&mut sepolicy>, statement: Utf8CStrRef);
        fn magisk_rules(sepol: Pin<&mut sepolicy>);
        fn xperm_to_string(perm: &Xperm) -> String;
        fn print_statement_help();
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
        let result: LoggedResult<()> = try {
            let file = FsPath::from(filename).open(O_RDONLY | O_CLOEXEC)?;
            let mut reader = BufReader::new(file);
            self.load_rules_from_reader(&mut reader);
        };
        result.ok();
    }

    fn load_rules_from_reader<T: BufRead>(mut self: Pin<&mut sepolicy>, reader: &mut T) {
        reader.foreach_lines(|line| {
            parse_statement(self.as_mut(), line);
            true
        });
    }
}

fn load_rule_file(sepol: Pin<&mut sepolicy>, filename: &Utf8CStr) {
    sepol.load_rule_file(filename);
}

fn load_rules(sepol: Pin<&mut sepolicy>, rules: &[u8]) {
    sepol.load_rules(rules);
}

trait SepolicyMagisk {
    fn magisk_rules(self: Pin<&mut Self>);
}

fn magisk_rules(sepol: Pin<&mut sepolicy>) {
    sepol.magisk_rules();
}

fn xperm_to_string(perm: &ffi::Xperm) -> String {
    let mut s = String::new();
    if perm.reset {
        s.push('~');
    }
    if perm.low == perm.high {
        s.write_fmt(format_args!("{{ {:#06X} }}", perm.low)).ok();
    } else {
        s.write_fmt(format_args!("{{ {:#06X}-{:#06X} }}", perm.low, perm.high))
            .ok();
    }
    s
}
