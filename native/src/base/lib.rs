#![feature(format_args_nl)]

pub use libc;

pub use logging::*;
pub use misc::*;
pub use xwrap::*;

mod logging;
mod misc;
mod xwrap;

#[cxx::bridge]
pub mod ffi {
    #[derive(Copy, Clone)]
    pub enum LogLevel {
        Error,
        Warn,
        Info,
        Debug,
    }

    extern "Rust" {
        fn log_with_rs(level: LogLevel, msg: &str);
        fn exit_on_error(b: bool);
        fn set_log_level_state(level: LogLevel, enabled: bool);
        fn cmdline_logging();
    }
}

#[cxx::bridge(namespace = "rust")]
pub mod ffi2 {
    extern "Rust" {
        fn xwrite(fd: i32, data: &[u8]) -> isize;
        fn xread(fd: i32, data: &mut [u8]) -> isize;
        fn xxread(fd: i32, data: &mut [u8]) -> isize;
        fn xpipe2(fds: &mut [i32; 2], flags: i32) -> i32;
    }
}
