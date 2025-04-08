#![allow(improper_ctypes, improper_ctypes_definitions)]
use crate::daemon::{MAGISKD, MagiskD};
use crate::ffi::{
    DbEntryKey, DbStatement, DbValues, MntNsMode, open_and_init_db, sqlite3, sqlite3_errstr,
};
use crate::socket::{IpcRead, IpcWrite};
use DbArg::{Integer, Text};
use base::{LoggedResult, ResultExt, Utf8CStr};
use num_derive::FromPrimitive;
use num_traits::FromPrimitive;
use std::ffi::c_void;
use std::fs::File;
use std::io::{BufReader, BufWriter};
use std::os::fd::{FromRawFd, OwnedFd, RawFd};
use std::pin::Pin;
use std::ptr;
use std::ptr::NonNull;
use thiserror::Error;

fn sqlite_err_str(code: i32) -> &'static Utf8CStr {
    // SAFETY: sqlite3 always returns UTF-8 strings
    unsafe { Utf8CStr::from_ptr_unchecked(sqlite3_errstr(code)) }
}

#[repr(transparent)]
#[derive(Error, Debug)]
#[error("sqlite3: {}", sqlite_err_str(self.0))]
pub struct SqliteError(i32);

pub type SqliteResult<T> = Result<T, SqliteError>;

pub trait SqliteReturn {
    fn sql_result(self) -> SqliteResult<()>;
}

impl SqliteReturn for i32 {
    fn sql_result(self) -> SqliteResult<()> {
        if self != 0 {
            Err(SqliteError(self))
        } else {
            Ok(())
        }
    }
}

pub trait SqlTable {
    fn on_row(&mut self, columns: &[String], values: &DbValues);
}

impl<T> SqlTable for T
where
    T: FnMut(&[String], &DbValues),
{
    fn on_row(&mut self, columns: &[String], values: &DbValues) {
        self.call_mut((columns, values))
    }
}

#[derive(Default)]
pub struct DbSettings {
    pub root_access: RootAccess,
    pub multiuser_mode: MultiuserMode,
    pub mnt_ns: MntNsMode,
    pub boot_count: i32,
    pub denylist: bool,
    pub zygisk: bool,
}

#[repr(i32)]
#[derive(Default, FromPrimitive)]
pub enum RootAccess {
    Disabled,
    AppsOnly,
    AdbOnly,
    #[default]
    AppsAndAdb,
}

#[repr(i32)]
#[derive(Default, FromPrimitive)]
pub enum MultiuserMode {
    #[default]
    OwnerOnly,
    OwnerManaged,
    User,
}

impl Default for MntNsMode {
    fn default() -> Self {
        MntNsMode::Requester
    }
}

impl DbEntryKey {
    fn to_str(self) -> &'static str {
        match self {
            DbEntryKey::RootAccess => "root_access",
            DbEntryKey::SuMultiuserMode => "multiuser_mode",
            DbEntryKey::SuMntNs => "mnt_ns",
            DbEntryKey::DenylistConfig => "denylist",
            DbEntryKey::ZygiskConfig => "zygisk",
            DbEntryKey::BootloopCount => "bootloop",
            DbEntryKey::SuManager => "requester",
            _ => "",
        }
    }
}

impl SqlTable for DbSettings {
    fn on_row(&mut self, columns: &[String], values: &DbValues) {
        let mut key = "";
        let mut value = 0;
        for (i, column) in columns.iter().enumerate() {
            if column == "key" {
                key = values.get_text(i as i32);
            } else if column == "value" {
                value = values.get_int(i as i32);
            }
        }
        match key {
            "root_access" => self.root_access = RootAccess::from_i32(value).unwrap_or_default(),
            "multiuser_mode" => {
                self.multiuser_mode = MultiuserMode::from_i32(value).unwrap_or_default()
            }
            "mnt_ns" => self.mnt_ns = MntNsMode { repr: value },
            "denylist" => self.denylist = value != 0,
            "zygisk" => self.zygisk = value != 0,
            "bootloop" => self.boot_count = value,
            _ => {}
        }
    }
}

#[repr(transparent)]
pub struct Sqlite3(NonNull<sqlite3>);
unsafe impl Send for Sqlite3 {}

type SqlBindCallback = Option<unsafe extern "C" fn(*mut c_void, i32, Pin<&mut DbStatement>) -> i32>;
type SqlExecCallback = Option<unsafe extern "C" fn(*mut c_void, &[String], &DbValues)>;

unsafe extern "C" {
    fn sql_exec_impl(
        db: *mut sqlite3,
        sql: &str,
        bind_callback: SqlBindCallback,
        bind_cookie: *mut c_void,
        exec_callback: SqlExecCallback,
        exec_cookie: *mut c_void,
    ) -> i32;
}

pub enum DbArg<'a> {
    Text(&'a str),
    Integer(i64),
}

struct DbArgs<'a> {
    args: &'a [DbArg<'a>],
    curr: usize,
}

unsafe extern "C" fn bind_arguments(v: *mut c_void, idx: i32, stmt: Pin<&mut DbStatement>) -> i32 {
    unsafe {
        let args = &mut *(v as *mut DbArgs<'_>);
        if args.curr < args.args.len() {
            let arg = &args.args[args.curr];
            args.curr += 1;
            match *arg {
                Text(v) => stmt.bind_text(idx, v),
                Integer(v) => stmt.bind_int64(idx, v),
            }
        } else {
            0
        }
    }
}

unsafe extern "C" fn read_db_row<T: SqlTable>(
    v: *mut c_void,
    columns: &[String],
    values: &DbValues,
) {
    unsafe {
        let table = &mut *(v as *mut T);
        table.on_row(columns, values);
    }
}

impl MagiskD {
    fn with_db<F: FnOnce(*mut sqlite3) -> i32>(&self, f: F) -> i32 {
        let mut db = self.sql_connection.lock().unwrap();
        if db.is_none() {
            let raw_db = open_and_init_db();
            *db = NonNull::new(raw_db).map(Sqlite3);
        }
        match *db {
            Some(ref mut db) => f(db.0.as_ptr()),
            _ => -1,
        }
    }

    fn db_exec_impl(
        &self,
        sql: &str,
        args: &[DbArg],
        exec_callback: SqlExecCallback,
        exec_cookie: *mut c_void,
    ) -> i32 {
        let mut bind_callback: SqlBindCallback = None;
        let mut bind_cookie: *mut c_void = ptr::null_mut();
        let mut db_args = DbArgs { args, curr: 0 };
        if !args.is_empty() {
            bind_callback = Some(bind_arguments);
            bind_cookie = (&mut db_args) as *mut DbArgs as *mut c_void;
        }
        self.with_db(|db| unsafe {
            sql_exec_impl(
                db,
                sql,
                bind_callback,
                bind_cookie,
                exec_callback,
                exec_cookie,
            )
        })
    }

    pub fn db_exec_with_rows<T: SqlTable>(&self, sql: &str, args: &[DbArg], out: &mut T) -> i32 {
        self.db_exec_impl(
            sql,
            args,
            Some(read_db_row::<T>),
            out as *mut T as *mut c_void,
        )
    }

    pub fn db_exec(&self, sql: &str, args: &[DbArg]) -> i32 {
        self.db_exec_impl(sql, args, None, ptr::null_mut())
    }

    pub fn set_db_setting(&self, key: DbEntryKey, value: i32) -> SqliteResult<()> {
        self.db_exec(
            "INSERT OR REPLACE INTO settings (key,value) VALUES(?,?)",
            &[Text(key.to_str()), Integer(value as i64)],
        )
        .sql_result()
    }

    pub fn get_db_setting(&self, key: DbEntryKey) -> i32 {
        // Get default values
        let mut val = match key {
            DbEntryKey::RootAccess => RootAccess::default() as i32,
            DbEntryKey::SuMultiuserMode => MultiuserMode::default() as i32,
            DbEntryKey::SuMntNs => MntNsMode::default().repr,
            DbEntryKey::DenylistConfig => 0,
            DbEntryKey::ZygiskConfig => self.is_emulator as i32,
            DbEntryKey::BootloopCount => 0,
            _ => -1,
        };
        let mut func = |_: &[String], values: &DbValues| {
            val = values.get_int(0);
        };
        self.db_exec_with_rows(
            "SELECT value FROM settings WHERE key=?",
            &[Text(key.to_str())],
            &mut func,
        )
        .sql_result()
        .log()
        .ok();
        val
    }

    pub fn get_db_settings(&self) -> SqliteResult<DbSettings> {
        let mut cfg = DbSettings {
            zygisk: self.is_emulator,
            ..Default::default()
        };
        self.db_exec_with_rows("SELECT * FROM settings", &[], &mut cfg)
            .sql_result()?;
        Ok(cfg)
    }

    pub fn get_db_string(&self, key: DbEntryKey) -> String {
        let mut val = "".to_string();
        let mut func = |_: &[String], values: &DbValues| {
            val.push_str(values.get_text(0));
        };
        self.db_exec_with_rows(
            "SELECT value FROM strings WHERE key=?",
            &[Text(key.to_str())],
            &mut func,
        )
        .sql_result()
        .log()
        .ok();
        val
    }

    pub fn rm_db_string(&self, key: DbEntryKey) -> SqliteResult<()> {
        self.db_exec("DELETE FROM strings WHERE key=?", &[Text(key.to_str())])
            .sql_result()
    }

    fn db_exec_for_client(&self, fd: OwnedFd) -> LoggedResult<()> {
        let mut file = File::from(fd);
        let mut reader = BufReader::new(&mut file);
        let sql: String = reader.read_decodable()?;
        let mut writer = BufWriter::new(&mut file);
        let mut output_fn = |columns: &[String], values: &DbValues| {
            let mut out = "".to_string();
            for (i, column) in columns.iter().enumerate() {
                if i != 0 {
                    out.push('|');
                }
                out.push_str(column);
                out.push('=');
                out.push_str(values.get_text(i as i32));
            }
            writer.write_encodable(&out).log_ok();
        };
        self.db_exec_with_rows(&sql, &[], &mut output_fn);
        writer.write_encodable("").log()
    }
}

impl MagiskD {
    pub fn set_db_setting_for_cxx(&self, key: DbEntryKey, value: i32) -> bool {
        self.set_db_setting(key, value).log().is_ok()
    }

    pub fn db_exec_for_cxx(&self, client_fd: RawFd) {
        // Take ownership
        let fd = unsafe { OwnedFd::from_raw_fd(client_fd) };
        self.db_exec_for_client(fd).ok();
    }
}

#[unsafe(export_name = "sql_exec_rs")]
unsafe extern "C" fn sql_exec_for_cxx(
    sql: &str,
    bind_callback: SqlBindCallback,
    bind_cookie: *mut c_void,
    exec_callback: SqlExecCallback,
    exec_cookie: *mut c_void,
) -> i32 {
    unsafe {
        MAGISKD.get().unwrap_unchecked().with_db(|db| {
            sql_exec_impl(
                db,
                sql,
                bind_callback,
                bind_cookie,
                exec_callback,
                exec_cookie,
            )
        })
    }
}
