#![feature(format_args_nl)]
#![feature(try_blocks)]

use std::fmt::Write;
use std::io::{BufRead, BufReader, Cursor};
use cxx::CxxString;
pub use base;
use base::libc::{O_CLOEXEC, O_RDONLY};
use base::{BufReadExt, FsPath, LoggedResult, Utf8CStr};

use crate::ffi::sepolicy;

mod rules;
mod statement;

#[cxx::bridge]
pub mod ffi {
    struct Xperm {
        low: u16,
        high: u16,
        reset: bool,
    }

    pub struct sepolicy {
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

        fn allow(self: &mut sepolicy, s: Vec<&str>, t: Vec<&str>, c: Vec<&str>, p: Vec<&str>);
        fn deny(self: &mut sepolicy, s: Vec<&str>, t: Vec<&str>, c: Vec<&str>, p: Vec<&str>);
        fn auditallow(
            self: &mut sepolicy,
            s: Vec<&str>,
            t: Vec<&str>,
            c: Vec<&str>,
            p: Vec<&str>,
        );
        fn dontaudit(
            self: &mut sepolicy,
            s: Vec<&str>,
            t: Vec<&str>,
            c: Vec<&str>,
            p: Vec<&str>,
        );
        fn allowxperm(
            self: &mut sepolicy,
            s: Vec<&str>,
            t: Vec<&str>,
            c: Vec<&str>,
            p: Vec<Xperm>,
        );
        fn auditallowxperm(
            self: &mut sepolicy,
            s: Vec<&str>,
            t: Vec<&str>,
            c: Vec<&str>,
            p: Vec<Xperm>,
        );
        fn dontauditxperm(
            self: &mut sepolicy,
            s: Vec<&str>,
            t: Vec<&str>,
            c: Vec<&str>,
            p: Vec<Xperm>,
        );
        fn permissive(self: &mut sepolicy, t: Vec<&str>);
        fn enforce(self: &mut sepolicy, t: Vec<&str>);
        fn typeattribute(self: &mut sepolicy, t: Vec<&str>, a: Vec<&str>);
        #[cxx_name = "type"]
        fn type_(self: &mut sepolicy, t: &str, a: Vec<&str>);
        fn attribute(self: &mut sepolicy, t: &str);
        fn type_transition(self: &mut sepolicy, s: &str, t: &str, c: &str, d: &str, o: &str);
        fn type_change(self: &mut sepolicy, s: &str, t: &str, c: &str, d: &str);
        fn type_member(self: &mut sepolicy, s: &str, t: &str, c: &str, d: &str);
        fn genfscon(self: &mut sepolicy, s: &str, t: &str, c: &str);
        #[allow(dead_code)]
        fn strip_dontaudit(self: &mut sepolicy);

        fn print_rules(self: &sepolicy);
        fn to_file(self: &sepolicy, file: Utf8CStrRef) -> bool;

        #[Self = sepolicy]
        fn from_file(file: Utf8CStrRef) -> UniquePtr<sepolicy>;
        #[Self = sepolicy]
        fn from_split() -> UniquePtr<sepolicy>;
        #[Self = sepolicy]
        fn compile_split() -> UniquePtr<sepolicy>;
        #[Self = sepolicy]
        unsafe fn from_data(data: *mut c_char, len: usize) -> UniquePtr<sepolicy>;
    }

    extern "Rust" {
        fn parse_statement(self: &mut sepolicy, statement: Utf8CStrRef);
        fn magisk_rules(self: &mut sepolicy);
        fn load_rule_file(self: &mut sepolicy, filename: Utf8CStrRef);
        fn load_rules(self: &mut sepolicy, rules: &CxxString);
        #[Self = sepolicy]
        fn xperm_to_string(perm: &Xperm) -> String;
        #[Self = sepolicy]
        fn print_statement_help();
    }
}

impl sepolicy {
    fn load_rules(self: &mut sepolicy, rules: &CxxString) {
        let mut cursor = Cursor::new(rules.as_bytes());
        self.load_rules_from_reader(&mut cursor);
    }

    pub fn load_rule_file(self: &mut sepolicy, filename: &Utf8CStr) {
        let result: LoggedResult<()> = try {
            let file = FsPath::from(filename).open(O_RDONLY | O_CLOEXEC)?;
            let mut reader = BufReader::new(file);
            self.load_rules_from_reader(&mut reader);
        };
        result.ok();
    }

    fn load_rules_from_reader<T: BufRead>(self: &mut sepolicy, reader: &mut T) {
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
