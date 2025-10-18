use nix::fcntl::OFlag;
use quick_protobuf::{BytesReader, MessageRead, MessageWrite, Writer};
use std::fs::File;
use std::io::{BufWriter, Read, Write};
use std::os::fd::FromRawFd;

use crate::resetprop::PropReader;
use crate::resetprop::proto::persistent_properties::PersistentProperties;
use crate::resetprop::proto::persistent_properties::mod_PersistentProperties::PersistentPropertyRecord;
use base::const_format::concatcp;
use base::libc::mkstemp;
use base::{
    Directory, FsPathBuilder, LibcReturn, LoggedResult, MappedFile, SilentLogExt, Utf8CStr,
    Utf8CStrBuf, WalkResult, clone_attr, cstr, debug, log_err,
};

const PERSIST_PROP_DIR: &str = "/data/property";
const PERSIST_PROP: &str = concatcp!(PERSIST_PROP_DIR, "/persistent_properties");

trait PropExt {
    fn find_index(&self, name: &Utf8CStr) -> Result<usize, usize>;
    fn find(self, name: &Utf8CStr) -> Option<PersistentPropertyRecord>;
}

impl PropExt for PersistentProperties {
    fn find_index(&self, name: &Utf8CStr) -> Result<usize, usize> {
        self.properties
            .binary_search_by(|p| p.name.as_deref().cmp(&Some(name.as_str())))
    }

    fn find(self, name: &Utf8CStr) -> Option<PersistentPropertyRecord> {
        let idx = self.find_index(name).ok()?;
        self.properties.into_iter().nth(idx)
    }
}

fn check_proto() -> bool {
    cstr!(PERSIST_PROP).exists()
}

fn file_get_prop(name: &Utf8CStr) -> LoggedResult<String> {
    let path = cstr::buf::default()
        .join_path(PERSIST_PROP_DIR)
        .join_path(name);
    let mut file = path.open(OFlag::O_RDONLY | OFlag::O_CLOEXEC).silent()?;
    debug!("resetprop: read prop from [{}]", path);
    let mut s = String::new();
    file.read_to_string(&mut s)?;
    Ok(s)
}

fn file_set_prop(name: &Utf8CStr, value: Option<&Utf8CStr>) -> LoggedResult<()> {
    let path = cstr::buf::default()
        .join_path(PERSIST_PROP_DIR)
        .join_path(name);
    if let Some(value) = value {
        let mut tmp = cstr::buf::default()
            .join_path(PERSIST_PROP_DIR)
            .join_path("prop.XXXXXX");
        {
            let mut f = unsafe {
                mkstemp(tmp.as_mut_ptr())
                    .into_os_result("mkstemp", None, None)
                    .map(|fd| File::from_raw_fd(fd))?
            };
            f.write_all(value.as_bytes())?;
        }
        debug!("resetprop: write prop to [{}]", tmp);
        tmp.rename_to(&path)?
    } else {
        path.remove().silent()?;
        debug!("resetprop: unlink [{}]", path);
    }
    Ok(())
}

fn proto_read_props() -> LoggedResult<PersistentProperties> {
    debug!("resetprop: decode with protobuf [{}]", PERSIST_PROP);
    let m = MappedFile::open(cstr!(PERSIST_PROP))?;
    let m = m.as_ref();
    let mut r = BytesReader::from_bytes(m);
    let mut props = PersistentProperties::from_reader(&mut r, m)?;
    // Keep the list sorted for binary search
    props
        .properties
        .sort_unstable_by(|a, b| a.name.cmp(&b.name));
    Ok(props)
}

fn proto_write_props(props: &PersistentProperties) -> LoggedResult<()> {
    let mut tmp = cstr::buf::default().join_path(concatcp!(PERSIST_PROP, ".XXXXXX"));
    {
        let f = unsafe {
            mkstemp(tmp.as_mut_ptr())
                .into_os_result("mkstemp", None, None)
                .map(|fd| File::from_raw_fd(fd))?
        };
        debug!("resetprop: encode with protobuf [{}]", tmp);
        props.write_message(&mut Writer::new(BufWriter::new(f)))?;
    }
    clone_attr(cstr!(PERSIST_PROP), &tmp)?;
    tmp.rename_to(cstr!(PERSIST_PROP))?;
    Ok(())
}

pub(super) fn persist_get_prop(key: &Utf8CStr) -> LoggedResult<String> {
    if check_proto() {
        let props = proto_read_props()?;
        let prop = props.find(key).silent()?;
        if let PersistentPropertyRecord {
            name: Some(_),
            value: Some(v),
        } = prop
        {
            return Ok(v);
        }
    } else {
        let value = file_get_prop(key)?;
        debug!("resetprop: get persist prop [{}]=[{}]", key, value);
        return Ok(value);
    }
    log_err!()
}

pub(super) fn persist_get_all_props(reader: &mut PropReader) -> LoggedResult<()> {
    if check_proto() {
        let props = proto_read_props()?;
        props.properties.into_iter().for_each(|prop| {
            if let PersistentPropertyRecord {
                name: Some(n),
                value: Some(v),
            } = prop
            {
                reader.put_str(n, v, 0);
            }
        });
    } else {
        let mut dir = Directory::open(cstr!(PERSIST_PROP_DIR))?;
        dir.pre_order_walk(|e| {
            if e.is_file()
                && let Ok(value) = file_get_prop(e.name())
            {
                reader.put_str(e.name().to_string(), value, 0);
            }
            // Do not traverse recursively
            Ok(WalkResult::Skip)
        })?;
    }
    Ok(())
}

pub(super) fn persist_delete_prop(key: &Utf8CStr) -> LoggedResult<()> {
    if check_proto() {
        let mut props = proto_read_props()?;
        let idx = props.find_index(key).silent()?;
        props.properties.remove(idx);
        proto_write_props(&props)?;
    } else {
        file_set_prop(key, None)?;
    }
    Ok(())
}

pub(super) fn persist_set_prop(key: &Utf8CStr, val: &Utf8CStr) -> LoggedResult<()> {
    if check_proto() {
        let mut props = proto_read_props()?;
        match props.find_index(key) {
            Ok(idx) => props.properties[idx].value = Some(val.to_string()),
            Err(idx) => props.properties.insert(
                idx,
                PersistentPropertyRecord {
                    name: Some(key.to_string()),
                    value: Some(val.to_string()),
                },
            ),
        }
        proto_write_props(&props)?;
    } else {
        file_set_prop(key, Some(val))?;
    }
    Ok(())
}
