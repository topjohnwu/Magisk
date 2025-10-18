use super::SuInfo;
use super::db::RootSettings;
use crate::consts::{INTERNAL_DIR, MAGISK_FILE_CON};
use crate::daemon::to_user_id;
use crate::ffi::{SuPolicy, SuRequest, get_magisk_tmp};
use crate::socket::IpcRead;
use ExtraVal::{Bool, Int, IntList, Str};
use base::{
    BytesExt, FileAttr, LibcReturn, LoggedResult, ResultExt, Utf8CStrBuf, cstr, fork_dont_care,
};
use nix::fcntl::OFlag;
use nix::poll::{PollFd, PollFlags, PollTimeout};
use num_traits::AsPrimitive;
use std::fmt::Write;
use std::fs::File;
use std::os::fd::AsFd;
use std::os::unix::net::UCred;
use std::process::{Command, exit};

struct Extra<'a> {
    key: &'static str,
    value: ExtraVal<'a>,
}

enum ExtraVal<'a> {
    Int(i32),
    Bool(bool),
    Str(&'a str),
    IntList(&'a [u32]),
}

impl Extra<'_> {
    fn add_intent(&self, cmd: &mut Command) {
        match self.value {
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
        match self.value {
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
        match self.value {
            Str(s) => {
                let tmp = format!("{}:s:{}", self.key, s);
                cmd.args(["--extra", &tmp]);
            }
            _ => self.add_bind(cmd),
        }
    }
}

pub(super) struct SuAppContext<'a> {
    pub(super) cred: UCred,
    pub(super) request: &'a SuRequest,
    pub(super) info: &'a SuInfo,
    pub(super) settings: &'a mut RootSettings,
    pub(super) sdk_int: i32,
}

impl SuAppContext<'_> {
    fn exec_cmd(&self, action: &'static str, extras: &[Extra], use_provider: bool) {
        let user = to_user_id(self.info.eval_uid);
        let user = user.to_string();

        if use_provider {
            let provider = format!("content://{}.provider", self.info.mgr_pkg);
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
            if self.sdk_int >= 30 {
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
            &self.info.mgr_pkg,
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

    fn app_request(&mut self) {
        let mut fifo = cstr::buf::new::<64>();
        fifo.write_fmt(format_args!(
            "{}/{}/su_request_{}",
            get_magisk_tmp(),
            INTERNAL_DIR,
            self.cred.pid.unwrap_or(-1)
        ))
        .ok();

        let fd: LoggedResult<File> = try {
            let mut attr = FileAttr::new();
            attr.st.st_mode = 0o600;
            attr.st.st_uid = self.info.mgr_uid.as_();
            attr.st.st_gid = self.info.mgr_uid.as_();
            attr.con.push_str(MAGISK_FILE_CON);

            fifo.mkfifo(0o600)?;
            fifo.set_attr(&attr)?;

            let extras = [
                Extra {
                    key: "fifo",
                    value: Str(&fifo),
                },
                Extra {
                    key: "uid",
                    value: Int(self.info.eval_uid),
                },
                Extra {
                    key: "pid",
                    value: Int(self.cred.pid.unwrap_or(-1)),
                },
            ];
            self.exec_cmd("request", &extras, false);

            // Open with O_RDWR to prevent FIFO open block
            let fd = fifo.open(OFlag::O_RDWR | OFlag::O_CLOEXEC)?;
            let mut pfd = [PollFd::new(fd.as_fd(), PollFlags::POLLIN)];

            // Wait for data input for at most 70 seconds
            nix::poll::poll(&mut pfd, PollTimeout::try_from(70 * 1000).unwrap())
                .check_os_err("poll", None, None)?;
            fd
        };

        fifo.remove().log_ok();

        if let Ok(mut fd) = fd {
            self.settings.policy = SuPolicy {
                repr: fd
                    .read_decodable::<i32>()
                    .log()
                    .map(i32::from_be)
                    .unwrap_or(SuPolicy::Deny.repr),
            };
        } else {
            self.settings.policy = SuPolicy::Deny;
        };
    }

    fn app_notify(&self) {
        let extras = [
            Extra {
                key: "from.uid",
                value: Int(self.cred.uid.as_()),
            },
            Extra {
                key: "pid",
                value: Int(self.cred.pid.unwrap_or(-1).as_()),
            },
            Extra {
                key: "policy",
                value: Int(self.settings.policy.repr),
            },
        ];
        self.exec_cmd("notify", &extras, true);
    }

    fn app_log(&self) {
        let command = if self.request.command.is_empty() {
            &self.request.shell
        } else {
            &self.request.command
        };
        let extras = [
            Extra {
                key: "from.uid",
                value: Int(self.cred.uid.as_()),
            },
            Extra {
                key: "to.uid",
                value: Int(self.request.target_uid),
            },
            Extra {
                key: "pid",
                value: Int(self.cred.pid.unwrap_or(-1).as_()),
            },
            Extra {
                key: "policy",
                value: Int(self.settings.policy.repr),
            },
            Extra {
                key: "target",
                value: Int(self.request.target_pid),
            },
            Extra {
                key: "context",
                value: Str(&self.request.context),
            },
            Extra {
                key: "gids",
                value: IntList(&self.request.gids),
            },
            Extra {
                key: "command",
                value: Str(command),
            },
            Extra {
                key: "notify",
                value: Bool(self.settings.notify),
            },
        ];
        self.exec_cmd("log", &extras, true);
    }

    pub(super) fn connect_app(&mut self) {
        // If policy is undetermined, show dialog for user consent
        if self.settings.policy == SuPolicy::Query {
            self.app_request();
        }

        if !self.settings.log && !self.settings.notify {
            return;
        }

        if fork_dont_care() != 0 {
            return;
        }

        // Notify su usage to application
        if self.settings.log {
            self.app_log();
        } else if self.settings.notify {
            self.app_notify();
        }

        exit(0);
    }
}
