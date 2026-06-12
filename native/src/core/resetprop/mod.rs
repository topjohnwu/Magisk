use base::libc::c_char;
use base::{Utf8CStr, libc};
pub use cli::{get_prop, load_prop_file, resetprop_main, set_prop};
use libc::timespec;
use std::collections::BTreeMap;
use std::ffi::CStr;
use std::ptr;
use std::sync::LazyLock;

mod cli;
mod persist;
mod proto;

static SYS_PROP: LazyLock<SysProp> = LazyLock::new(|| unsafe { get_sys_prop() });

#[repr(C)]
struct PropInfo {
    _private: cxx::private::Opaque,
}

type CharPtr = *const c_char;
type ReadCallback = unsafe extern "C" fn(&mut PropReader, CharPtr, CharPtr, u32);
type ForEachCallback = unsafe extern "C" fn(&PropInfo, &mut PropReader);

enum PropReader<'a> {
    Value(&'a mut String),
    ValueSerial(&'a mut String, &'a mut u32),
    List(&'a mut BTreeMap<String, String>),
}

impl PropReader<'_> {
    fn put_cstr(&mut self, key: CharPtr, val: CharPtr, serial: u32) {
        let key = unsafe { CStr::from_ptr(key) };
        let val = unsafe { CStr::from_ptr(val) };
        match self {
            PropReader::Value(v) => {
                **v = String::from_utf8_lossy(val.to_bytes()).into_owned();
            }
            PropReader::ValueSerial(v, s) => {
                **v = String::from_utf8_lossy(val.to_bytes()).into_owned();
                **s = serial;
            }
            PropReader::List(map) => {
                map.insert(
                    String::from_utf8_lossy(key.to_bytes()).into_owned(),
                    String::from_utf8_lossy(val.to_bytes()).into_owned(),
                );
            }
        }
    }

    fn put_str(&mut self, key: String, val: String, serial: u32) {
        match self {
            PropReader::Value(v) => {
                **v = val;
            }
            PropReader::ValueSerial(v, s) => {
                **v = val;
                **s = serial;
            }
            PropReader::List(map) => {
                map.insert(key, val);
            }
        }
    }
}

unsafe extern "C" {
    // SAFETY: the improper_ctypes warning is about PropReader. We only pass PropReader
    // to C functions as raw pointers, and all actual usage happens on the Rust side.
    #[allow(improper_ctypes)]
    fn get_sys_prop() -> SysProp;

    fn prop_info_is_long(info: &PropInfo) -> bool;
    #[link_name = "__system_property_find2"]
    fn sys_prop_find(key: CharPtr) -> Option<&'static mut PropInfo>;
    #[link_name = "__system_property_update2"]
    fn sys_prop_update(info: &mut PropInfo, val: CharPtr, val_len: u32) -> i32;
    #[link_name = "__system_property_add2"]
    fn sys_prop_add(key: CharPtr, key_len: u32, val: CharPtr, val_len: u32) -> i32;
    #[link_name = "__system_property_delete"]
    fn sys_prop_delete(key: CharPtr, prune: bool) -> i32;
    #[link_name = "__system_property_get_context"]
    fn sys_prop_get_context(key: CharPtr) -> CharPtr;
    #[link_name = "__system_property_area_serial2"]
    fn sys_prop_area_serial() -> u32;
}

#[repr(C)]
struct SysProp {
    set: unsafe extern "C" fn(CharPtr, CharPtr) -> i32,
    find: unsafe extern "C" fn(CharPtr) -> Option<&'static PropInfo>,
    read_callback: unsafe extern "C" fn(&PropInfo, ReadCallback, &mut PropReader) -> i32,
    foreach: unsafe extern "C" fn(ForEachCallback, &mut PropReader) -> i32,
    wait: unsafe extern "C" fn(Option<&PropInfo>, u32, &mut u32, *const timespec) -> i32,
}

// Safe abstractions over raw C APIs

impl PropInfo {
    fn read(&self, reader: &mut PropReader) {
        SYS_PROP.read(self, reader);
    }

    fn update(&mut self, val: &Utf8CStr) {
        SYS_PROP.update(self, val);
    }

    fn is_long(&self) -> bool {
        unsafe { prop_info_is_long(self) }
    }
}

impl SysProp {
    fn read(&self, info: &PropInfo, reader: &mut PropReader) {
        unsafe extern "C" fn read_fn(r: &mut PropReader, key: CharPtr, val: CharPtr, serial: u32) {
            r.put_cstr(key, val, serial);
        }
        unsafe {
            (self.read_callback)(info, read_fn, reader);
        }
    }

    fn find(&self, key: &Utf8CStr) -> Option<&'static PropInfo> {
        unsafe { (self.find)(key.as_ptr()) }
    }

    fn find_mut(&self, key: &Utf8CStr) -> Option<&'static mut PropInfo> {
        unsafe { sys_prop_find(key.as_ptr()) }
    }

    fn set(&self, key: &Utf8CStr, val: &Utf8CStr) {
        unsafe {
            (self.set)(key.as_ptr(), val.as_ptr());
        }
    }

    fn add(&self, key: &Utf8CStr, val: &Utf8CStr) {
        unsafe {
            sys_prop_add(
                key.as_ptr(),
                key.len() as u32,
                val.as_ptr(),
                val.len() as u32,
            );
        }
    }

    fn update(&self, info: &mut PropInfo, val: &Utf8CStr) {
        unsafe {
            sys_prop_update(info, val.as_ptr(), val.len() as u32);
        }
    }

    fn delete(&self, key: &Utf8CStr, prune: bool) -> bool {
        unsafe { sys_prop_delete(key.as_ptr(), prune) == 0 }
    }

    fn for_each(&self, reader: &mut PropReader) {
        unsafe extern "C" fn for_each_fn(info: &PropInfo, vals: &mut PropReader) {
            SYS_PROP.read(info, vals);
        }
        unsafe {
            (self.foreach)(for_each_fn, reader);
        }
    }

    fn wait(&self, info: Option<&PropInfo>, old_serial: u32, new_serial: &mut u32) {
        unsafe {
            (self.wait)(info, old_serial, new_serial, ptr::null());
        }
    }

    fn get_context(&self, key: &Utf8CStr) -> &'static Utf8CStr {
        unsafe { Utf8CStr::from_ptr_unchecked(sys_prop_get_context(key.as_ptr())) }
    }

    fn area_serial(&self) -> u32 {
        unsafe { sys_prop_area_serial() }
    }
}
