use crate::daemon::{to_app_id, MagiskD, AID_APP_END, AID_APP_START};
use crate::db::DbArg::Integer;
use crate::db::{SqlTable, SqliteResult, SqliteReturn};
use crate::ffi::{DbValues, RootSettings, SuPolicy};
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
    fn get_root_settings(&self, uid: i32, settings: &mut RootSettings) -> SqliteResult {
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
            if app_id >= AID_APP_START && app_id <= AID_APP_END {
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
}

pub fn get_default_root_settings() -> RootSettings {
    RootSettings::default()
}
