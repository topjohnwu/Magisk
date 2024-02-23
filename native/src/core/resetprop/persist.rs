use std::io::Read;
use std::{
    fs::File,
    io::{BufWriter, Write},
    ops::{Deref, DerefMut},
    os::fd::FromRawFd,
    pin::Pin,
};

use quick_protobuf::{BytesReader, MessageRead, MessageWrite, Writer};

use base::libc::{O_CLOEXEC, O_RDONLY};
use base::{
    clone_attr, cstr, debug, libc::mkstemp, Directory, FsPath, FsPathBuf, LibcReturn, LoggedResult,
    MappedFile, ResultNoLog, Utf8CStr, Utf8CStrBufArr, WalkResult,
};

use crate::ffi::{prop_cb_exec, PropCb};
use crate::resetprop::proto::persistent_properties::{
    mod_PersistentProperties::PersistentPropertyRecord, PersistentProperties,
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

trait PropCbExec {
    fn exec(&mut self, name: &Utf8CStr, value: &Utf8CStr);
}

impl PropCbExec for Pin<&mut PropCb> {
    fn exec(&mut self, name: &Utf8CStr, value: &Utf8CStr) {
        unsafe { prop_cb_exec(self.as_mut(), name.as_ptr(), value.as_ptr(), u32::MAX) }
    }
}

impl Deref for PersistentProperties {
    type Target = Vec<PersistentPropertyRecord>;

    fn deref(&self) -> &Self::Target {
        &self.properties
    }
}

impl DerefMut for PersistentProperties {
    fn deref_mut(&mut self) -> &mut Self::Target {
        &mut self.properties
    }
}

trait PropExt {
    fn find_index(&self, name: &Utf8CStr) -> Result<usize, usize>;
    fn find(&mut self, name: &Utf8CStr) -> LoggedResult<&mut PersistentPropertyRecord>;
}

impl PropExt for PersistentProperties {
    fn find_index(&self, name: &Utf8CStr) -> Result<usize, usize> {
        self.binary_search_by(|p| p.name.as_deref().cmp(&Some(name.deref())))
    }

    fn find(&mut self, name: &Utf8CStr) -> LoggedResult<&mut PersistentPropertyRecord> {
        let idx = self.find_index(name).no_log()?;
        Ok(&mut self[idx])
    }
}

fn check_proto() -> bool {
    FsPath::from(cstr!(PERSIST_PROP!())).exists()
}

fn file_get_prop(name: &Utf8CStr) -> LoggedResult<String> {
    let mut buf = Utf8CStrBufArr::default();
    let path = FsPathBuf::new(&mut buf)
        .join(PERSIST_PROP_DIR!())
        .join(name);
    let mut file = path.open(O_RDONLY | O_CLOEXEC).no_log()?;
    debug!("resetprop: read prop from [{}]", path);
    let mut s = String::new();
    file.read_to_string(&mut s)?;
    Ok(s)
}

fn file_set_prop(name: &Utf8CStr, value: Option<&Utf8CStr>) -> LoggedResult<()> {
    let mut buf = Utf8CStrBufArr::default();
    let path = FsPathBuf::new(&mut buf)
        .join(PERSIST_PROP_DIR!())
        .join(name);
    if let Some(value) = value {
        let mut buf = Utf8CStrBufArr::default();
        let mut tmp = FsPathBuf::new(&mut buf)
            .join(PERSIST_PROP_DIR!())
            .join("prop.XXXXXX");
        {
            let mut f = unsafe {
                let fd = mkstemp(tmp.as_mut_ptr()).check_os_err()?;
                File::from_raw_fd(fd)
            };
            f.write_all(value.as_bytes())?;
        }
        debug!("resetprop: write prop to [{}]", tmp);
        tmp.rename_to(path)?
    } else {
        path.remove().no_log()?;
        debug!("resetprop: unlink [{}]", path);
    }
    Ok(())
}

fn proto_read_props() -> LoggedResult<PersistentProperties> {
    debug!("resetprop: decode with protobuf [{}]", PERSIST_PROP!());
    let m = MappedFile::open(cstr!(PERSIST_PROP!()))?;
    let m = m.as_ref();
    let mut r = BytesReader::from_bytes(m);
    let mut props = PersistentProperties::from_reader(&mut r, m)?;
    // Keep the list sorted for binary search
    props.sort_unstable_by(|a, b| a.name.cmp(&b.name));
    Ok(props)
}

fn proto_write_props(props: &PersistentProperties) -> LoggedResult<()> {
    let mut buf = Utf8CStrBufArr::default();
    let mut tmp = FsPathBuf::new(&mut buf).join(concat!(PERSIST_PROP!(), ".XXXXXX"));
    {
        let f = unsafe {
            let fd = mkstemp(tmp.as_mut_ptr()).check_os_err()?;
            File::from_raw_fd(fd)
        };
        debug!("resetprop: encode with protobuf [{}]", tmp);
        props.write_message(&mut Writer::new(BufWriter::new(f)))?;
    }
    clone_attr(FsPath::from(cstr!(PERSIST_PROP!())), &tmp)?;
    tmp.rename_to(cstr!(PERSIST_PROP!()))?;
    Ok(())
}

pub unsafe fn persist_get_prop(name: &Utf8CStr, prop_cb: Pin<&mut PropCb>) {
    fn inner(name: &Utf8CStr, mut prop_cb: Pin<&mut PropCb>) -> LoggedResult<()> {
        if check_proto() {
            let mut props = proto_read_props()?;
            let prop = props.find(name)?;
            if let PersistentPropertyRecord {
                name: Some(ref mut n),
                value: Some(ref mut v),
            } = prop
            {
                prop_cb.exec(Utf8CStr::from_string(n), Utf8CStr::from_string(v));
            }
        } else {
            let mut value = file_get_prop(name)?;
            prop_cb.exec(name, Utf8CStr::from_string(&mut value));
            debug!("resetprop: found prop [{}] = [{}]", name, value);
        }
        Ok(())
    }
    inner(name, prop_cb).ok();
}

pub unsafe fn persist_get_props(prop_cb: Pin<&mut PropCb>) {
    fn inner(mut prop_cb: Pin<&mut PropCb>) -> LoggedResult<()> {
        if check_proto() {
            let mut props = proto_read_props()?;
            props.iter_mut().for_each(|prop| {
                if let PersistentPropertyRecord {
                    name: Some(ref mut n),
                    value: Some(ref mut v),
                } = prop
                {
                    prop_cb.exec(Utf8CStr::from_string(n), Utf8CStr::from_string(v));
                }
            });
        } else {
            let mut dir = Directory::open(cstr!(PERSIST_PROP_DIR!()))?;
            dir.pre_order_walk(|e| {
                if e.is_file() {
                    if let Ok(name) = Utf8CStr::from_cstr(e.d_name()) {
                        if let Ok(mut value) = file_get_prop(name) {
                            prop_cb.exec(name, Utf8CStr::from_string(&mut value));
                        }
                    }
                }
                // Do not traverse recursively
                Ok(WalkResult::Skip)
            })?;
        }
        Ok(())
    }
    inner(prop_cb).ok();
}

pub unsafe fn persist_delete_prop(name: &Utf8CStr) -> bool {
    fn inner(name: &Utf8CStr) -> LoggedResult<()> {
        if check_proto() {
            let mut props = proto_read_props()?;
            let idx = props.find_index(name).no_log()?;
            props.remove(idx);
            proto_write_props(&props)
        } else {
            file_set_prop(name, None)
        }
    }
    inner(name).is_ok()
}

pub unsafe fn persist_set_prop(name: &Utf8CStr, value: &Utf8CStr) -> bool {
    unsafe fn inner(name: &Utf8CStr, value: &Utf8CStr) -> LoggedResult<()> {
        if check_proto() {
            let mut props = proto_read_props()?;
            match props.find_index(name) {
                Ok(idx) => props[idx].value = Some(value.to_string()),
                Err(idx) => props.insert(
                    idx,
                    PersistentPropertyRecord {
                        name: Some(name.to_string()),
                        value: Some(value.to_string()),
                    },
                ),
            }
            proto_write_props(&props)
        } else {
            file_set_prop(name, Some(value))
        }
    }
    inner(name, value).is_ok()
}
