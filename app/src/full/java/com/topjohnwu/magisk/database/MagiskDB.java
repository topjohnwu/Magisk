package com.topjohnwu.magisk.database;

import com.topjohnwu.magisk.Const;
import com.topjohnwu.magisk.Data;
import com.topjohnwu.magisk.container.Policy;
import com.topjohnwu.magisk.container.SuLogEntry;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.superuser.Shell;

import java.io.File;
import java.util.Collections;
import java.util.List;

import androidx.annotation.NonNull;

public class MagiskDB {

    static final String POLICY_TABLE = "policies";
    static final String LOG_TABLE = "logs";
    static final String SETTINGS_TABLE = "settings";
    static final String STRINGS_TABLE = "strings";
    static final File LEGACY_MANAGER_DB =
            new File(Utils.fmt("/sbin/.magisk/db-%d/magisk.db", Const.USER_ID));

    @NonNull
    public static MagiskDB getInstance() {
        if (LEGACY_MANAGER_DB.canWrite()) {
            return MagiskDBLegacy.newInstance();
        } else if (Shell.rootAccess()) {
            return Data.magiskVersionCode >= Const.MAGISK_VER.CMDLINE_DB ?
                new MagiskDBCmdline() : MagiskDBLegacy.newInstance();
        } else {
            return new MagiskDB();
        }
    }

    public void clearOutdated() {}

    public void deletePolicy(Policy policy) {
        deletePolicy(policy.uid);
    }

    public void deletePolicy(String pkg) {}

    public void deletePolicy(int uid) {}

    public Policy getPolicy(int uid) { return null; }

    public void updatePolicy(Policy policy) {}

    public List<Policy> getPolicyList() { return Collections.emptyList(); }

    public List<List<SuLogEntry>> getLogs() { return Collections.emptyList(); }

    public void addLog(SuLogEntry log) {}

    public void clearLogs() {}

    public void setSettings(String key, int value) {}

    public int getSettings(String key, int defaultValue) { return defaultValue; }

    public void setStrings(String key, String value) {}

    public String getStrings(String key, String defaultValue) { return defaultValue; }
}
