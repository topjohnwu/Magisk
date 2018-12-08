package com.topjohnwu.magisk.database;

import android.content.ContentValues;
import android.content.Context;
import android.content.pm.PackageManager;
import android.database.Cursor;
import android.database.DatabaseUtils;
import android.database.sqlite.SQLiteDatabase;
import android.os.Build;
import android.os.Process;
import android.text.TextUtils;
import android.widget.Toast;

import com.topjohnwu.magisk.Const;
import com.topjohnwu.magisk.Data;
import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.container.Policy;
import com.topjohnwu.magisk.container.SuLogEntry;
import com.topjohnwu.magisk.utils.LocaleManager;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.superuser.Shell;
import com.topjohnwu.superuser.io.SuFile;

import java.text.DateFormat;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Date;
import java.util.List;

public class MagiskDBLegacy extends MagiskDB {

    private static final int DATABASE_VER = 6;
    private static final int OLD_DATABASE_VER = 5;

    private PackageManager pm;
    private SQLiteDatabase db;

    static MagiskDBLegacy newInstance() {
        try {
            return new MagiskDBLegacy();
        } catch (Exception e) {
            // Let's cleanup everything and try again
            Shell.su("db_clean '*'").exec();
            return new MagiskDBLegacy();
        }
    }

    private MagiskDBLegacy() {
        pm = Data.MM().getPackageManager();
        db = openDatabase();
        db.disableWriteAheadLogging();
        int version = Data.magiskVersionCode >= Const.MAGISK_VER.DBVER_SIX ? DATABASE_VER : OLD_DATABASE_VER;
        int curVersion = db.getVersion();
        if (curVersion < version) {
            onUpgrade(db, curVersion);
        } else if (curVersion > DATABASE_VER) {
            /* Higher than we can possibly support */
            onDowngrade(db);
        }
        db.setVersion(version);
        clearOutdated();
    }

    private SQLiteDatabase openDatabase() {
        MagiskManager mm = Data.MM();
        Context de = Build.VERSION.SDK_INT >= Build.VERSION_CODES.N
                ? mm.createDeviceProtectedStorageContext() : mm;
        if (!LEGACY_MANAGER_DB.canWrite()) {
            if (!Shell.rootAccess() || Data.magiskVersionCode < 0) {
                // We don't want the app to crash, create a db and return
                return mm.openOrCreateDatabase("su.db", Context.MODE_PRIVATE, null);
            }
            // Cleanup
            Shell.su("db_clean " + Const.USER_ID).exec();
            // Global database
            final SuFile GLOBAL_DB = new SuFile("/data/adb/magisk.db");
            mm.deleteDatabase("su.db");
            de.deleteDatabase("su.db");
            if (Data.magiskVersionCode < Const.MAGISK_VER.SEPOL_REFACTOR) {
                // We need some additional policies on old versions
                Shell.su("db_sepatch").exec();
            }
            if (!GLOBAL_DB.exists()) {
                Shell.su("db_init").exec();
                SQLiteDatabase.openOrCreateDatabase(GLOBAL_DB, null).close();
                Shell.su("db_restore").exec();
            }
            Shell.su("db_setup " + Process.myUid()).exec();
        }
        // Not using legacy mode, open the mounted global DB
        return SQLiteDatabase.openOrCreateDatabase(LEGACY_MANAGER_DB, null);
    }

    private void onUpgrade(SQLiteDatabase db, int oldVersion) {
        if (oldVersion == 0) {
            createTables(db);
            oldVersion = 3;
        }
        if (oldVersion == 1) {
            // We're dropping column app_name, rename and re-construct table
            db.execSQL(Utils.fmt("ALTER TABLE %s RENAME TO %s_old", POLICY_TABLE));

            // Create the new tables
            createTables(db);

            // Migrate old data to new tables
            db.execSQL(Utils.fmt("INSERT INTO %s SELECT " +
                            "uid, package_name, policy, until, logging, notification FROM %s_old",
                    POLICY_TABLE, POLICY_TABLE));
            db.execSQL(Utils.fmt("DROP TABLE %s_old", POLICY_TABLE));

            Data.MM().deleteDatabase("sulog.db");
            ++oldVersion;
        }
        if (oldVersion == 2) {
            db.execSQL(Utils.fmt("UPDATE %s SET time=time*1000", LOG_TABLE));
            ++oldVersion;
        }
        if (oldVersion == 3) {
            db.execSQL(Utils.fmt("CREATE TABLE IF NOT EXISTS %s (key TEXT, value TEXT, PRIMARY KEY(key))", STRINGS_TABLE));
            ++oldVersion;
        }
        if (oldVersion == 4) {
            db.execSQL(Utils.fmt("UPDATE %s SET uid=uid%%100000", POLICY_TABLE));
            ++oldVersion;
        }
        if (oldVersion == 5) {
            setSettings(Const.Key.SU_FINGERPRINT,
                    Data.MM().prefs.getBoolean(Const.Key.SU_FINGERPRINT, false) ? 1 : 0);
            ++oldVersion;
        }
    }

    // Remove everything, we do not support downgrade
    private void onDowngrade(SQLiteDatabase db) {
        Utils.toast(R.string.su_db_corrupt, Toast.LENGTH_LONG);
        db.execSQL("DROP TABLE IF EXISTS " + POLICY_TABLE);
        db.execSQL("DROP TABLE IF EXISTS " + LOG_TABLE);
        db.execSQL("DROP TABLE IF EXISTS " + SETTINGS_TABLE);
        db.execSQL("DROP TABLE IF EXISTS " + STRINGS_TABLE);
        onUpgrade(db, 0);
    }

    private void createTables(SQLiteDatabase db) {
        // Policies
        db.execSQL(
                "CREATE TABLE IF NOT EXISTS " + POLICY_TABLE + " " +
                        "(uid INT, package_name TEXT, policy INT, " +
                        "until INT, logging INT, notification INT, " +
                        "PRIMARY KEY(uid))");

        // Logs
        db.execSQL(
                "CREATE TABLE IF NOT EXISTS " + LOG_TABLE + " " +
                        "(from_uid INT, package_name TEXT, app_name TEXT, from_pid INT, " +
                        "to_uid INT, action INT, time INT, command TEXT)");

        // Settings
        db.execSQL(
                "CREATE TABLE IF NOT EXISTS " + SETTINGS_TABLE + " " +
                        "(key TEXT, value INT, PRIMARY KEY(key))");
    }

    @Override
    public void clearOutdated() {
        // Clear outdated policies
        db.delete(POLICY_TABLE, Utils.fmt("until > 0 AND until < %d", System.currentTimeMillis() / 1000), null);
        // Clear outdated logs
        db.delete(LOG_TABLE, Utils.fmt("time < %d", System.currentTimeMillis() - Data.suLogTimeout * 86400000), null);
    }

    @Override
    public void deletePolicy(String pkg) {
        db.delete(POLICY_TABLE, "package_name=?", new String[] { pkg });
    }

    @Override
    public void deletePolicy(int uid) {
        db.delete(POLICY_TABLE, Utils.fmt("uid=%d", uid), null);
    }

    @Override
    public Policy getPolicy(int uid) {
        Policy policy = null;
        try (Cursor c = db.query(POLICY_TABLE, null, Utils.fmt("uid=%d", uid), null, null, null, null)) {
            if (c.moveToNext()) {
                ContentValues values = new ContentValues();
                DatabaseUtils.cursorRowToContentValues(c, values);
                policy = new Policy(values, pm);
            }
        } catch (PackageManager.NameNotFoundException e) {
            deletePolicy(uid);
            return null;
        }
        return policy;
    }

    @Override
    public void updatePolicy(Policy policy) {
        db.replace(POLICY_TABLE, null, policy.getContentValues());
    }

    @Override
    public List<Policy> getPolicyList() {
        try (Cursor c = db.query(POLICY_TABLE, null, Utils.fmt("uid/100000=%d", Const.USER_ID),
                null, null, null, null)) {
            List<Policy> ret = new ArrayList<>(c.getCount());
            while (c.moveToNext()) {
                try {
                    ContentValues values = new ContentValues();
                    DatabaseUtils.cursorRowToContentValues(c, values);
                    Policy policy = new Policy(values, pm);
                    ret.add(policy);
                } catch (PackageManager.NameNotFoundException e) {
                    // The app no longer exist, remove from DB
                    deletePolicy(c.getInt(c.getColumnIndex("uid")));
                }
            }
            Collections.sort(ret);
            return ret;
        }
    }

    @Override
    public List<List<SuLogEntry>> getLogs() {
        try (Cursor c = db.query(LOG_TABLE, null, Utils.fmt("from_uid/100000=%d", Const.USER_ID),
                null, null, null, "time DESC")) {
            List<List<SuLogEntry>> ret = new ArrayList<>();
            List<SuLogEntry> list = null;
            String dateString = null, newString;
            while (c.moveToNext()) {
                Date date = new Date(c.getLong(c.getColumnIndex("time")));
                newString = DateFormat.getDateInstance(DateFormat.MEDIUM, LocaleManager.locale).format(date);
                if (!TextUtils.equals(dateString, newString)) {
                    dateString = newString;
                    list = new ArrayList<>();
                    ret.add(list);
                }
                ContentValues values = new ContentValues();
                DatabaseUtils.cursorRowToContentValues(c, values);
                list.add(new SuLogEntry(values));
            }
            return ret;
        }
    }

    @Override
    public void addLog(SuLogEntry log) {
        db.insert(LOG_TABLE, null, log.getContentValues());
    }

    @Override
    public void clearLogs() {
        db.delete(LOG_TABLE, null, null);
    }

    @Override
    public void setSettings(String key, int value) {
        ContentValues data = new ContentValues();
        data.put("key", key);
        data.put("value", value);
        db.replace(SETTINGS_TABLE, null, data);
    }

    @Override
    public int getSettings(String key, int defaultValue) {
        int value = defaultValue;
        try (Cursor c = db.query(SETTINGS_TABLE, null, "key=?",new String[] { key }, null, null, null)) {
            if (c.moveToNext()) {
                value = c.getInt(c.getColumnIndex("value"));
            }
        }
        return value;
    }

    @Override
    public void setStrings(String key, String value) {
        if (value == null) {
            db.delete(STRINGS_TABLE, "key=?", new String[] { key });
        } else {
            ContentValues data = new ContentValues();
            data.put("key", key);
            data.put("value", value);
            db.replace(STRINGS_TABLE, null, data);
        }
    }

    @Override
    public String getStrings(String key, String defaultValue) {
        String value = defaultValue;
        try (Cursor c = db.query(STRINGS_TABLE, null, "key=?",new String[] { key }, null, null, null)) {
            if (c.moveToNext()) {
                value = c.getString(c.getColumnIndex("value"));
            }
        }
        return value;
    }
}
