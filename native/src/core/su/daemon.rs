use crate::UCred;
use crate::daemon::{AID_ROOT, AID_SHELL, MagiskD, to_app_id, to_user_id};
use crate::db::{DbSettings, MultiuserMode, RootAccess};
use crate::ffi::{
    SuAppRequest, SuPolicy, SuRequest, app_log, app_notify, app_request, exec_root_shell,
};
use crate::socket::IpcRead;
use crate::su::db::RootSettings;
use base::{LoggedResult, ResultExt, WriteExt, debug, error, exit_on_error, libc, warn};
use std::fs::File;
use std::os::fd::{FromRawFd, IntoRawFd};
use std::os::unix::net::UnixStream;
use std::sync::{Arc, Mutex};
use std::time::{Duration, Instant};

const DEFAULT_SHELL: &str = "/system/bin/sh";

impl Default for SuRequest {
    fn default() -> Self {
        SuRequest {
            target_uid: AID_ROOT,
            target_pid: -1,
            login: false,
            keep_env: false,
            drop_cap: false,
            shell: DEFAULT_SHELL.to_string(),
            command: "".to_string(),
            context: "".to_string(),
            gids: vec![],
        }
    }
}

pub struct SuInfo {
    uid: i32,
    eval_uid: i32,
    cfg: DbSettings,
    mgr_pkg: String,
    mgr_uid: i32,
    access: Mutex<AccessInfo>,
}

struct AccessInfo {
    settings: RootSettings,
    timestamp: Instant,
}

impl Default for SuInfo {
    fn default() -> Self {
        SuInfo {
            uid: -1,
            eval_uid: -1,
            cfg: Default::default(),
            mgr_pkg: Default::default(),
            mgr_uid: -1,
            access: Default::default(),
        }
    }
}

impl Default for AccessInfo {
    fn default() -> Self {
        AccessInfo {
            settings: Default::default(),
            timestamp: Instant::now(),
        }
    }
}

impl SuInfo {
    fn allow(uid: i32) -> SuInfo {
        let access = RootSettings {
            policy: SuPolicy::Allow,
            log: false,
            notify: false,
        };
        SuInfo {
            uid,
            access: Mutex::new(AccessInfo::new(access)),
            ..Default::default()
        }
    }

    fn deny(uid: i32) -> SuInfo {
        let access = RootSettings {
            policy: SuPolicy::Deny,
            log: false,
            notify: false,
        };
        SuInfo {
            uid,
            access: Mutex::new(AccessInfo::new(access)),
            ..Default::default()
        }
    }
}

impl AccessInfo {
    fn new(settings: RootSettings) -> AccessInfo {
        AccessInfo {
            settings,
            timestamp: Instant::now(),
        }
    }

    fn is_fresh(&self) -> bool {
        self.timestamp.elapsed() < Duration::from_secs(3)
    }

    fn refresh(&mut self) {
        self.timestamp = Instant::now();
    }
}

impl MagiskD {
    pub fn su_daemon_handler(&self, client: i32, cred: &UCred) {
        let cred = cred.0;
        debug!(
            "su: request from uid=[{}], pid=[{}], client=[{}]",
            cred.uid, cred.pid, client
        );

        let mut client = unsafe { UnixStream::from_raw_fd(client) };

        let mut req = match client.read_decodable::<SuRequest>().log() {
            Ok(req) => req,
            Err(_) => {
                warn!("su: remote process probably died, abort");
                client.write_pod(&SuPolicy::Deny.repr).ok();
                return;
            }
        };

        let info = self.get_su_info(cred.uid as i32);
        let app_req = SuAppRequest {
            uid: cred.uid as i32,
            pid: cred.pid,
            eval_uid: info.eval_uid,
            mgr_pkg: &info.mgr_pkg,
            mgr_uid: info.mgr_uid,
            request: &req,
        };

        {
            let mut access = info.access.lock().unwrap();

            if access.settings.policy == SuPolicy::Query {
                let fd = app_request(&app_req);
                if fd < 0 {
                    access.settings.policy = SuPolicy::Deny;
                } else {
                    let mut fd = unsafe { File::from_raw_fd(fd) };
                    access.settings.policy = SuPolicy {
                        repr: fd
                            .read_decodable::<i32>()
                            .log()
                            .map(i32::from_be)
                            .unwrap_or(SuPolicy::Deny.repr),
                    };
                }
            }

            if access.settings.log {
                app_log(&app_req, access.settings.policy, access.settings.notify);
            } else if access.settings.notify {
                app_notify(&app_req, access.settings.policy);
            }

            // Before unlocking, refresh the timestamp
            access.refresh();

            if access.settings.policy == SuPolicy::Restrict {
                req.drop_cap = true;
            }

            if access.settings.policy == SuPolicy::Deny {
                warn!("su: request rejected ({})", info.uid);
                client.write_pod(&SuPolicy::Deny.repr).ok();
                return;
            }
        }

        // At this point, the root access is granted.
        // Fork a child root process and monitor its exit value.
        let child = unsafe { libc::fork() };
        if child == 0 {
            debug!("su: fork handler");

            // Abort upon any error occurred
            exit_on_error(true);

            // ack
            client.write_pod(&0).ok();

            exec_root_shell(client.into_raw_fd(), cred.pid, &mut req, info.cfg.mnt_ns);
            return;
        }
        if child < 0 {
            error!("su: fork failed, abort");
            return;
        }

        // Wait result
        debug!("su: waiting child pid=[{}]", child);
        let mut status = 0;
        let code = unsafe {
            if libc::waitpid(child, &mut status, 0) > 0 {
                libc::WEXITSTATUS(status)
            } else {
                -1
            }
        };
        debug!("su: return code=[{}]", code);
        client.write_pod(&code).ok();
    }

    fn get_su_info(&self, uid: i32) -> Arc<SuInfo> {
        if uid == AID_ROOT {
            return Arc::new(SuInfo::allow(AID_ROOT));
        }

        let cached = self.cached_su_info.load();
        if cached.uid == uid && cached.access.lock().unwrap().is_fresh() {
            return cached;
        }

        let info = self.build_su_info(uid);
        self.cached_su_info.store(info.clone());
        info
    }

    fn build_su_info(&self, uid: i32) -> Arc<SuInfo> {
        let result: LoggedResult<Arc<SuInfo>> = try {
            let cfg = self.get_db_settings()?;

            // Check multiuser settings
            let eval_uid = match cfg.multiuser_mode {
                MultiuserMode::OwnerOnly => {
                    if to_user_id(uid) != 0 {
                        return Arc::new(SuInfo::deny(uid));
                    }
                    uid
                }
                MultiuserMode::OwnerManaged => to_app_id(uid),
                _ => uid,
            };

            let mut access = RootSettings::default();
            self.get_root_settings(eval_uid, &mut access)?;

            // We need to talk to the manager, get the app info
            let (mgr_uid, mgr_pkg) =
                if access.policy == SuPolicy::Query || access.log || access.notify {
                    self.get_manager(to_user_id(eval_uid), true)
                } else {
                    (-1, String::new())
                };

            // If it's the manager, allow it silently
            if to_app_id(uid) == to_app_id(mgr_uid) {
                return Arc::new(SuInfo::allow(uid));
            }

            // Check su access settings
            match cfg.root_access {
                RootAccess::Disabled => {
                    warn!("Root access is disabled!");
                    return Arc::new(SuInfo::deny(uid));
                }
                RootAccess::AdbOnly => {
                    if uid != AID_SHELL {
                        warn!("Root access limited to ADB only!");
                        return Arc::new(SuInfo::deny(uid));
                    }
                }
                RootAccess::AppsOnly => {
                    if uid == AID_SHELL {
                        warn!("Root access is disabled for ADB!");
                        return Arc::new(SuInfo::deny(uid));
                    }
                }
                _ => {}
            };

            // If still not determined, check if manager exists
            if access.policy == SuPolicy::Query && mgr_uid < 0 {
                return Arc::new(SuInfo::deny(uid));
            }

            // Finally, the SuInfo
            Arc::new(SuInfo {
                uid,
                eval_uid,
                cfg,
                mgr_pkg,
                mgr_uid,
                access: Mutex::new(AccessInfo::new(access)),
            })
        };

        result.unwrap_or(Arc::new(SuInfo::deny(uid)))
    }
}
