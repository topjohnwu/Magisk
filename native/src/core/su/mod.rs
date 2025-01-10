use crate::daemon::{
    to_app_id, to_user_id, MagiskD, AID_APP_END, AID_APP_START, AID_ROOT, AID_SHELL,
};
use crate::db::DbArg::Integer;
use crate::db::{SqlTable, SqliteResult, SqliteReturn};
use crate::ffi::{DbValues, MultiuserMode, RootAccess, RootSettings, SuPolicy};
use base::ResultExt;

impl Default for SuPolicy {
    fn default() -> Self {
        SuPolicy::Query
    }
}

impl Default for RootSettings {
    fn default() -> Self {
        RootSettings {
            policy: Default::default(),
            log: true,
            notify: true,
        }
    }
}

impl SqlTable for RootSettings {
    fn on_row(&mut self, columns: &[String], values: &DbValues) {
        for (i, column) in columns.iter().enumerate() {
            let val = values.get_int(i as i32);
            if column == "policy" {
                self.policy.repr = val;
            } else if column == "logging" {
                self.log = val != 0;
            } else if column == "notify" {
                self.notify = val != 0;
            }
        }
    }
}

struct UidList(Vec<i32>);

impl SqlTable for UidList {
    fn on_row(&mut self, _: &[String], values: &DbValues) {
        self.0.push(values.get_int(0));
    }
}

impl MagiskD {
    fn get_root_settings(&self, uid: i32, settings: &mut RootSettings) -> SqliteResult<()> {
        self.db_exec_with_rows(
            "SELECT policy, logging, notification FROM policies \
             WHERE uid=? AND (until=0 OR until>strftime('%s', 'now'))",
            &[Integer(uid as i64)],
            settings,
        )
        .sql_result()
    }

    pub fn get_root_settings_for_cxx(&self, uid: i32, settings: &mut RootSettings) -> bool {
        self.get_root_settings(uid, settings).log().is_ok()
    }

    pub fn prune_su_access(&self) {
        let mut list = UidList(Vec::new());
        if self
            .db_exec_with_rows("SELECT uid FROM policies", &[], &mut list)
            .sql_result()
            .log()
            .is_err()
        {
            return;
        }

        let app_list = self.get_app_no_list();
        let mut rm_uids = Vec::new();

        for uid in list.0 {
            let app_id = to_app_id(uid);
            if (AID_APP_START..=AID_APP_END).contains(&app_id) {
                let app_no = app_id - AID_APP_START;
                if !app_list.contains(app_no as usize) {
                    // The app_id is no longer installed
                    rm_uids.push(uid);
                }
            }
        }

        for uid in rm_uids {
            self.db_exec("DELETE FROM policies WHERE uid=?", &[Integer(uid as i64)]);
        }
    }

    pub fn uid_granted_root(&self, mut uid: i32) -> bool {
        if uid == AID_ROOT {
            return true;
        }

        let cfg = match self.get_db_settings().log() {
            Ok(cfg) => cfg,
            Err(_) => return false,
        };

        // Check user root access settings
        match cfg.root_access {
            RootAccess::Disabled => return false,
            RootAccess::AppsOnly => {
                if uid == AID_SHELL {
                    return false;
                }
            }
            RootAccess::AdbOnly => {
                if uid != AID_SHELL {
                    return false;
                }
            }
            _ => {}
        }

        // Check multiuser settings
        match cfg.multiuser_mode {
            MultiuserMode::OwnerOnly => {
                if to_user_id(uid) != 0 {
                    return false;
                }
            }
            MultiuserMode::OwnerManaged => uid = to_app_id(uid),
            _ => {}
        }

        let mut granted = false;
        let mut output_fn =
            |_: &[String], values: &DbValues| granted = values.get_int(0) == SuPolicy::Allow.repr;
        self.db_exec_with_rows(
            "SELECT policy FROM policies WHERE uid=? AND (until=0 OR until>strftime('%s', 'now'))",
            &[Integer(uid as i64)],
            &mut output_fn,
        );

        granted
    }
}

pub fn get_default_root_settings() -> RootSettings {
    RootSettings::default()
}
