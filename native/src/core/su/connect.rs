use super::SuInfo;
use crate::daemon::{MagiskD, to_user_id};
use crate::ffi::{SuRequest, fork_dont_care};
use ExtraVal::{Bool, Int, IntList, Str};
use base::{BytesExt, error};
use nix::sys::signal::SigSet;
use num_traits::AsPrimitive;
use std::fmt::Write;
use std::os::unix::net::UCred;
use std::os::unix::process::CommandExt;
use std::process::{Command, exit};

fn app_process() -> Command {
    let mut cmd = Command::new("/system/bin/app_process");
    // daemon thread blocks all signals, unblock it before starting app_process
    unsafe {
        cmd.pre_exec(|| SigSet::empty().thread_set_mask().map_err(Into::into));
    }
    cmd
}

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
                    list.iter().for_each(|i| {
                        write!(&mut tmp, "{i},").ok();
                    });
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

impl MagiskD {
    fn exec_cmd(&self, eval_uid: i32, mgr_pkg: &str, action: &'static str, extras: &[Extra]) {
        let user = to_user_id(eval_uid).to_string();

        let provider = format!("content://{mgr_pkg}.provider");
        let mut cmd = app_process();
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

        match cmd.output() {
            Ok(output) => {
                if output.stderr.contains(b"Error") || output.stdout.contains(b"Error") {
                    error!(
                        "content call failed: {}",
                        String::from_utf8_lossy(&output.stderr)
                    );
                }
            }
            Err(e) => {
                error!("failed to execute content call: {e}");
            }
        }
    }

    fn app_notify(&self, cred: &UCred, info: &SuInfo) {
        let extras = [
            Extra {
                key: "from.uid",
                value: Int(cred.uid.as_()),
            },
            Extra {
                key: "pid",
                value: Int(cred.pid.unwrap_or(-1).as_()),
            },
            Extra {
                key: "policy",
                value: Int(info.settings.policy.repr),
            },
        ];
        self.exec_cmd(info.eval_uid, &info.mgr_pkg, "notify", &extras);
    }

    fn app_log(&self, cred: &UCred, info: &SuInfo, request: &SuRequest) {
        let command = if request.command.is_empty() {
            &request.shell
        } else {
            &request.command
        };
        let extras = [
            Extra {
                key: "from.uid",
                value: Int(cred.uid.as_()),
            },
            Extra {
                key: "to.uid",
                value: Int(request.target_uid),
            },
            Extra {
                key: "pid",
                value: Int(cred.pid.unwrap_or(-1).as_()),
            },
            Extra {
                key: "policy",
                value: Int(info.settings.policy.repr),
            },
            Extra {
                key: "target",
                value: Int(request.target_pid),
            },
            Extra {
                key: "context",
                value: Str(&request.context),
            },
            Extra {
                key: "gids",
                value: IntList(&request.gids),
            },
            Extra {
                key: "command",
                value: Str(command),
            },
            Extra {
                key: "notify",
                value: Bool(info.settings.notify),
            },
        ];
        self.exec_cmd(info.eval_uid, &info.mgr_pkg, "log", &extras);
    }

    pub(super) fn notify_app(&self, cred: &UCred, info: &SuInfo, request: &SuRequest) {
        if info.mgr_pkg.is_empty() {
            return;
        }

        if !info.settings.log && !info.settings.notify {
            return;
        }

        if fork_dont_care() != 0 {
            return;
        }

        // Notify su usage to application
        if info.settings.log {
            self.app_log(cred, info, request);
        } else if info.settings.notify {
            self.app_notify(cred, info);
        }

        exit(0);
    }
}
