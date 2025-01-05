use crate::daemon::MagiskD;
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
}

pub fn get_default_root_settings() -> RootSettings {
    RootSettings::default()
}
