package com.topjohnwu.magisk.database;

import com.topjohnwu.magisk.Const;
import com.topjohnwu.magisk.container.Policy;
import com.topjohnwu.magisk.container.SuLogEntry;
import com.topjohnwu.magisk.utils.Utils;

import java.io.File;
import java.util.List;

import androidx.annotation.NonNull;

public abstract class MagiskDB {

    public static final int DATABASE_VER = 6;
    public static final int OLD_DATABASE_VER = 5;
    public static final String POLICY_TABLE = "policies";
    public static final String LOG_TABLE = "logs";
    public static final String SETTINGS_TABLE = "settings";
    public static final String STRINGS_TABLE = "strings";
    public static final File LEGACY_MANAGER_DB =
            new File(Utils.fmt("/sbin/.core/db-%d/magisk.db", Const.USER_ID));

    @NonNull
    public static MagiskDB getInstance() {
        return MagiskDBLegacy.newInstance();
    }

    public abstract void clearOutdated();

    public void deletePolicy(Policy policy) {
        deletePolicy(policy.uid);
    }

    public abstract void deletePolicy(String pkg);

    public abstract void deletePolicy(int uid);

    public abstract Policy getPolicy(int uid);

    public abstract void addPolicy(Policy policy);

    public abstract void updatePolicy(Policy policy);

    public abstract List<Policy> getPolicyList();

    public abstract List<List<SuLogEntry>> getLogs();

    public abstract void addLog(SuLogEntry log);

    public abstract void clearLogs();

    public abstract void setSettings(String key, int value);

    public abstract int getSettings(String key, int defaultValue);

    public abstract void setStrings(String key, String value);

    public abstract String getStrings(String key, String defaultValue);
}
