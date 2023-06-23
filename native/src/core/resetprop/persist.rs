use crate::ffi::{clone_attr, prop_cb, prop_cb_exec};
use crate::resetprop::proto::persistent_properties::{
    mod_PersistentProperties::PersistentPropertyRecord, PersistentProperties,
};
use base::{cstr, debug, libc::mkstemp, Directory, LoggedResult, MappedFile, Utf8CStr, WalkResult};
use core::ffi::c_char;
use quick_protobuf::{BytesReader, MessageRead, MessageWrite, Writer};
use std::{
    fs::{read_to_string, remove_file, rename, File},
    io::{BufWriter, Write},
    os::fd::FromRawFd,
    path::{Path, PathBuf},
};

macro_rules! PERSIST_PROP_DIR {
    () => {
        "/data/property"
    };
}

macro_rules! PERSIST_PROP {
    () => {
        concat!(PERSIST_PROP_DIR!(), "/persistent_properties")
    };
}

struct PropCb {
    cb: *mut prop_cb,
}

struct MatchNameCb<'a> {
    cb: PropCb,
    name: &'a Utf8CStr,
}

struct PropCollectCb<'a> {
    props: &'a mut PersistentProperties,
    replace_name: Option<&'a Utf8CStr>,
    replace_value: Option<&'a Utf8CStr>,
}

trait PropCbExec {
    fn exec(&mut self, name: &Utf8CStr, value: &Utf8CStr);
}

impl PropCbExec for PropCb {
    fn exec(&mut self, name: &Utf8CStr, value: &Utf8CStr) {
        if !self.cb.is_null() {
            unsafe { prop_cb_exec(self.cb, name.as_ptr(), value.as_ptr()) }
        }
    }
}

impl PropCbExec for MatchNameCb<'_> {
    fn exec(&mut self, name: &Utf8CStr, value: &Utf8CStr) {
        if name.as_bytes() == self.name.as_bytes() {
            self.cb.exec(name, value);
            debug!("resetprop: found prop [{}] = [{}]", name, value);
        }
    }
}

impl PropCbExec for PropCollectCb<'_> {
    fn exec(&mut self, name: &Utf8CStr, value: &Utf8CStr) {
        let replace_value = self.replace_value.unwrap_or(value);
        let value = self.replace_name.map_or(value, |replace_name| {
            if name.as_bytes() == replace_name.as_bytes() {
                replace_value
            } else {
                value
            }
        });
        self.props.properties.push(PersistentPropertyRecord {
            name: Some(name.to_string()),
            value: Some(value.to_string()),
        })
    }
}

fn check_pb() -> bool {
    Path::new(PERSIST_PROP!()).exists()
}

fn pb_get_prop(cb: &mut dyn PropCbExec) -> LoggedResult<()> {
    debug!("resetprop: decode with protobuf [{}]", PERSIST_PROP!());
    let m = MappedFile::open(cstr!(PERSIST_PROP!()))?;
    let m = m.as_ref();
    let mut r = BytesReader::from_bytes(m);
    let mut pp = PersistentProperties::from_reader(&mut r, m)?;
    pp.properties.iter_mut().for_each(|p| {
        if let PersistentPropertyRecord {
            name: Some(ref mut n),
            value: Some(ref mut v),
        } = p
        {
            cb.exec(Utf8CStr::from_string(n), Utf8CStr::from_string(v));
        }
    });
    Ok(())
}

fn file_get_prop(name: &Utf8CStr) -> LoggedResult<String> {
    let path = PathBuf::new().join(PERSIST_PROP_DIR!()).join(name);
    let path = path.as_path();
    debug!("resetprop: read prop from [{}]\n", path.display());
    Ok(read_to_string(path)?)
}

fn pb_write_props(props: &PersistentProperties) -> LoggedResult<()> {
    let mut tmp = String::from(concat!(PERSIST_PROP!(), ".XXXXXX"));
    let tmp = Utf8CStr::from_string(&mut tmp);
    {
        let f = unsafe {
            let fd = mkstemp(tmp.as_ptr() as *mut c_char);
            if fd < 0 {
                return Err(Default::default());
            }
            File::from_raw_fd(fd)
        };
        debug!("resetprop: encode with protobuf [{}]", tmp);
        props.write_message(&mut Writer::new(BufWriter::new(f)))?;
    }
    unsafe {
        clone_attr(cstr!(PERSIST_PROP!()).as_ptr(), tmp.as_ptr());
    }
    rename(tmp, PERSIST_PROP!())?;
    Ok(())
}

fn file_set_prop(name: &Utf8CStr, value: Option<&Utf8CStr>) -> LoggedResult<()> {
    let path = PathBuf::new().join(PERSIST_PROP_DIR!()).join(name);
    let path = path.as_path();
    if let Some(value) = value {
        let mut tmp = String::from(concat!(PERSIST_PROP_DIR!(), ".prop.XXXXXX"));
        {
            let mut f = unsafe {
                let fd = mkstemp(tmp.as_mut_ptr() as *mut c_char);
                if fd < 0 {
                    return Err(Default::default());
                }
                File::from_raw_fd(fd)
            };
            f.write_all(value.as_bytes())?;
        }
        debug!("resetprop: write prop to [{}]\n", tmp);
        rename(tmp, path)?;
    } else {
        debug!("resetprop: unlink [{}]\n", path.display());
        remove_file(path)?;
    }
    Ok(())
}

fn do_persist_get_prop(name: &Utf8CStr, mut prop_cb: PropCb) -> LoggedResult<()> {
    if check_pb() {
        pb_get_prop(&mut MatchNameCb { cb: prop_cb, name })
    } else {
        let mut value = file_get_prop(name)?;
        prop_cb.exec(name, Utf8CStr::from_string(&mut value));
        debug!("resetprop: found prop [{}] = [{}]", name, value);
        Ok(())
    }
}

fn do_persist_get_props(mut prop_cb: PropCb) -> LoggedResult<()> {
    if check_pb() {
        pb_get_prop(&mut prop_cb)
    } else {
        let mut dir = Directory::open(cstr!(PERSIST_PROP_DIR!()))?;
        dir.for_all_file(|f| {
            if let Ok(name) = Utf8CStr::from_bytes(f.d_name().to_bytes()) {
                if let Ok(mut value) = file_get_prop(name) {
                    prop_cb.exec(name, Utf8CStr::from_string(&mut value));
                }
            }
            Ok(WalkResult::Continue)
        })?;
        Ok(())
    }
}

fn do_persist_delete_prop(name: &Utf8CStr) -> LoggedResult<()> {
    if check_pb() {
        let mut pp = PersistentProperties { properties: vec![] };
        pb_get_prop(&mut PropCollectCb {
            props: &mut pp,
            replace_name: Some(name),
            replace_value: None,
        })?;
        pb_write_props(&pp)
    } else {
        file_set_prop(name, None)
    }
}

fn do_persist_set_prop(name: &Utf8CStr, value: &Utf8CStr) -> LoggedResult<()> {
    if check_pb() {
        let mut pp = PersistentProperties { properties: vec![] };
        pb_get_prop(&mut PropCollectCb {
            props: &mut pp,
            replace_name: Some(name),
            replace_value: Some(value),
        })?;
        pb_write_props(&pp)
    } else {
        file_set_prop(name, Some(value))
    }
}

pub unsafe fn persist_get_prop(name: *const c_char, prop_cb: *mut prop_cb) {
    unsafe fn inner(name: *const c_char, prop_cb: *mut prop_cb) -> LoggedResult<()> {
        do_persist_get_prop(Utf8CStr::from_ptr(name)?, PropCb { cb: prop_cb })
    }
    inner(name, prop_cb).ok();
}

pub unsafe fn persist_get_props(prop_cb: *mut prop_cb) {
    do_persist_get_props(PropCb { cb: prop_cb }).ok();
}

pub unsafe fn persist_delete_prop(name: *const c_char) -> bool {
    unsafe fn inner(name: *const c_char) -> LoggedResult<()> {
        do_persist_delete_prop(Utf8CStr::from_ptr(name)?)
    }
    inner(name).is_ok()
}
pub unsafe fn persist_set_prop(name: *const c_char, value: *const c_char) -> bool {
    unsafe fn inner(name: *const c_char, value: *const c_char) -> LoggedResult<()> {
        do_persist_set_prop(Utf8CStr::from_ptr(name)?, Utf8CStr::from_ptr(value)?)
    }
    inner(name, value).is_ok()
}
