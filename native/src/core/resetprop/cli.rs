use super::persist::{
    persist_delete_prop, persist_get_all_props, persist_get_prop, persist_set_prop,
};
use super::{PropInfo, PropReader, SYS_PROP};
use argh::{EarlyExit, FromArgs, MissingRequirements};
use base::libc::PROP_VALUE_MAX;
use base::{
    BufReadExt, CmdArgs, EarlyExitExt, LogLevel, LoggedResult, ResultExt, Utf8CStr, Utf8CStrBuf,
    Utf8CString, argh, cstr, debug, log_err, set_log_level_state,
};
use nix::fcntl::OFlag;
use std::collections::BTreeMap;
use std::ffi::c_char;
use std::io::BufReader;

#[derive(FromArgs, Default)]
struct ResetProp {
    #[argh(switch, short = 'v')]
    verbose: bool,
    #[argh(switch, short = 'w', long = none)]
    wait_mode: bool,
    #[argh(switch, short = 'p', long = none)]
    persist: bool,
    #[argh(switch, short = 'P', long = none)]
    persist_only: bool,
    #[argh(switch, short = 'Z', long = none)]
    context: bool,
    #[argh(switch, short = 'n', long = none)]
    skip_svc: bool,
    #[argh(option, short = 'f')]
    file: Option<Utf8CString>,
    #[argh(option, short = 'd', long = "delete")]
    delete_key: Option<Utf8CString>,
    #[argh(positional, greedy = true)]
    args: Vec<Utf8CString>,
}

fn print_usage(cmd: &str) {
    eprintln!(
        r#"resetprop - System Property Manipulation Tool

Usage: {cmd} [flags] [arguments...]

Read mode arguments:
   (no arguments)    print all properties
   NAME              get property of NAME

Write mode arguments:
   NAME VALUE        set property NAME as VALUE
   -f,--file   FILE  load and set properties from FILE
   -d,--delete NAME  delete property

Wait mode arguments (toggled with -w):
    NAME             wait until property NAME changes
    NAME OLD_VALUE   if value of property NAME is not OLD_VALUE, get value
                     or else wait until property NAME changes

General flags:
   -h,--help         show this message
   -v,--verbose      print verbose output to stderr
   -w                switch to wait mode

Read mode flags:
   -p      also read persistent properties from storage
   -P      only read persistent properties from storage
   -Z      get property context instead of value

Write mode flags:
   -n      set properties bypassing property_service
   -p      always write persistent prop changes to storage
"#
    );
}

impl ResetProp {
    fn get(&self, key: &Utf8CStr) -> Option<String> {
        if self.context {
            return Some(SYS_PROP.get_context(key).to_string());
        }

        let mut val = if !self.persist_only {
            SYS_PROP.find(key).map(|info| {
                let mut v = String::new();
                info.read(&mut PropReader::Value(&mut v));
                debug!("resetprop: get prop [{key}]=[{v}]");
                v
            })
        } else {
            None
        };

        if val.is_none() && (self.persist || self.persist_only) && key.starts_with("persist.") {
            val = persist_get_prop(key).ok();
        }

        if val.is_none() {
            debug!("resetprop: prop [{key}] does not exist");
        }

        val
    }

    fn print_all(&self) {
        let mut map: BTreeMap<String, String> = BTreeMap::new();
        if !self.persist_only {
            SYS_PROP.for_each(&mut PropReader::List(&mut map));
        }
        if self.persist || self.persist_only {
            persist_get_all_props(&mut PropReader::List(&mut map)).log_ok();
        }
        for (mut k, v) in map.into_iter() {
            if self.context {
                println!(
                    "[{k}]: [{}]",
                    SYS_PROP.get_context(Utf8CStr::from_string(&mut k))
                );
            } else {
                println!("[{k}]: [{v}]");
            }
        }
    }

    fn set(&self, key: &Utf8CStr, val: &Utf8CStr) {
        let mut skip_svc = self.skip_svc;
        let mut info = SYS_PROP.find_mut(key);

        // Delete existing read-only properties if they are or will be long properties,
        // which cannot directly go through __system_property_update
        if key.starts_with("ro.") {
            skip_svc = true;
            if let Some(pi) = &info
                && (pi.is_long() || val.len() >= PROP_VALUE_MAX as usize)
            {
                // Skip pruning nodes as we will add it back ASAP
                SYS_PROP.delete(key, false);
                info = None;
            }
        }

        #[allow(unused_variables)]
        let msg = if skip_svc {
            "direct modification"
        } else {
            "property_service"
        };

        if let Some(pi) = info {
            if skip_svc {
                pi.update(val);
            } else {
                SYS_PROP.set(key, val);
            }
            debug!("resetprop: update prop [{key}]=[{val}] by {msg}");
        } else {
            if skip_svc {
                SYS_PROP.add(key, val);
            } else {
                SYS_PROP.set(key, val);
            }
            debug!("resetprop: create prop [{key}]=[{val}] by {msg}");
        }

        // When bypassing property_service, persistent props won't be stored in storage.
        // Explicitly handle this situation.
        if skip_svc && self.persist && key.starts_with("persist.") {
            persist_set_prop(key, val).log_ok();
        }
    }

    fn delete(&self, key: &Utf8CStr) -> bool {
        debug!("resetprop: delete prop [{key}]");
        let mut ret = false;
        ret |= SYS_PROP.delete(key, true);
        if self.persist && key.starts_with("persist.") {
            ret |= persist_delete_prop(key).is_ok()
        }
        ret
    }

    fn wait(&self) {
        let key = &self.args[0];
        let val = self.args.get(1).map(|s| &**s);

        // Find PropInfo
        let info: &PropInfo;
        loop {
            let i = SYS_PROP.find(key);
            if let Some(i) = i {
                info = i;
                break;
            } else {
                debug!("resetprop: waiting for prop [{key}] to exist");
                let mut serial = SYS_PROP.area_serial();
                SYS_PROP.wait(None, serial, &mut serial);
            }
        }

        if let Some(val) = val {
            let mut curr_val = String::new();
            let mut serial = 0;
            loop {
                let mut r = PropReader::ValueSerial(&mut curr_val, &mut serial);
                SYS_PROP.read(info, &mut r);
                if *val != *curr_val {
                    debug!("resetprop: get prop [{key}]=[{curr_val}]");
                    break;
                }
                debug!("resetprop: waiting for prop [{key}]!=[{val}]");
                SYS_PROP.wait(Some(info), serial, &mut serial);
            }
        }
    }

    fn load_file(&self, file: &Utf8CStr) -> LoggedResult<()> {
        let fd = file.open(OFlag::O_RDONLY | OFlag::O_CLOEXEC)?;
        let mut key = cstr::buf::dynamic(128);
        let mut val = cstr::buf::dynamic(128);
        BufReader::new(fd).for_each_prop(|k, v| {
            key.clear();
            val.clear();
            key.push_str(k);
            val.push_str(v);
            self.set(&key, &val);
            true
        });
        Ok(())
    }

    fn run(self) -> LoggedResult<()> {
        if self.wait_mode {
            self.wait();
        } else if let Some(file) = &self.file {
            self.load_file(file)?;
        } else if let Some(key) = &self.delete_key {
            if !self.delete(key) {
                return log_err!();
            }
        } else {
            match self.args.len() {
                0 => self.print_all(),
                1 => {
                    if let Some(val) = self.get(&self.args[0]) {
                        println!("{val}");
                    } else {
                        return log_err!();
                    }
                }
                2 => self.set(&self.args[0], &self.args[1]),
                _ => unreachable!(),
            }
        }
        Ok(())
    }
}

pub fn resetprop_main(argc: i32, argv: *mut *mut c_char) -> i32 {
    set_log_level_state(LogLevel::Debug, false);
    let cmds = CmdArgs::new(argc, argv.cast());
    let cmds = cmds.as_slice();

    let cli = ResetProp::from_args(&[cmds[0]], &cmds[1..])
        .and_then(|cli| {
            let mut special_mode = 0;
            if cli.wait_mode {
                if cli.args.is_empty() {
                    let mut missing = MissingRequirements::default();
                    missing.missing_positional_arg("NAME");
                    missing.err_on_any()?;
                }
                special_mode += 1;
            }
            if cli.file.is_some() {
                special_mode += 1;
            }
            if cli.delete_key.is_some() {
                special_mode += 1;
            }
            if special_mode > 1 {
                return Err(EarlyExit::from(
                    "Multiple operation mode detected!\n".to_string(),
                ));
            }
            if cli.args.len() > 2 {
                return Err(EarlyExit::from(format!(
                    "Unrecognized argument: {}\n",
                    cli.args[2]
                )));
            }
            Ok(cli)
        })
        .on_early_exit(|| print_usage(cmds[0]));

    if cli.verbose {
        set_log_level_state(LogLevel::Debug, true);
    }

    if cli.run().is_ok() { 0 } else { 1 }
}

// Magisk's own helper functions

pub fn set_prop(key: &Utf8CStr, val: &Utf8CStr) {
    let prop = ResetProp {
        // All Magisk's internal usage should skip property_service
        skip_svc: true,
        ..Default::default()
    };
    prop.set(key, val);
}

pub fn load_prop_file(file: &Utf8CStr) {
    let prop = ResetProp {
        // All Magisk's internal usage should skip property_service
        skip_svc: true,
        ..Default::default()
    };
    prop.load_file(file).ok();
}

pub fn get_prop(key: &Utf8CStr) -> String {
    let prop = ResetProp {
        persist: key.starts_with("persist."),
        ..Default::default()
    };
    prop.get(key).unwrap_or_default()
}
