#![feature(format_args_nl)]
#![allow(clippy::missing_safety_doc)]

use base::Utf8CStr;
use cert::read_certificate;
use daemon::{daemon_entry, find_apk_path, get_magiskd, zygisk_entry, MagiskD};
use logging::{android_logging, magisk_logging, zygisk_logging};
use resetprop::{persist_delete_prop, persist_get_prop, persist_get_props, persist_set_prop};

mod cert;
#[path = "../include/consts.rs"]
mod consts;
mod daemon;
mod logging;
mod resetprop;

#[cxx::bridge]
pub mod ffi {
    extern "C++" {
        include!("resetprop/resetprop.hpp");

        #[cxx_name = "prop_cb"]
        type PropCb;
        unsafe fn get_prop_rs(name: *const c_char, persist: bool) -> String;
        unsafe fn prop_cb_exec(cb: Pin<&mut PropCb>, name: *const c_char, value: *const c_char);
    }

    extern "Rust" {
        fn rust_test_entry();
        fn android_logging();
        fn magisk_logging();
        fn zygisk_logging();
        fn find_apk_path(pkg: &[u8], data: &mut [u8]) -> usize;
        fn read_certificate(fd: i32, version: i32) -> Vec<u8>;
        unsafe fn persist_get_prop(name: *const c_char, prop_cb: Pin<&mut PropCb>);
        unsafe fn persist_get_props(prop_cb: Pin<&mut PropCb>);
        unsafe fn persist_delete_prop(name: *const c_char) -> bool;
        unsafe fn persist_set_prop(name: *const c_char, value: *const c_char) -> bool;
    }

    #[namespace = "rust"]
    extern "Rust" {
        fn daemon_entry();
        fn zygisk_entry();

        type MagiskD;
        fn get_magiskd() -> &'static MagiskD;
        fn get_log_pipe(self: &MagiskD) -> i32;
        fn close_log_pipe(self: &MagiskD);
        fn setup_logfile(self: &MagiskD);
        fn is_emulator(self: &MagiskD) -> bool;
    }
}

fn rust_test_entry() {}

pub fn get_prop(name: &Utf8CStr, persist: bool) -> String {
    unsafe { ffi::get_prop_rs(name.as_ptr(), persist) }
}
