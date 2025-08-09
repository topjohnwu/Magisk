use crate::consts::{INTERNAL_DIR, MAGISK_FILE_CON};
use crate::daemon::{MagiskD, to_user_id};
use crate::ffi::{SuAppRequest, SuPolicy, get_magisk_tmp};
use ExtraVal::{Bool, Int, IntList, Str};
use base::{
    BytesExt, FileAttr, LibcReturn, LoggedResult, OsError, ResultExt, cstr, fork_dont_care, info,
    libc,
};
use libc::pollfd as PollFd;
use num_traits::AsPrimitive;
use std::{fmt::Write, fs::File, os::fd::AsRawFd, process::Command, process::exit};

struct Extra {
    key: &'static str,
    value: ExtraVal,
}

enum ExtraVal {
    Int(i32),
    Bool(bool),
    Str(String),
    IntList(Vec<u32>),
}

impl Extra {
    fn add_intent(&self, cmd: &mut Command) {
        match &self.value {
            Int(i) => {
                cmd.args(["--ei", self.key, &i.to_string()]);
            }
            Bool(b) => {
                cmd.args(["--ez", self.key, &b.to_string()]);
            }
            Str(s) => {
                cmd.args(["--es", self.key, s]);
            }
            IntList(list) => {
                cmd.args(["--es", self.key]);
                let mut tmp = String::new();
                list.iter().for_each(|i| write!(&mut tmp, "{i},").unwrap());
                tmp.pop();
                cmd.arg(&tmp);
            }
        }
    }

    fn add_bind(&self, cmd: &mut Command) {
        let mut tmp: String;
        match &self.value {
            Int(i) => {
                tmp = format!("{}:i:{}", self.key, i);
            }
            Bool(b) => {
                tmp = format!("{}:b:{}", self.key, b);
            }
            Str(s) => {
                let s = s.replace("\\", "\\\\").replace(":", "\\:");
                tmp = format!("{}:s:{}", self.key, s);
            }
            IntList(list) => {
                tmp = format!("{}:s:", self.key);
                if !list.is_empty() {
                    list.iter().for_each(|i| write!(&mut tmp, "{i},").unwrap());
                    tmp.pop();
                }
            }
        }
        cmd.args(["--extra", &tmp]);
    }

    fn add_bind_legacy(&self, cmd: &mut Command) {
        match &self.value {
            Str(s) => {
                let tmp = format!("{}:s:{}", self.key, s);
                cmd.args(["--extra", &tmp]);
            }
            _ => self.add_bind(cmd),
        }
    }
}

impl MagiskD {
    fn exec_cmd(
        &self,
        action: &'static str,
        extras: &[Extra],
        info: &SuAppRequest,
        use_provider: bool,
    ) {
        let user = to_user_id(info.eval_uid);
        let user = user.to_string();

        if use_provider {
            let provider = format!("content://{}.provider", info.mgr_pkg);
            let mut cmd = Command::new("/system/bin/app_process");
            cmd.args([
                "/system/bin",
                "com.android.commands.content.Content",
                "call",
                "--uri",
                &provider,
                "--user",
                &user,
                "--method",
                action,
            ]);
            if self.sdk_int() >= 30 {
                extras.iter().for_each(|e| e.add_bind(&mut cmd))
            } else {
                extras.iter().for_each(|e| e.add_bind_legacy(&mut cmd))
            }
            cmd.env("CLASSPATH", "/system/framework/content.jar");

            if let Ok(output) = cmd.output()
                && !output.stderr.contains(b"Error")
                && !output.stdout.contains(b"Error")
            {
                // The provider call succeed
                return;
            }
        }

        let mut cmd = Command::new("/system/bin/app_process");
        cmd.args([
            "/system/bin",
            "com.android.commands.am.Am",
            "start",
            "-p",
            info.mgr_pkg,
            "--user",
            &user,
            "-a",
            "android.intent.action.VIEW",
            "-f",
            // FLAG_ACTIVITY_NEW_TASK|FLAG_ACTIVITY_MULTIPLE_TASK|
            // FLAG_ACTIVITY_EXCLUDE_FROM_RECENTS|FLAG_INCLUDE_STOPPED_PACKAGES
            "0x18800020",
            "--es",
            "action",
            action,
        ]);
        extras.iter().for_each(|e| e.add_intent(&mut cmd));
        cmd.env("CLASSPATH", "/system/framework/am.jar");

        // Sometimes `am start` will fail, keep trying until it works
        loop {
            if let Ok(output) = cmd.output()
                && !output.stdout.is_empty()
            {
                break;
            }
        }
    }

    pub fn app_request(&self, info: &SuAppRequest) -> LoggedResult<File> {
        let mut fifo = cstr::buf::new::<64>();
        fifo.write_fmt(format_args!(
            "{}/{}/su_request_{}",
            get_magisk_tmp(),
            INTERNAL_DIR,
            info.pid
        ))
        .ok();

        let fd: LoggedResult<File> = try {
            let mut attr = FileAttr::new();
            attr.st.st_mode = 0o600;
            attr.st.st_uid = info.mgr_uid.as_();
            attr.st.st_gid = info.mgr_uid.as_();
            attr.con.write_str(MAGISK_FILE_CON).ok();

            fifo.mkfifo(0o600)?;
            fifo.set_attr(&attr)?;

            let extras = [
                Extra {
                    key: "fifo",
                    value: Str(fifo.to_string()),
                },
                Extra {
                    key: "uid",
                    value: Int(info.eval_uid),
                },
                Extra {
                    key: "pid",
                    value: Int(info.pid),
                },
            ];
            self.exec_cmd("request", &extras, info, false);

            // Open with O_RDWR to prevent FIFO open block
            let fd = fifo.open(libc::O_RDWR | libc::O_CLOEXEC)?;

            // Wait for data input for at most 70 seconds
            let mut pfd = PollFd {
                fd: fd.as_raw_fd(),
                events: libc::POLLIN,
                revents: 0,
            };
            if unsafe { libc::poll(&mut pfd, 1, 70 * 1000).as_os_result("poll", None, None)? } == 0
            {
                Err(OsError::with_os_error(libc::ETIMEDOUT, "poll", None, None))?;
            }

            fd
        };

        fifo.remove().log_ok();
        fd
    }

    pub fn app_notify(&self, info: &SuAppRequest, policy: SuPolicy) {
        if fork_dont_care() != 0 {
            return;
        }
        let extras = [
            Extra {
                key: "from.uid",
                value: Int(info.uid),
            },
            Extra {
                key: "pid",
                value: Int(info.pid),
            },
            Extra {
                key: "policy",
                value: Int(policy.repr),
            },
        ];
        self.exec_cmd("notify", &extras, info, true);
        exit(0);
    }

    pub fn app_log(&self, info: &SuAppRequest, policy: SuPolicy, notify: bool) {
        if fork_dont_care() != 0 {
            return;
        }
        let command = if info.request.command.is_empty() {
            &info.request.shell
        } else {
            &info.request.command
        };
        let extras = [
            Extra {
                key: "from.uid",
                value: Int(info.uid),
            },
            Extra {
                key: "to.uid",
                value: Int(info.request.target_uid),
            },
            Extra {
                key: "pid",
                value: Int(info.pid),
            },
            Extra {
                key: "policy",
                value: Int(policy.repr),
            },
            Extra {
                key: "target",
                value: Int(info.request.target_pid),
            },
            Extra {
                key: "context",
                value: Str(info.request.context.clone()),
            },
            Extra {
                key: "gids",
                value: IntList(info.request.gids.clone()),
            },
            Extra {
                key: "command",
                value: Str(command.clone()),
            },
            Extra {
                key: "notify",
                value: Bool(notify),
            },
        ];
        self.exec_cmd("log", &extras, info, true);
        exit(0);
    }
}
