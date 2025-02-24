#![feature(format_args_nl)]
#![feature(try_blocks)]

pub use base;
use base::libc::{O_CLOEXEC, O_RDONLY};
use base::{BufReadExt, FsPath, LoggedResult, Utf8CStr};
use cxx::CxxString;
use std::fmt::Write;
use std::io::{BufRead, BufReader, Cursor};

use crate::ffi::SePolicy;

mod rules;
mod statement;
#[cfg(feature = "main")]
mod cli;

#[cxx::bridge]
pub mod ffi {
    struct Xperm {
        low: u16,
        high: u16,
        reset: bool,
    }

    struct SePolicy {
        #[cxx_name = "impl"]
        _impl: UniquePtr<sepol_impl>,
    }

    unsafe extern "C++" {
        include!("policy.hpp");
        include!("../base/include/base.hpp");

        #[namespace = "rust"]
        #[cxx_name = "Utf8CStr"]
        type Utf8CStrRef<'a> = base::ffi::Utf8CStrRef<'a>;

        type sepol_impl;

        fn allow(self: &mut SePolicy, s: Vec<&str>, t: Vec<&str>, c: Vec<&str>, p: Vec<&str>);
        fn deny(self: &mut SePolicy, s: Vec<&str>, t: Vec<&str>, c: Vec<&str>, p: Vec<&str>);
        fn auditallow(self: &mut SePolicy, s: Vec<&str>, t: Vec<&str>, c: Vec<&str>, p: Vec<&str>);
        fn dontaudit(self: &mut SePolicy, s: Vec<&str>, t: Vec<&str>, c: Vec<&str>, p: Vec<&str>);
        fn allowxperm(self: &mut SePolicy, s: Vec<&str>, t: Vec<&str>, c: Vec<&str>, p: Vec<Xperm>);
        fn auditallowxperm(
            self: &mut SePolicy,
            s: Vec<&str>,
            t: Vec<&str>,
            c: Vec<&str>,
            p: Vec<Xperm>,
        );
        fn dontauditxperm(
            self: &mut SePolicy,
            s: Vec<&str>,
            t: Vec<&str>,
            c: Vec<&str>,
            p: Vec<Xperm>,
        );
        fn permissive(self: &mut SePolicy, t: Vec<&str>);
        fn enforce(self: &mut SePolicy, t: Vec<&str>);
        fn typeattribute(self: &mut SePolicy, t: Vec<&str>, a: Vec<&str>);
        #[cxx_name = "type"]
        fn type_(self: &mut SePolicy, t: &str, a: Vec<&str>);
        fn attribute(self: &mut SePolicy, t: &str);
        fn type_transition(self: &mut SePolicy, s: &str, t: &str, c: &str, d: &str, o: &str);
        fn type_change(self: &mut SePolicy, s: &str, t: &str, c: &str, d: &str);
        fn type_member(self: &mut SePolicy, s: &str, t: &str, c: &str, d: &str);
        fn genfscon(self: &mut SePolicy, s: &str, t: &str, c: &str);
        #[allow(dead_code)]
        fn strip_dontaudit(self: &mut SePolicy);

        fn print_rules(self: &SePolicy);
        fn to_file(self: &SePolicy, file: Utf8CStrRef) -> bool;

        #[Self = SePolicy]
        fn from_file(file: Utf8CStrRef) -> SePolicy;
        #[Self = SePolicy]
        fn from_split() -> SePolicy;
        #[Self = SePolicy]
        fn compile_split() -> SePolicy;
        #[Self = SePolicy]
        fn from_data(data: &[u8]) -> SePolicy;
    }

    extern "Rust" {
        fn parse_statement(self: &mut SePolicy, statement: Utf8CStrRef);
        fn magisk_rules(self: &mut SePolicy);
        fn load_rule_file(self: &mut SePolicy, filename: Utf8CStrRef);
        fn load_rules(self: &mut SePolicy, rules: &CxxString);
        #[Self = SePolicy]
        fn xperm_to_string(perm: &Xperm) -> String;
    }
}

impl SePolicy {
    fn load_rules(self: &mut SePolicy, rules: &CxxString) {
        let mut cursor = Cursor::new(rules.as_bytes());
        self.load_rules_from_reader(&mut cursor);
    }

    pub fn load_rule_file(self: &mut SePolicy, filename: &Utf8CStr) {
        let result: LoggedResult<()> = try {
            let file = FsPath::from(filename).open(O_RDONLY | O_CLOEXEC)?;
            let mut reader = BufReader::new(file);
            self.load_rules_from_reader(&mut reader);
        };
        result.ok();
    }

    fn load_rules_from_reader<T: BufRead>(self: &mut SePolicy, reader: &mut T) {
        reader.foreach_lines(|line| {
            self.parse_statement(line);
            true
        });
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
}
