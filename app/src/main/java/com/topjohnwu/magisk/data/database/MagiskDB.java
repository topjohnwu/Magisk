package com.topjohnwu.magisk.data.database;

import android.content.ContentValues;
import android.content.Context;
import android.content.pm.PackageManager;
import android.text.TextUtils;

import com.topjohnwu.magisk.Config;
import com.topjohnwu.magisk.Const;
import com.topjohnwu.magisk.model.entity.Policy;
import com.topjohnwu.magisk.model.entity.SuLogEntry;
import com.topjohnwu.magisk.utils.LocaleManager;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.superuser.Shell;

import java.text.DateFormat;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Date;
import java.util.List;
import java.util.Map;

import kotlin.DeprecationLevel;

@Deprecated
@kotlin.Deprecated(message = "", level = DeprecationLevel.ERROR)
public class MagiskDB {

    private static final String POLICY_TABLE = "policies";
    private static final String LOG_TABLE = "logs";
    private static final String SETTINGS_TABLE = "settings";
    private static final String STRINGS_TABLE = "strings";

    private final PackageManager pm;

    @Deprecated
    public MagiskDB(Context context) {
        pm = context.getPackageManager();
    }

    @Deprecated
    public void deletePolicy(Policy policy) {
        deletePolicy(policy.uid);
    }

    @Deprecated
    private List<String> rawSQL(String fmt, Object... args) {
        return Shell.su("magisk --sqlite '" + Utils.fmt(fmt, args) + "'").exec().getOut();
    }

    @Deprecated
    private List<ContentValues> SQL(String fmt, Object... args) {
        List<ContentValues> list = new ArrayList<>();
        for (String raw : rawSQL(fmt, args)) {
            ContentValues values = new ContentValues();
            String[] cols = raw.split("\\|");
            for (String col : cols) {
                String[] pair = col.split("=", 2);
                if (pair.length != 2)
                    continue;
                values.put(pair[0], pair[1]);
            }
            list.add(values);
        }
        return list;
    }

    @Deprecated
    private String toSQL(ContentValues values) {
        StringBuilder keys = new StringBuilder(), vals = new StringBuilder();
        keys.append('(');
        vals.append("VALUES(");
        boolean first = true;
        for (Map.Entry<String, Object> entry : values.valueSet()) {
            if (!first) {
                keys.append(',');
                vals.append(',');
            } else {
                first = false;
            }
            keys.append(entry.getKey());
            vals.append('"');
            vals.append(entry.getValue());
            vals.append('"');
        }
        keys.append(')');
        vals.append(')');
        keys.append(vals);
        return keys.toString();
    }

    @Deprecated
    public void clearOutdated() {
        rawSQL(
                "DELETE FROM %s WHERE until > 0 AND until < %d;" +
                        "DELETE FROM %s WHERE time < %d",
                POLICY_TABLE, System.currentTimeMillis() / 1000,
                LOG_TABLE, System.currentTimeMillis() - Config.suLogTimeout * 86400000
        );
    }

    @Deprecated
    public void deletePolicy(String pkg) {
        rawSQL("DELETE FROM %s WHERE package_name=\"%s\"", POLICY_TABLE, pkg);
    }

    @Deprecated
    public void deletePolicy(int uid) {
        rawSQL("DELETE FROM %s WHERE uid=%d", POLICY_TABLE, uid);
    }

    @Deprecated
    public Policy getPolicy(int uid) {
        List<ContentValues> res =
                SQL("SELECT * FROM %s WHERE uid=%d", POLICY_TABLE, uid);
        if (!res.isEmpty()) {
            try {
                return new Policy(res.get(0), pm);
            } catch (PackageManager.NameNotFoundException e) {
                deletePolicy(uid);
            }
        }
        return null;
    }

    @Deprecated
    public void updatePolicy(Policy policy) {
        rawSQL("REPLACE INTO %s %s", POLICY_TABLE, toSQL(policy.getContentValues()));
    }

    @Deprecated
    public List<Policy> getPolicyList() {
        List<Policy> list = new ArrayList<>();
        for (ContentValues values : SQL("SELECT * FROM %s WHERE uid/100000=%d", POLICY_TABLE, Const.USER_ID)) {
            try {
                list.add(new Policy(values, pm));
            } catch (PackageManager.NameNotFoundException e) {
                deletePolicy(values.getAsInteger("uid"));
            }
        }
        Collections.sort(list);
        return list;
    }

    @Deprecated
    public List<List<SuLogEntry>> getLogs() {
        List<List<SuLogEntry>> ret = new ArrayList<>();
        List<SuLogEntry> list = null;
        String dateString = null, newString;
        for (ContentValues values : SQL("SELECT * FROM %s ORDER BY time DESC", LOG_TABLE)) {
            Date date = new Date(values.getAsLong("time"));
            newString = DateFormat.getDateInstance(DateFormat.MEDIUM, LocaleManager.getLocale()).format(date);
            if (!TextUtils.equals(dateString, newString)) {
                dateString = newString;
                list = new ArrayList<>();
                ret.add(list);
            }
            list.add(new SuLogEntry(values));
        }
        return ret;
    }

    @Deprecated
    public void addLog(SuLogEntry log) {
        rawSQL("INSERT INTO %s %s", LOG_TABLE, toSQL(log.getContentValues()));
    }

    @Deprecated
    public void clearLogs() {
        rawSQL("DELETE FROM %s", LOG_TABLE);
    }

    @Deprecated
    public void rmSettings(String key) {
        rawSQL("DELETE FROM %s WHERE key=\"%s\"", SETTINGS_TABLE, key);
    }

    @Deprecated
    public void setSettings(String key, int value) {
        ContentValues data = new ContentValues();
        data.put("key", key);
        data.put("value", value);
        rawSQL("REPLACE INTO %s %s", SETTINGS_TABLE, toSQL(data));
    }

    @Deprecated
    public int getSettings(String key, int defaultValue) {
        List<ContentValues> res = SQL("SELECT value FROM %s WHERE key=\"%s\"", SETTINGS_TABLE, key);
        if (res.isEmpty())
            return defaultValue;
        return res.get(0).getAsInteger("value");
    }

    @Deprecated
    public void setStrings(String key, String value) {
        if (value == null) {
            rawSQL("DELETE FROM %s WHERE key=\"%s\"", STRINGS_TABLE, key);
            return;
        }
        ContentValues data = new ContentValues();
        data.put("key", key);
        data.put("value", value);
        rawSQL("REPLACE INTO %s %s", STRINGS_TABLE, toSQL(data));
    }

    @Deprecated
    public String getStrings(String key, String defaultValue) {
        List<ContentValues> res = SQL("SELECT value FROM %s WHERE key=\"%s\"", STRINGS_TABLE, key);
        if (res.isEmpty())
            return defaultValue;
        return res.get(0).getAsString("value");
    }
}
